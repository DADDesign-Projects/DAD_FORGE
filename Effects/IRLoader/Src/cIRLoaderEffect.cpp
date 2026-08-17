// ======================================================================
// ======================================================================
// File: cIRLoaderEffect.cpp
// Description: Uniform Partitioned Convolution with smart spread - Implementation
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

//**********************************************************************************
// Includes
//**********************************************************************************
#if __has_include("EffectsConfig.h")
#include "EffectsConfig.h"
#else
#undef ACTIVE_EFFECT
#define ACTIVE_EFFECT 500
#endif
#if ACTIVE_EFFECT == EFFECT_IR_LOADER
#include "cIRLoaderEffect.h"
#include "GPIO.h"
#include "cIRLoaderEffect.h"
#include "cFlasherStorage.h"
#include <cstring>
#include <cmath>

extern DadPersistentStorage::cFlasherStorage __FlasherStorage;

namespace DadEffect {

//**********************************************************************************
// Constants
//**********************************************************************************

constexpr uint32_t IR_LOADER_ID = BUILD_ID('I', 'R', 'L', 'D');

// =============================================================================
// Static memory placement
// =============================================================================
RAM_D1  float32_t cIRLoaderEffect::m_FirCoeffs[IR_MAX_TAPS];
ITCM    float32_t cIRLoaderEffect::m_PartitionFFTs[NUM_PARTITIONS][FFT_SIZE];
        float32_t cIRLoaderEffect::m_InputFFTs[NUM_PARTITIONS][FFT_SIZE];
        float32_t cIRLoaderEffect::m_FreqAcc[FFT_SIZE];
        float32_t cIRLoaderEffect::m_TimeAcc[FFT_SIZE];
        float32_t cIRLoaderEffect::m_Overlap[PARTITION_SIZE];
RAM_D1  float32_t cIRLoaderEffect::m_MonoIn[AUDIO_BUFFER_SIZE];
RAM_D1  float32_t cIRLoaderEffect::m_MonoOut[AUDIO_BUFFER_SIZE];
        float32_t cIRLoaderEffect::m_OutputBuffer[PARTITION_SIZE];

//**********************************************************************************
// Class Implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------
void cIRLoaderEffect::onInitialize()
{
    // Initialize gain parameter
    m_ParameterGain.Init(IR_LOADER_ID, 50.0f, 0.0f, 100.0f, 5.0f, 1.0f, GainChange, (uint32_t)this, 0.5f, 20, true);
    m_ParameterGainView.Init(&m_ParameterGain, "Gain", "Gain", "%", "percent");

    // Initialize mix parameter with callback
    m_ParameterMix.Init(IR_LOADER_ID, 100.0f, 0.0f, 100.0f, 5.0f, 1.0f, MixChange, (uint32_t)this, 1.0f, 21);
    m_ParameterMixView.Init(&m_ParameterMix, "Mix", "Mix Dry/Wet", "%", "percent");

    // Setup UI panels
#ifndef HARD_DRYWET
    m_ParametrerIRPanel.Init(&m_ParameterGainView, nullptr, &m_ParameterMixView);
#else
    m_ParametrerIRPanel.Init(&m_ParameterGainView, nullptr, nullptr);
#endif
    m_PanelOfListChoice.Init(IR_LOADER_ID, IRChange, (uint32_t)this);
    m_cPanelOfParametricEqualizer.Init(IR_LOADER_ID);

    // Add menu items
    m_Menu.addMenuItem(&m_ParametrerIRPanel, "IRLoad");
    m_Menu.addMenuItem(&m_PanelOfListChoice, "List IR");
    m_Menu.addMenuItem(&m_cPanelOfParametricEqualizer, "Equalizer");

    // Initialize FFT instance
    arm_rfft_fast_init_f32(&m_RfftInstance, FFT_SIZE);

    // Clear all buffers
    memset(m_PartitionFFTs, 0, sizeof(m_PartitionFFTs));
    memset(m_InputFFTs, 0, sizeof(m_InputFFTs));
    memset(m_FreqAcc, 0, sizeof(m_FreqAcc));
    memset(m_Overlap, 0, sizeof(m_Overlap));
    memset(m_SAI_InputAccum, 0, sizeof(m_SAI_InputAccum));
    memset(m_SAI_OutputAccum, 0, sizeof(m_SAI_OutputAccum));

    // Reset state variables
    m_HistIdx = 0;
    m_SAI_Fill = 0;
    m_SAI_OutPos = PARTITION_SIZE;
    m_SpreadPhase = 0;

    // Scan flash storage for IR files
    // and Load IR from flash storage
    LoadIRFiles();
    m_FadeGain = 0.0f;
    m_FadeStep = GUI_FAST_UPDATE_MS / IR_FADE_TIME_MS;
    m_FadeState = eFadeInFadeOut::StartFadeOut;

    __DryWet.setMix(100);
}

// -----------------------------------------------------------------------------
// Get effect ID
// -----------------------------------------------------------------------------
uint32_t cIRLoaderEffect::getEffectID() { return IR_LOADER_ID; }

// -----------------------------------------------------------------------------
// Scan flash storage for IR files and load to PanelOfListChoice
// -----------------------------------------------------------------------------
void cIRLoaderEffect::LoadIRFiles()
{
    const uint16_t MAX_INDEX = 100;
    uint16_t index = 0;

    while (index < MAX_INDEX)
    {
        const char* fileName = __FlasherStorage.GetFileName(index);
        if (fileName == nullptr || fileName[0] == '\0') break;

        // Get filename length
        size_t len = 0;
        while (len < 256 && fileName[len] != '\0') ++len;
        if (len < 3 || len >= 256) { ++index; continue; }

        // Check for .IR extension (case insensitive)
        const char* ext = fileName + len - 3;
        if ((ext[0] == '.') &&
            ((ext[1] == 'I' || ext[1] == 'i') &&
             (ext[2] == 'R' || ext[2] == 'r')))
        {
            m_PanelOfListChoice.addItem2List(std::string(fileName, len));
        }
        ++index;
    }
}

// -----------------------------------------------------------------------------
// Prepare partitions from FIR coefficients
// -----------------------------------------------------------------------------
void cIRLoaderEffect::PreparePartitions()
{
    float32_t tmp[FFT_SIZE];

    // Transform each partition to frequency domain
    for (int p = 0; p < NUM_PARTITIONS; p++)
    {
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, &m_FirCoeffs[p * PARTITION_SIZE], PARTITION_SIZE * sizeof(float32_t));
        arm_rfft_fast_f32(&m_RfftInstance, tmp, m_PartitionFFTs[p], 0);
    }

