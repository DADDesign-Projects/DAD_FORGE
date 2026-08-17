// ======================================================================
// ======================================================================
// File: cIRLoaderEffect.h
// Description: Uniform Partitioned Convolution with smart spread - Header
// 
// Copyright(c) 2025-2026 DadDesign-Projects
// 
// SPDX-License-Identifier: Apache-2.0
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at 
// http://www.apache.org/licenses/LICENSE-2.0
// ======================================================================
// ======================================================================

#pragma once

//**********************************************************************************
// Includes
//**********************************************************************************
#include "cEffectBase.h"
#include "ID.h"
#define DECLARE_EFFECT DadEffect::cIRLoaderEffect __Effect
#define EFFECT_NAME "IR Loader"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "IRLoader.png"
constexpr uint32_t EFFECT_BUILD = BUILD_ID('I','R','P','1');

#include "HardwareDefines.h"
#include "arm_math.h"
#include "cPanelOfListChoice.h"
#include "cPanelOfParametricEqualizer.h"


//**********************************************************************************
// Constants
//**********************************************************************************
#define IR_MAX_TAPS         8192                     // Maximum number of FIR taps

// Number of spread phases (can be tuned)
#define SPREAD_PHASES       4

// PARTITION_SIZE is derived automatically
#define PARTITION_SIZE      (SPREAD_PHASES * AUDIO_BUFFER_SIZE)

#define FFT_SIZE            (PARTITION_SIZE * 2)
#define NUM_PARTITIONS      (IR_MAX_TAPS / PARTITION_SIZE)
#define PARTS_PER_PHASE     (NUM_PARTITIONS / SPREAD_PHASES)

static_assert(PARTITION_SIZE * NUM_PARTITIONS == IR_MAX_TAPS, "IR_MAX_TAPS must be divisible by PARTITION_SIZE");
static_assert(NUM_PARTITIONS % SPREAD_PHASES == 0, "NUM_PARTITIONS must be divisible by SPREAD_PHASES");
static_assert(PARTITION_SIZE == SPREAD_PHASES * AUDIO_BUFFER_SIZE, "PARTITION_SIZE mismatch");

constexpr float IR_FADE_TIME_MS = 350; // 0,35 second

constexpr float kBlocGain = 1.4f;

namespace DadEffect {

//**********************************************************************************
// Data Structures
//**********************************************************************************

#pragma pack(push, 1)
struct sWavHeader {
    char     riffId[4];         // "RIFF" chunk ID
    uint32_t riffSize;          // Size of RIFF chunk
    char     waveId[4];         // "WAVE" format ID
    char     fmtId[4];          // "fmt " chunk ID
    uint32_t fmtSize;           // Size of fmt chunk
    uint16_t audioFormat;       // Audio format (1 = PCM, 3 = IEEE float)
    uint16_t numChannels;       // Number of audio channels
    uint32_t sampleRate;        // Sample rate in Hz
    uint32_t byteRate;          // Bytes per second
    uint16_t blockAlign;        // Block alignment
    uint16_t bitsPerSample;     // Bits per sample
};
#pragma pack(pop)

//**********************************************************************************
// Enumerations
//**********************************************************************************

enum eIRLoadResult {
    IR_LOAD_OK = 0,                     // IR loaded successfully
    IR_LOAD_TRUNCATED,                  // IR was truncated to fit IR_MAX_TAPS
    IR_LOAD_ERR_FILE_NOT_FOUND,         // File not found in flash storage
    IR_LOAD_ERR_FORMAT,                 // Invalid WAV format
    IR_LOAD_ERR_SAMPLERATE,             // Sample rate not supported (must be 48000Hz)
    IR_LOAD_ERR_NO_DATA_CHUNK,          // No data chunk found in WAV file
    IR_NO_LOAD                          // No IR loaded yet
};

//**********************************************************************************
// Class Declaration
//**********************************************************************************

class cIRLoaderEffect : public cEffectBase {
public:
    // -----------------------------------------------------------------------------
    // Construction and initialization
    // -----------------------------------------------------------------------------
    cIRLoaderEffect() = default;
    void onInitialize() override;
    uint32_t getEffectID() override;
    
    // -----------------------------------------------------------------------------
    // IR loading and management
    // -----------------------------------------------------------------------------
    void LoadIRFiles();                     // Scan flash storage for .IR files
    eIRLoadResult LoadIRFromFlash(const char* fileName);   // Load IR from flash
    
