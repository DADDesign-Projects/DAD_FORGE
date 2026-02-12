//==================================================================================
//==================================================================================
// File: BiquadFilter.cpp
// Description: Biquad filter implementation with various filter types
//
// Implementation of coefficient calculations provided in the
// Cookbook formulae for audio EQ biquad filter coefficients
// by Robert Bristow-Johnson  <rbj@audioimagination.com>
//
//==================================================================================
//==================================================================================

#include "BiquadFilter.h"

namespace DadDSP {

//**********************************************************************************
// Filter configuration
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialize filter with parameters
// -----------------------------------------------------------------------------
void cBiQuad::Initialize(float sampleRate, float cutoffFreq, float gainDb, float bandwidth, FilterType type) {
    // Store filter parameters
	m_InvSampleRate = 1.0f / sampleRate; // Sampling rate in second
    m_cutoffFreq = cutoffFreq;    // Cutoff frequency in Hz
    m_gainDb = gainDb;            // Gain in dB
    m_bandwidth = bandwidth;      // Bandwidth parameter
    m_type = type;                // Filter type

    // Calculate filter coefficients
    CalculateParameters();
}

//**********************************************************************************
// Coefficient calculation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Calculate filter coefficients based on current parameters
// -----------------------------------------------------------------------------
void cBiQuad::CalculateParameters() {
    float a0, a1, a2, b0, b1, b2; // Raw filter coefficients

    // Calculate intermediate variables for coefficient computation
    float A = std::pow(10, m_gainDb * 0.025f);                   // Gain in linear scale
    float omega = 2 * kPi * m_cutoffFreq * m_InvSampleRate;      // Angular frequency
    float sn = std::sin(omega);                                  // Sine of omega
    float cs = std::cos(omega);                                  // Cosine of omega
    float alpha = sn * std::sinh(kNaturalLog2 / 2 * m_bandwidth * omega / sn); // Bandwidth parameter
    float beta = std::sqrt(A + A);                               // Intermediate variable for shelving filters

    // Calculate coefficients based on filter type using Cookbook formulae
    switch (m_type) {
    case FilterType::LPF:
    case FilterType::LPF24:
        // Low-pass filter coefficients
        b0 = (1 - cs) * 0.5f;
        b1 = 1 - cs;
        b2 = (1 - cs) * 0.5f;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case FilterType::HPF:
    case FilterType::HPF24:
        // High-pass filter coefficients
        b0 = (1 + cs) * 0.5f;
        b1 = -(1 + cs);
        b2 = (1 + cs) * 0.5f;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case FilterType::BPF:
        // Band-pass filter coefficients
        b0 = alpha;
        b1 = 0;
        b2 = -alpha;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case FilterType::NOTCH:
        // Notch filter coefficients
        b0 = 1;
        b1 = -2 * cs;
        b2 = 1;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case FilterType::PEQ:
        // Peaking EQ filter coefficients
        b0 = 1 + (alpha * A);
        b1 = -2 * cs;
        b2 = 1 - (alpha * A);
        a0 = 1 + (alpha / A);
        a1 = -2 * cs;
        a2 = 1 - (alpha / A);
        break;
    case FilterType::LSH:
        // Low shelf filter coefficients
        b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
        b1 = 2 * A * ((A - 1) - (A + 1) * cs);
        b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
        a0 = (A + 1) + (A - 1) * cs + beta * sn;
        a1 = -2 * ((A - 1) + (A + 1) * cs);
        a2 = (A + 1) + (A - 1) * cs - beta * sn;
        break;
    case FilterType::HSH:
        // High shelf filter coefficients
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
        break;
    case FilterType::AFP:
    default:
        // All-pass filter coefficients
        b0 = 1 - alpha;
        b1 = -2 * cs;
        b2 = 1 + alpha;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    }

    // Precompute normalized coefficients for efficient processing
    // Disable interrupts during coefficient update to ensure thread safety
    float a = 1.0f / a0;
    __disable_irq();
    m_a0 = b0 * a;    // b0 normalized
    m_a1 = b1 * a;    // b1 normalized
    m_a2 = b2 * a;    // b2 normalized
    m_a3 = a1 * a;    // a1 normalized
    m_a4 = a2 * a;    // a2 normalized
    __DMB(); // Data Memory Barrier
    __enable_irq();
}

//**********************************************************************************
// Signal processing
//**********************************************************************************


} // namespace DadDSP

//***End of file**************************************************************
