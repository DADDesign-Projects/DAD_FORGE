//==================================================================================
//==================================================================================
// File: cModulator.cpp
// Description: Pitch modulation effect implementation using delay line and LFO
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cModulator.h"

namespace DadDSP {

//**********************************************************************************
// cModulator
//**********************************************************************************

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize modulator with parameters and buffer
void cModulator::Initialize(float sampleRate, float* pBuffer, uint32_t BufferSize,
                           float LFOFrequence, float PitchVariationMin,
                           float PitchVariationMax, float TimeOffset)
{
    m_sampleRate = sampleRate;                    // Store sample rate
    m_BufferSize = BufferSize;                    // Store buffer size
    m_LFOFrequence = LFOFrequence + 1;            // Store LFO frequency with offset
    m_PitchVariationMin = PitchVariationMin;      // Store minimum pitch variation
    m_PitchVariationMax = PitchVariationMax;      // Store maximum pitch variation
    m_NbSampleOffset = sampleRate * TimeOffset;   // Calculate time offset in samples

    m_DelayLine.Initialize(pBuffer, BufferSize);  // Initialize delay line
    m_DelayLine.Clear();                          // Clear delay buffer
    m_DCO.Initialize(sampleRate, 0.0f, 0.0f, 100.0f, 0.5f);  // Initialize DCO

    setFreq(LFOFrequence);                        // Set initial frequency
}

// -----------------------------------------------------------------------------
// Set LFO frequency and recalculate pitch compensation
void cModulator::setFreq(float Freq)
{
    if((Freq != m_LFOFrequence) && (Freq > 0.0f)) {
        m_LFOFrequence = Freq;                    // Update frequency
        m_DCO.setFreq(Freq);                      // Update DCO frequency
        m_SamplesMax = calcSample(m_PitchVariationMax, m_sampleRate, m_LFOFrequence);  // Recalculate max samples
        m_SamplesMin = calcSample(m_PitchVariationMin, m_sampleRate, m_LFOFrequence);  // Recalculate min samples

        if((m_SamplesMax + m_NbSampleOffset) >= m_BufferSize) Error_Handler();  // Validate buffer size
    }
}

// -----------------------------------------------------------------------------
// Set pitch variation range
void cModulator::setPitchVariation(float PitchVariationMin, float PitchVariationMax)
{
    if(PitchVariationMin != m_PitchVariationMin) {
        m_PitchVariationMin = PitchVariationMin;  // Update min pitch variation
        m_SamplesMin = calcSample(m_PitchVariationMin, m_sampleRate, m_LFOFrequence);  // Recalculate min samples
    }
    if(PitchVariationMax != m_PitchVariationMax) {
        m_PitchVariationMax = PitchVariationMax;  // Update max pitch variation
        m_SamplesMax = calcSample(m_PitchVariationMax, m_sampleRate, m_LFOFrequence);  // Recalculate max samples

        if((m_SamplesMax + m_NbSampleOffset) >= m_BufferSize) Error_Handler();  // Validate buffer size
    }
}

// -----------------------------------------------------------------------------
// Process audio sample with pitch modulation
ITCM float cModulator::Process(float Sample, float Depth)
{
    // Check for valid parameters
    if((Depth > 1.0f) || (m_BufferSize == 0)) {
        return Sample;  // Return input unchanged if invalid parameters
    }

    // Advance DCO step for LFO generation
    m_DCO.Step();

    // Get DCO sine value for modulation (range 0 to +1)
    float DCO = m_DCO.getSineValue();

    // Calculate dynamic delay based on LFO and depth
    float Delay = m_NbSampleOffset + m_SamplesMax + ((m_SamplesMax - m_SamplesMin) * Depth * DCO);

    // Clamp delay to valid buffer range
    if(Delay < 0.0f) {
        Delay = 0.0f;
    }
    if(Delay >= m_BufferSize) {
        Delay = m_BufferSize - 1.0f;
    }

    // Process sample through delay line
    m_DelayLine.Push(Sample);                     // Store current sample in delay line
    return m_DelayLine.Pull(Delay);               // Retrieve and return delayed sample
}

// =============================================================================
// Private Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Calculate sample delay based on pitch ratio and LFO frequency
float cModulator::calcSample(float Pitch, float SampleRate, float LFO) const
{
    // Formula: samples = |(1 - PitchRatio) × Fs| / (2π × f_LFO)
    // PitchRatio = PitchPercentage / 100 + 1
    const float TWO_PI = 6.28318530718f;          // 2π constant
    float PitchRatio = (Pitch / 100) + 1;         // Calculate pitch ratio
    float x = ((1 - PitchRatio) * SampleRate);    // Calculate numerator
    if(x < 0) x = -x;                            // Take absolute value
    return x / (TWO_PI * LFO);                   // Return calculated sample delay
}

} // namespace DadDSP

//***End of file**************************************************************