    // -----------------------------------------------------------------------------
    // Audio processing
    // -----------------------------------------------------------------------------
    void onProcess(AudioBuffer *pIn, AudioBuffer *pOut, DadGUI::eEffectState_t State, bool Silence) override;
    bool onBlocProcess(AudioBuffer *pIn, AudioBuffer *pOut) override;

    // -----------------------------------------------------------------------------
    // Periodically updates fadin/fadout IR change
    void on_GUI_FastUpdate() override;

protected:
    // -----------------------------------------------------------------------------
    // Protected methods
    // -----------------------------------------------------------------------------
    bool FindDataChunk(const uint8_t* wavPtr, uint32_t fileSize, const uint8_t** dataPtr, uint32_t* dataSize);

    uint16_t ConvertAndLoadSamples(const uint8_t* dataPtr, uint32_t dataSize,
                                   uint16_t bitsPerSample, uint16_t numChannels,
                                   bool isFloat, bool* wasTruncated);

    // Prepare FFT partitions from FIR coefficients
    void PreparePartitions();

    // Prepare FFT partitions from FIR coefficients
    void MultiplyPartitions(int startK, int endK);

    // -----------------------------------------------------------------------------
    // Static callbacks
    // -----------------------------------------------------------------------------
    static void MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void GainChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void IRChange(DadDSP::cParameter* pParameter, uint32_t Context);

    // =============================================================================
    // Member variables
    // =============================================================================
    
    // UI Parameters
    DadGUI::cUIParameter            m_ParameterGain;            // Gain parameter
    DadGUI::cUIParameter            m_ParameterMix;             // Dry/wet mix parameter
    DadGUI::cParameterNumNormalView m_ParameterGainView;        // Gain UI view
    DadGUI::cParameterNumNormalView m_ParameterMixView;         // Mix UI view
    DadGUI::cPanelOfParameterView   m_ParametrerIRPanel;        // IR parameter panel
    float 							m_Gain=0.0f;

    // Static DSP buffers
    static float32_t m_FirCoeffs[IR_MAX_TAPS];                  // FIR coefficients
    static float32_t m_PartitionFFTs[NUM_PARTITIONS][FFT_SIZE]; // Frequency domain partitions
    static float32_t m_InputFFTs[NUM_PARTITIONS][FFT_SIZE];     // Input FFT history
    static float32_t m_FreqAcc[FFT_SIZE];                       // Frequency accumulator
    static float32_t m_TimeAcc[FFT_SIZE];                       // Time domain accumulator
    static float32_t m_Overlap[PARTITION_SIZE];                 // Overlap buffer for OLA
    static float32_t m_MonoIn[AUDIO_BUFFER_SIZE];               // Mono input buffer
    static float32_t m_MonoOut[AUDIO_BUFFER_SIZE];              // Mono output buffer
    static float32_t m_OutputBuffer[PARTITION_SIZE];            // Output partition buffer

    // SAI buffering
    float32_t m_SAI_InputAccum[PARTITION_SIZE];      			// Input accumulation buffer
    float32_t m_SAI_OutputAccum[PARTITION_SIZE];				// Output accumulation buffer
    uint16_t  m_SAI_Fill = 0;									// Fill counter for input accumulation
    uint16_t  m_SAI_OutPos = PARTITION_SIZE;         			// Output position counter

    // Spread state
    uint8_t   m_SpreadPhase = 0;                     			// Current spread phase (0..SPREAD_PHASES-1)

    // FFT instance
    arm_rfft_fast_instance_f32 m_RfftInstance;       			// ARM CMSIS FFT instance

    // State variables
    uint16_t 		m_HistIdx = 0;                          	// History buffer index
    uint16_t 		m_CurrentNumTaps = 0;                   	// Number of loaded taps
    eIRLoadResult 	m_LastLoadResult = IR_NO_LOAD;     			// Last load result

    // UI components
    DadGUI::cPanelOfListChoice 			m_PanelOfListChoice;  			// File list panel
    DadGUI::cPanelOfParametricEqualizer m_cPanelOfParametricEqualizer;   // Panel of Parametric Equalizer

    // FadeIn FadeOut variables
    enum class eFadeInFadeOut{
    	StartFadeOut,
		FadeOut,
		FadeIn,
		End
    } m_FadeState;

    float 		m_FadeGain;
    float		m_FadeStep;
    std::string	m_FadeFileName;

};

} // namespace DadEffect

//***End of file**************************************************************
