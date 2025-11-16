//==================================================================================
//==================================================================================
// File: AudioManager.cpp
// Description: Audio hardware management implementation for STM32H MPU
//
// Copyright (c) 22025 Dad Design.
//==================================================================================
//==================================================================================

//**********************************************************************************
// Audio Hardware Management on STM32H MPU
//**********************************************************************************

#include "HardwareAndCo.h"

// =============================================================================
// Constants and Definitions
// =============================================================================

#define SAI_HALF_BUFFER_SIZE  (AUDIO_BUFFER_SIZE * 2)  // Stereo buffer size
#define SAI_BUFFER_SIZE       (AUDIO_BUFFER_SIZE * 4)  // Full buffer size

// =============================================================================
// Global Variables
// =============================================================================

// Audio buffers in non-cached RAM
NO_CACHE_RAM AudioBuffer In[AUDIO_BUFFER_SIZE];        // Input audio buffer
NO_CACHE_RAM AudioBuffer Out1[AUDIO_BUFFER_SIZE];      // First output buffer
NO_CACHE_RAM AudioBuffer Out2[AUDIO_BUFFER_SIZE];      // Second output buffer

NO_CACHE_RAM AudioBuffer* pOut;                        // Current output buffer pointer
NO_CACHE_RAM int32_t rxBuffer[SAI_BUFFER_SIZE];        // Receive buffer for SAI
NO_CACHE_RAM int32_t txBuffer[SAI_BUFFER_SIZE];        // Transmit buffer for SAI

// =============================================================================
// Conversion Functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert int32_t to float with 24-bit signed audio format
// -----------------------------------------------------------------------------
inline float int32ToFloat(int32_t sample) {
    // Handle sign extension for 24-bit audio data
    if (sample & 0x00800000) {
        sample |= 0xFF000000;  // Extend sign bit for negative values
    } else {
        sample &= 0x00FFFFFF;  // Mask to 24 bits for positive values
    }
    // Normalize to floating point range [-1.0, 1.0]
    return static_cast<float>(sample) / 8388608.0f;
}

// -----------------------------------------------------------------------------
// Convert int32_t buffer to float AudioBuffer
// -----------------------------------------------------------------------------
ITCM void ConvertToAudioBuffer(int32_t* intBuf, AudioBuffer* floatBuf) {
    // Process each stereo sample in the buffer
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        floatBuf[i].Left = int32ToFloat(intBuf[i * 2]);        // Convert left channel
        floatBuf[i].Right = int32ToFloat(intBuf[i * 2 + 1]);   // Convert right channel
    }
}

// -----------------------------------------------------------------------------
// Convert float to int32_t with 24-bit signed audio format
// -----------------------------------------------------------------------------
inline int32_t floatToInt32(float sample) {
    // Clamp input to valid audio range
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;

    // Scale to 24-bit integer range
    int32_t intSample = static_cast<int32_t>(sample * 8388608.0f);

    // Ensure 24-bit format
    return intSample & 0xFFFFFF;
}

// -----------------------------------------------------------------------------
// Convert float AudioBuffer to int32_t buffer
// -----------------------------------------------------------------------------
ITCM void ConvertFromAudioBuffer(AudioBuffer* floatBuf, int32_t* intBuf) {
    // Process each stereo sample in the buffer
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        intBuf[i * 2] = floatToInt32(floatBuf[i].Left);        // Convert left channel
        intBuf[i * 2 + 1] = floatToInt32(floatBuf[i].Right);   // Convert right channel
    }
}

// =============================================================================
// SAI Callback Functions
// =============================================================================

// Global SAI handle pointers
SAI_HandleTypeDef *__phSaiTx = nullptr;  // Transmit SAI handle
SAI_HandleTypeDef *__phSaiRx = nullptr;  // Receive SAI handle

// -----------------------------------------------------------------------------
// Callback for transmission complete
// -----------------------------------------------------------------------------
ITCM void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    if (__phSaiTx == hsai) {
        __disable_irq();
        // Convert second half of output buffer for transmission
        ConvertFromAudioBuffer(pOut, &txBuffer[SAI_HALF_BUFFER_SIZE]);
        __enable_irq();
    }
}

// -----------------------------------------------------------------------------
// Callback for half transmission complete
// -----------------------------------------------------------------------------
ITCM void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    if (__phSaiTx == hsai) {
        __disable_irq();
        // Convert first half of output buffer for transmission
        ConvertFromAudioBuffer(pOut, txBuffer);
        __enable_irq();
    }
}

// -----------------------------------------------------------------------------
// Callback for reception complete
// -----------------------------------------------------------------------------
ITCM void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {
    if (__phSaiRx == hsai) {
        // Convert second half of received buffer to float format
        ConvertToAudioBuffer(&rxBuffer[SAI_HALF_BUFFER_SIZE], In);

        // Process audio data through application callback
        AudioCallback(In, Out2);

        __disable_irq();
        pOut = Out2;  // Switch to second output buffer
        __enable_irq();
    }
}

// -----------------------------------------------------------------------------
// Callback for half reception complete
// -----------------------------------------------------------------------------
ITCM void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    if (__phSaiRx == hsai) {
        // Convert first half of received buffer to float format
        ConvertToAudioBuffer(rxBuffer, In);

        // Process audio data through application callback
        AudioCallback(In, Out1);

        __disable_irq();
        pOut = Out1;  // Switch to first output buffer
        __enable_irq();
    }
}

// =============================================================================
// Audio Management Functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize and start audio processing
// -----------------------------------------------------------------------------
HAL_StatusTypeDef StartAudio(SAI_HandleTypeDef *phSaiTx, SAI_HandleTypeDef *phSaiRx) {
    HAL_StatusTypeDef Result;

    // Initialize buffers and pointers
    pOut = Out1;

    // Clear all audio buffers
    for (uint16_t Index = 0; Index < AUDIO_BUFFER_SIZE; Index++) {
        In[Index].Left = 0.0f;
        In[Index].Right = 0.0f;
        Out1[Index].Left = 0.0f;
        Out1[Index].Right = 0.0f;
        Out2[Index].Left = 0.0f;
        Out2[Index].Right = 0.0f;
    }

    // Clear SAI buffers
    for (uint16_t Index = 0; Index < SAI_BUFFER_SIZE; Index++) {
        rxBuffer[Index] = 0;
        txBuffer[Index] = 0;
    }

    // Store SAI handles for callback identification
    __phSaiRx = phSaiRx;
    __phSaiTx = phSaiTx;

    // Start receiving audio data via DMA
    if (HAL_OK != (Result = HAL_SAI_Receive_DMA(phSaiRx, (uint8_t*)rxBuffer, SAI_BUFFER_SIZE))) {
        return Result;
    }

    // Start transmitting audio data via DMA
    return HAL_SAI_Transmit_DMA(phSaiTx, (uint8_t*)txBuffer, SAI_BUFFER_SIZE);
}

//***End of file**************************************************************