    // Clear all buffers
    memset(m_InputFFTs, 0, sizeof(m_InputFFTs));
    memset(m_FreqAcc, 0, sizeof(m_FreqAcc));
    memset(m_Overlap, 0, sizeof(m_Overlap));
    memset(m_SAI_InputAccum, 0, sizeof(m_SAI_InputAccum));
    memset(m_SAI_OutputAccum, 0, sizeof(m_SAI_OutputAccum));

    // Reset state
    m_HistIdx = 0;
    m_SAI_Fill = 0;
    m_SAI_OutPos = PARTITION_SIZE;
    m_SpreadPhase = 0;
}

// -----------------------------------------------------------------------------
// Helper: multiplies a range of partitions and accumulates into m_FreqAcc
// -----------------------------------------------------------------------------
void cIRLoaderEffect::MultiplyPartitions(int startK, int endK)
{
    for (int k = startK; k < endK; k++)
    {
        // Get input history index (circular buffer)
        int inputIdx = m_HistIdx - k;
        if (inputIdx < 0) inputIdx += NUM_PARTITIONS;

        float32_t* pInFFT  = m_InputFFTs[inputIdx];
        float32_t* pPartFFT = m_PartitionFFTs[k];

        // DC and Nyquist bins (real only)
        m_FreqAcc[0] += pInFFT[0] * pPartFFT[0];
        m_FreqAcc[1] += pInFFT[1] * pPartFFT[1];

        // Complex bins (real/imag pairs)
        for (int n = 2; n < FFT_SIZE; n += 2)
        {
            float32_t aR = pInFFT[n];
            float32_t aI = pInFFT[n + 1];
            float32_t bR = pPartFFT[n];
            float32_t bI = pPartFFT[n + 1];

            // Complex multiplication: (aR + j*aI) * (bR + j*bI)
            m_FreqAcc[n]     += aR * bR - aI * bI;
            m_FreqAcc[n + 1] += aR * bI + aI * bR;
        }
    }
}

