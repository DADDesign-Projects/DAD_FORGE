//==================================================================================
//==================================================================================
// File: cModulator.cpp
// Description:
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "cModulator2.h"

namespace DadDSP {

//**********************************************************************************
// cModulator
//**********************************************************************************

void cModulator2::Initialize(float sampleRate, float* pBuffer, uint32_t BufferSize, float Frequence, float PitchVariation){
	m_BufferSize = BufferSize;
	m_DelayLine.Initialize(pBuffer, BufferSize);  // Initialize delay line
	m_DelayLine.Clear();                          // Clear delay line buffer
	m_DCO.Initialize(sampleRate, 0.0f, 0.01f, 10.0f, 0.5f);  // Initialize DCO
	m_PitchVariation = PitchVariation;
	setFreq(Frequence);
	m_PitchVariation = PitchVariation;
	calcCoefPitch();
}

// Calculate pitch compensation coefficient
// This ensures constant pitch deviation regardless of LFO frequency
void cModulator2::calcCoefPitch(){
	if(m_Frequence > 0.0f) {
		// For constant pitch variation:
		// depth_samples = PitchVariation_samples_per_sec / (2π × freq_Hz)
		// PitchVariation is in samples/sec (corresponds directly to pitch shift)
		// Typical values: 3-8 samples/sec for natural vibrato

		const float TWO_PI = 6.28318530718f;

		// Calculate required modulation amplitude in samples
		// to achieve constant pitch variation
		m_CoefPitch = m_PitchVariation / (2 * TWO_PI * m_Frequence);

	} else {
		m_CoefPitch = 0.0f;
	}
}

// Set LFO frequency and recalculate pitch compensation
void cModulator2::setFreq(float Freq){
	if(Freq != m_Frequence && Freq > 0.0f) {
		m_Frequence = Freq;
		m_DCO.setFreq(Freq);
		calcCoefPitch();  // Recalculate coefficient when frequency changes
	}
}

// Set pitch variation amount (in samples/sec)
// Typical values:
//   - 3.0 : subtle vibrato
//   - 5.0 : moderate vibrato
//   - 8.0 : pronounced vibrato
void cModulator2::setPitchVariation(float PitchVar){
	if(PitchVar != m_PitchVariation) {
		m_PitchVariation = PitchVar;
		calcCoefPitch();  // Recalculate coefficient when variation changes
	}
}

ITCM float cModulator2::Process(float Sample, float Depth){
    // Check for valid parameters
    if((Depth > 1.0f) || (m_BufferSize == 0)) {
        return Sample;  // Return input unchanged if invalid
    }

    // Advance DCO step
    m_DCO.Step();

    // Get DCO sine value for modulation (-1 to +1)
    float DCO = m_DCO.getSineValue();

    // Calculate delay with pitch compensation
    // Center delay + modulation with pitch-compensated amplitude
    float Delay = m_BufferSize * (0.5f + (DCO * m_CoefPitch * Depth));

    // Clamp delay to valid range
    if(Delay < 0.0f) Delay = 0.0f;
    if(Delay >= m_BufferSize) Delay = m_BufferSize - 1.0f;

    // Process sample through delay line
    m_DelayLine.Push(Sample);                    // Store current sample
    return m_DelayLine.Pull((int32_t)Delay);     // Retrieve delayed sample
}

} // namespace DadDSP
//***End of file**************************************************************
