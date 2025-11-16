//==================================================================================
//==================================================================================
// File: cModulator.h
// Description:
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "main.h"
#include "cDCO.h"
#include "cDelayLine.h"

namespace DadDSP {

//**********************************************************************************
// cModulator Class Declaration
//**********************************************************************************

class cModulator2 {
public:
    // =============================================================================
    // Constructor/Destructor
    // =============================================================================
    cModulator2() = default;
    ~cModulator2() = default;

    // =============================================================================
    // Public Methods
    // =============================================================================

    void Initialize(float sampleRate, float* pBuffer, uint32_t BufferSize, float Frequence, float PitchVariation);

    void calcCoefPitch();

    void setFreq(float Freq);

    void setPitchVariation(float PitchVar);

    ITCM float Process(float Sample, float Depth);

protected:
    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    cDCO 		m_DCO;                   // Digital Controlled Oscillator

    float 		m_Frequence;
    float 		m_PitchVariation;

    cDelayLine 	m_DelayLine;        	// Delay line for modulation effects
    uint32_t 	m_BufferSize = 0;     	// Size of delay buffer

    float 		m_CoefPitch;            // Compensation coefficient for delay calculation
};

} // namespace DadDSP
//***End of file**************************************************************