// -----------------------------------------------------------------------------
// Block processing - main convolution algorithm with smart spread
// -----------------------------------------------------------------------------
bool cIRLoaderEffect::onBlocProcess(AudioBuffer *pIn, AudioBuffer *pOut)
{
    // Check if IR is loaded
    if ((m_LastLoadResult != IR_LOAD_OK) && (m_LastLoadResult != IR_LOAD_TRUNCATED))
        return false;

    // 1. Convert to mono
    for (uint32_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
        m_MonoIn[i] = pIn[i].Left;

    // 2. Buffering with smart spread processing
    for (uint32_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        m_SAI_InputAccum[m_SAI_Fill++] = m_MonoIn[i];

        // Background work: only during the first 3 calls,
        // and only once per call (i == 0)
        // This distributes the computational load across multiple audio blocks
        if ((m_SAI_Fill < PARTITION_SIZE) && (i == 0) && (m_SpreadPhase < SPREAD_PHASES - 1))
        {
            // Big k values are multiplied first (independent of the new FFT)
            // Phase 0 -> k = 96..127
            // Phase 1 -> k = 64..95
            // Phase 2 -> k = 32..63
            int startK = (SPREAD_PHASES - 1 - m_SpreadPhase) * PARTS_PER_PHASE;
            int endK   = startK + PARTS_PER_PHASE;
            MultiplyPartitions(startK, endK);  // Currently commented out
            m_SpreadPhase++;
        }

        if (m_SAI_Fill >= PARTITION_SIZE)
        {
            m_SAI_Fill = 0;

            // === Full block: FFT + last (small) k + IFFT ===

            // 1. FFT of the current input
            float32_t inTmp[FFT_SIZE];
            memset(inTmp, 0, sizeof(inTmp));
            memcpy(inTmp, m_SAI_InputAccum, PARTITION_SIZE * sizeof(float32_t));
            arm_rfft_fast_f32(&m_RfftInstance, inTmp, m_InputFFTs[m_HistIdx], 0);  // Currently commented out

            // 2. Last multiplications = small k (0..31),
            //    which depend on the FFT we just computed
            MultiplyPartitions(0, PARTS_PER_PHASE);

            // 3. IFFT + scaling
            arm_rfft_fast_f32(&m_RfftInstance, m_FreqAcc, m_TimeAcc, 1);  // Currently commented out

            // 4. Overlap-add
            for (int n = 0; n < PARTITION_SIZE; n++)
            {
                m_OutputBuffer[n] = m_TimeAcc[n] + m_Overlap[n];
                m_Overlap[n]      = m_TimeAcc[n + PARTITION_SIZE];
            }

            // 5. Prepare next cycle
            m_HistIdx = (m_HistIdx + 1) % NUM_PARTITIONS;
            memset(m_FreqAcc, 0, sizeof(m_FreqAcc));
            m_SpreadPhase = 0;

            memcpy(m_SAI_OutputAccum, m_OutputBuffer, sizeof(m_OutputBuffer));
            m_SAI_OutPos = 0;
        }
    }

    // 3. Output
    for (uint32_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        float32_t sample = 0.0f;
        if (m_SAI_OutPos < PARTITION_SIZE)
            sample = m_SAI_OutputAccum[m_SAI_OutPos++];
        m_MonoOut[i] = sample;
    }

    // 4. Copy to output buffer (dual mono)
    for (uint32_t i = 0; i < AUDIO_BUFFER_SIZE; i++)
    {
        pOut[i].Left  = m_MonoOut[i] * kBlocGain;
        pOut[i].Right = m_MonoOut[i] * kBlocGain;
    }
    return true;
}

// -----------------------------------------------------------------------------
// Per-sample processing (for non-block processing mode)
// -----------------------------------------------------------------------------
void cIRLoaderEffect::onProcess(AudioBuffer *pIn, AudioBuffer *pOut,
                                     DadGUI::eEffectState_t State, bool Silence)
{
    float gainWet = __DryWet.getGainWet();

    pOut->Left  = pIn->Left  * m_Gain * gainWet * m_FadeGain;
    pOut->Right = pOut->Left;
}

//**********************************************************************************
// WAV Loading Implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Find the data chunk in a WAV file
// -----------------------------------------------------------------------------
bool cIRLoaderEffect::FindDataChunk(const uint8_t* wavPtr, uint32_t fileSize,
                                         const uint8_t** dataPtr, uint32_t* dataSize)
{
    uint32_t offset = sizeof(sWavHeader);
    while (offset + 8 <= fileSize)
    {
        const char* chunkId = (const char*)(wavPtr + offset);
        uint32_t chunkSize = *(uint32_t*)(wavPtr + offset + 4);

        if (memcmp(chunkId, "data", 4) == 0)
        {
            *dataPtr  = wavPtr + offset + 8;
            *dataSize = chunkSize;
            // Ensure we don't read past file end
            if (offset + 8 + chunkSize > fileSize)
                *dataSize = fileSize - (offset + 8);
            return true;
        }
        offset += 8 + chunkSize;
        // Chunk size padding
        if (chunkSize & 1) offset++;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Convert WAV samples to float32 and load into FIR coefficient buffer
// -----------------------------------------------------------------------------
uint16_t cIRLoaderEffect::ConvertAndLoadSamples(const uint8_t* dataPtr, uint32_t dataSize,
                                                     uint16_t bitsPerSample, uint16_t numChannels,
                                                     bool isFloat, bool* wasTruncated)
{
    uint32_t bytesPerSample = bitsPerSample / 8;
    uint32_t frameSize      = bytesPerSample * numChannels;
    uint32_t totalFrames    = dataSize / frameSize;

    // Check if truncation is needed
    bool truncated = (totalFrames > IR_MAX_TAPS);
    uint32_t numTaps = truncated ? IR_MAX_TAPS : totalFrames;
    // Align to 8 samples for SIMD compatibility
    numTaps = (numTaps / 8) * 8;
    if (numTaps == 0) numTaps = 8;

    // Convert samples
    for (uint32_t i = 0; i < numTaps; i++)
    {
        const uint8_t* framePtr = dataPtr + i * frameSize;
        float32_t sampleSum = 0.0f;

        // Sum all channels (convert to mono)
        for (uint16_t ch = 0; ch < numChannels; ch++)
        {
            const uint8_t* sPtr = framePtr + ch * bytesPerSample;
            float32_t sVal = 0.0f;

            if (isFloat && bitsPerSample == 32)
            {
                // IEEE 32-bit float
                sVal = *reinterpret_cast<const float32_t*>(sPtr);
            }
            else if (!isFloat && bitsPerSample == 16)
            {
                // 16-bit PCM
                int16_t raw = *reinterpret_cast<const int16_t*>(sPtr);
                sVal = (float32_t)raw / 32768.0f;
            }
            else if (!isFloat && bitsPerSample == 24)
            {
                // 24-bit PCM (converted to 32-bit)
                int32_t raw = (sPtr[0]) | (sPtr[1] << 8) | (sPtr[2] << 16);
                if (raw & 0x00800000) raw |= 0xFF000000;
                sVal = (float32_t)raw / 8388608.0f;
            }
            sampleSum += sVal;
        }
        // Average channels
        m_FirCoeffs[i] = sampleSum / (float32_t)numChannels;
    }

    // Zero-fill remaining taps if truncated
    if (numTaps < IR_MAX_TAPS)
        memset(&m_FirCoeffs[numTaps], 0, (IR_MAX_TAPS - numTaps) * sizeof(float32_t));

    *wasTruncated = truncated;
    return (uint16_t)numTaps;
}

// -----------------------------------------------------------------------------
// Load IR from flash storage
// -----------------------------------------------------------------------------
eIRLoadResult cIRLoaderEffect::LoadIRFromFlash(const char* fileName)
{
    // Get file pointer from flash storage
    const uint8_t* wavPtr = reinterpret_cast<const uint8_t*>(__FlasherStorage.GetFilePtr(fileName));
    if (wavPtr == nullptr)
        return m_LastLoadResult = IR_LOAD_ERR_FILE_NOT_FOUND;

    // Validate WAV header
    const sWavHeader* hdr = reinterpret_cast<const sWavHeader*>(wavPtr);
    if (memcmp(hdr->riffId, "RIFF", 4) != 0 || memcmp(hdr->waveId, "WAVE", 4) != 0)
        return m_LastLoadResult = IR_LOAD_ERR_FORMAT;

    // Check sample rate (must be 48kHz)
    if (hdr->sampleRate != 48000)
        return m_LastLoadResult = IR_LOAD_ERR_SAMPLERATE;

    // Find data chunk
    uint32_t fileSize = __FlasherStorage.GetFileSize(fileName);
    const uint8_t* dataPtr = nullptr;
    uint32_t dataSize = 0;

    if (!FindDataChunk(wavPtr, fileSize, &dataPtr, &dataSize))
        return m_LastLoadResult = IR_LOAD_ERR_NO_DATA_CHUNK;

    // Convert and load samples
    bool isFloat = (hdr->audioFormat == 3);
    bool wasTruncated = false;
    uint16_t numTaps = ConvertAndLoadSamples(dataPtr, dataSize,
                                             hdr->bitsPerSample, hdr->numChannels,
                                             isFloat, &wasTruncated);
    m_CurrentNumTaps = numTaps;

    // Peak normalization to prevent clipping
    float32_t peak = 0.0f;
    for (uint32_t i = 0; i < numTaps; i++)
    {
        float32_t a = fabsf(m_FirCoeffs[i]);
        if (a > peak) peak = a;
    }
    if (peak > 1e-6f)
    {
        float32_t scale = 1.0f / peak;
        for (uint32_t i = 0; i < numTaps; i++)
            m_FirCoeffs[i] *= scale;
    }

    // Prepare partitions for convolution
    PreparePartitions();
    m_LastLoadResult = wasTruncated ? IR_LOAD_TRUNCATED : IR_LOAD_OK;
    return m_LastLoadResult;
}

// -----------------------------------------------------------------------------
// Periodically updates fadin/fadout IR change
void cIRLoaderEffect::on_GUI_FastUpdate(){
	cEffectBase::on_GUI_FastUpdate();

	switch (m_FadeState){
		case eFadeInFadeOut::StartFadeOut:
			if(true == m_PanelOfListChoice.getSelectedItemText(m_FadeFileName)){
				m_FadeState = eFadeInFadeOut::FadeOut;
			}
			break;
		case eFadeInFadeOut::FadeOut:
			m_FadeGain -= m_FadeStep;
			if(m_FadeGain <= 0){
				m_FadeGain = 0.0f;
				m_LastLoadResult = eIRLoadResult::IR_NO_LOAD;
				LoadIRFromFlash(m_FadeFileName.c_str());
 				m_FadeState = eFadeInFadeOut::FadeIn;
			}
			break;
		case eFadeInFadeOut::FadeIn:
			m_FadeGain += m_FadeStep;
			if(m_FadeGain >= 1.0f){
				m_FadeGain = 1.0f;
				m_FadeState = eFadeInFadeOut::End;
			}
			break;
		case eFadeInFadeOut::End:
			break;
	}
}

//**********************************************************************************
// Callbacks
//**********************************************************************************

// -----------------------------------------------------------------------------
// Mix parameter change callback
// -----------------------------------------------------------------------------
void cIRLoaderEffect::MixChange(DadDSP::cParameter* pParameter, uint32_t Context){
#ifndef HARD_DRYWET
    __DryWet.setMix(pParameter->getValue());
#endif
}

// -----------------------------------------------------------------------------
// Gain parameter change callback
// -----------------------------------------------------------------------------
void cIRLoaderEffect::GainChange(DadDSP::cParameter* pParameter, uint32_t Context){
    cIRLoaderEffect* pThis = (cIRLoaderEffect*)Context;
    constexpr float k = 8.0f; // plus k est grand, plus la montée est tardive
    pThis->m_Gain = 2.0f * (powf(k, pParameter->getNormalizedValue()) - 1.0f) / (k - 1.0f);
}

// -----------------------------------------------------------------------------
// IR selection change callback
// -----------------------------------------------------------------------------
void cIRLoaderEffect::IRChange(DadDSP::cParameter* pParameter, uint32_t Context){
    cIRLoaderEffect* pThis = (cIRLoaderEffect*)Context;
    pThis->m_FadeState = eFadeInFadeOut::StartFadeOut;
}

} // namespace DadEffect
#endif
