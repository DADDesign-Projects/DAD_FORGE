//==================================================================================
//==================================================================================
// File: cModulator.h
// Description: Pitch modulation effect header with LFO-controlled delay
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "cDCO.h"
#include "cDelayLine.h"
#include "Sections.h"

namespace DadDSP {

//**********************************************************************************
// cModulator Class Declaration
//**********************************************************************************

class cModulator {
public:
    // =============================================================================
    // Constructor/Destructor
    // =============================================================================

    cModulator() = default;      // Default constructor
    ~cModulator() = default;     // Default destructor

    // =============================================================================
    // Public Methods
    // =============================================================================

    // Initialize modulator with parameters and buffer
    void Initialize(float sampleRate, float* pBuffer, uint32_t BufferSize, float LFOFrequence,
                    float PitchVariationMin, float PitchVariationMax, float TimeOffset);

    // Calculate sample delay based on pitch ratio and LFO frequency
    float calcSample(float Pitch, float SampleRate, float LFO) const;

    // Set LFO frequency and recalculate pitch compensation
    void setFreq(float Freq);

    // Set pitch variation range
    void setPitchVariation(float PitchVariationMin, float PitchVariationMax);

    // Process audio sample with pitch modulation
    ITCM float Process(float Sample, float Depth);

protected:
    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    float       m_sampleRate;        // Audio sample rate in Hz
    uint32_t    m_BufferSize;        // Size of delay buffer in samples
    float       m_LFOFrequence;      // LFO frequency in Hz
    float       m_PitchVariationMin; // Minimum pitch variation percentage
    float       m_PitchVariationMax; // Maximum pitch variation percentage
    float       m_SamplesMax;        // Maximum delay in samples
    float       m_SamplesMin;        // Minimum delay in samples
    float       m_NbSampleOffset;    // Time offset in samples
    cDelayLine  m_DelayLine;         // Delay line for pitch shifting
    cDCO        m_DCO;               // Digital controlled oscillator for LFO
};

} // namespace DadDSP

//***End of file**************************************************************
