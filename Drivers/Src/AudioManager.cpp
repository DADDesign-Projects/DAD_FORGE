//==================================================================================
// File: AudioManager.cpp
// Description: Audio hardware management implementation for STM32H MPU
// Optimized for Speed using ARM DSP Extensions and Hardware Intrinsics
//==================================================================================
#include "AudioManager.h"
#include "HardwareAndCo.h"
#include "arm_math.h" // Nécessaire pour les intrinsics ARM et CMSIS-DSP

// =============================================================================
// Constants and Definitions
// =============================================================================

#define ALIGN_8 __attribute__((aligned(8)))
#define SAI_HALF_BUFFER_SIZE  (AUDIO_BUFFER_SIZE * 2)  // Stereo buffer size (int32 elements)
#define SAI_BUFFER_SIZE       (AUDIO_BUFFER_SIZE * 4)  // Full buffer size

// Facteurs de conversion pré-calculés (constexpr pour optimisation compile-time)
static constexpr float kIntToFloatScale = 1.0f / 8388608.0f;
static constexpr float kFloatToIntScale = 8388608.0f;

// =============================================================================
// Global Variables
// =============================================================================

// Audio buffers in non-cached RAM
ALIGN_8 NO_CACHE_RAM AudioBuffer In[AUDIO_BUFFER_SIZE];
ALIGN_8 NO_CACHE_RAM AudioBuffer Out1[AUDIO_BUFFER_SIZE];
ALIGN_8 NO_CACHE_RAM AudioBuffer Out2[AUDIO_BUFFER_SIZE];

// Volatile est important car modifié en interruption et lu ailleurs
volatile NO_CACHE_RAM AudioBuffer* pOut;

ALIGN_8 NO_CACHE_RAM int32_t rxBuffer[SAI_BUFFER_SIZE];
ALIGN_8 NO_CACHE_RAM int32_t txBuffer[SAI_BUFFER_SIZE];

SAI_HandleTypeDef *__phSaiTx = nullptr;
SAI_HandleTypeDef *__phSaiRx = nullptr;

// =============================================================================
// Optimized Conversion Functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert int32_t buffer to float AudioBuffer (Optimized)
// -----------------------------------------------------------------------------
void ConvertToAudioBuffer(const int32_t* __restrict intBuf, AudioBuffer* __restrict floatBuf) {

    // Pointers for iteration
    const int32_t* pSrc = intBuf;
    AudioBuffer* pDst = floatBuf;

    // Loop unrolling hints could be added here, but compiler usually handles this well
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        int32_t leftRaw = (*pSrc++) << 8;
        int32_t rightRaw = (*pSrc++) << 8;

        // Conversion rapide multiplication
        pDst->Left  = (float)(leftRaw >> 8) * kIntToFloatScale;
        pDst->Right = (float)(rightRaw >> 8) * kIntToFloatScale;

        pDst++;
    }
}

// -----------------------------------------------------------------------------
// Convert float AudioBuffer to int32_t buffer (Optimized)
// -----------------------------------------------------------------------------
void ConvertFromAudioBuffer(const AudioBuffer* __restrict floatBuf, int32_t* __restrict intBuf) {

    const AudioBuffer* pSrc = floatBuf;
    int32_t* pDst = intBuf;

    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        // Conversion avec saturation matérielle
        // 1. Multiplication par le facteur d'échelle
        float fLeft = pSrc->Left * kFloatToIntScale;
        float fRight = pSrc->Right * kFloatToIntScale;

        // 2. Casting et Saturation via Intrinsic ARM (__SSAT)
        // __SSAT sature la valeur à 24 bits. C'est beaucoup plus rapide que les if/else.
        // Note: On cast en int32_t avant saturation.

        int32_t iLeft = (int32_t)fLeft;
        int32_t iRight = (int32_t)fRight;

        // Sature à 24 bits (-8388608 à 8388607)
        // Le masque 0xFFFFFF n'est techniquement pas requis si le SAI ignore les bits hauts,
        // mais __SSAT gère proprement les dépassements.
        *pDst++ = __SSAT(iLeft, 24);
        *pDst++ = __SSAT(iRight, 24);

        pSrc++;
    }
}

// =============================================================================
// SAI Callback Functions
// =============================================================================

// -----------------------------------------------------------------------------
// Helper inline pour éviter la duplication de code Tx
// -----------------------------------------------------------------------------
inline void ProcessTxCallback(SAI_HandleTypeDef *hsai, int32_t* targetBuffer) {
    if (__phSaiTx == hsai) {
        // Pas besoin de __disable_irq ici si pOut est lu atomiquement ou stable
        // Nous lisons le pointeur courant pOut
        ConvertFromAudioBuffer((AudioBuffer*)pOut, targetBuffer);
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    ProcessTxCallback(hsai, &txBuffer[SAI_HALF_BUFFER_SIZE]);
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    ProcessTxCallback(hsai, txBuffer);
}

// -----------------------------------------------------------------------------
// Helper inline pour éviter la duplication de code Rx
// -----------------------------------------------------------------------------
inline void ProcessRxCallback(SAI_HandleTypeDef *hsai, int32_t* sourceBuffer, AudioBuffer* targetFloatBuf) {
    if (__phSaiRx == hsai) {
        // 1. Conversion Entrée
        ConvertToAudioBuffer(sourceBuffer, In);

        // 2. Traitement Audio (Callback Utilisateur)
        // Note: Si ce callback est long, il devrait être fait hors interruption
        // (via un flag dans le main loop)
        AudioCallback(In, targetFloatBuf);

        // 3. Swap Buffer Output
        // L'assignation d'un pointeur 32 bits est atomique sur ARM Cortex-M.
        // __disable_irq() n'est pas nécessaire et ajoute de la latence.
        pOut = targetFloatBuf;
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
    ProcessRxCallback(hsai, &rxBuffer[SAI_HALF_BUFFER_SIZE], Out2);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    ProcessRxCallback(hsai, rxBuffer, Out1);
}

// =============================================================================
// Audio Management Functions
// =============================================================================

HAL_StatusTypeDef StartAudio(SAI_HandleTypeDef *phSaiTx, SAI_HandleTypeDef *phSaiRx) {
    HAL_StatusTypeDef Result;

    // Initialize buffers and pointers
    pOut = Out1;

    // Utilisation de memset (souvent optimisé par la lib C) au lieu de boucles manuelles
    memset((void*)In, 0, sizeof(In));
    memset((void*)Out1, 0, sizeof(Out1));
    memset((void*)Out2, 0, sizeof(Out2));
    memset((void*)rxBuffer, 0, sizeof(rxBuffer));
    memset((void*)txBuffer, 0, sizeof(txBuffer));

    __phSaiRx = phSaiRx;
    __phSaiTx = phSaiTx;

    if (HAL_OK != (Result = HAL_SAI_Receive_DMA(phSaiRx, (uint8_t*)rxBuffer, SAI_BUFFER_SIZE))) {
        return Result;
    }

    return HAL_SAI_Transmit_DMA(phSaiTx, (uint8_t*)txBuffer, SAI_BUFFER_SIZE);
}
