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
    m_sampleRate = sampleRate;    // Sampling rate in Hz
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
    float A = std::pow(10, m_gainDb / 40);                       // Gain in linear scale
    float omega = 2 * kPi * m_cutoffFreq / m_sampleRate;         // Angular frequency
    float sn = std::sin(omega);                                  // Sine of omega
    float cs = std::cos(omega);                                  // Cosine of omega
    float alpha = sn * std::sinh(kNaturalLog2 / 2 * m_bandwidth * omega / sn); // Bandwidth parameter
    float beta = std::sqrt(A + A);                               // Intermediate variable for shelving filters

    // Calculate coefficients based on filter type using Cookbook formulae
    switch (m_type) {
    case FilterType::LPF:
    case FilterType::LPF24:
        // Low-pass filter coefficients
        b0 = (1 - cs) / 2;
        b1 = 1 - cs;
        b2 = (1 - cs) / 2;
        a0 = 1 + alpha;
        a1 = -2 * cs;
        a2 = 1 - alpha;
        break;
    case FilterType::HPF:
    case FilterType::HPF24:
        // High-pass filter coefficients
        b0 = (1 + cs) / 2;
        b1 = -(1 + cs);
        b2 = (1 + cs) / 2;
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
    default:
        // High shelf filter coefficients
        b0 = A * ((A + 1) + (A - 1) * cs + beta * sn);
        b1 = -2 * A * ((A - 1) + (A + 1) * cs);
        b2 = A * ((A + 1) + (A - 1) * cs - beta * sn);
        a0 = (A + 1) - (A - 1) * cs + beta * sn;
        a1 = 2 * ((A - 1) - (A + 1) * cs);
        a2 = (A + 1) - (A - 1) * cs - beta * sn;
        break;
    }

    // Precompute normalized coefficients for efficient processing
    // Disable interrupts during coefficient update to ensure thread safety
    __disable_irq();
    m_a0 = b0 / a0;    // b0 normalized
    m_a1 = b1 / a0;    // b1 normalized
    m_a2 = b2 / a0;    // b2 normalized
    m_a3 = a1 / a0;    // a1 normalized
    m_a4 = a2 / a0;    // a2 normalized
    __enable_irq();
}

//**********************************************************************************
// Frequency response analysis
//**********************************************************************************

// -----------------------------------------------------------------------------
// Calculate filter gain at specified frequency
// -----------------------------------------------------------------------------
float cBiQuad::GainDb(float freq) {
    // Calculate intermediate variable for frequency response
    float Phi = 4 * std::sin(std::pow(kPi * freq / m_sampleRate, 2.0));

    // Calculate numerator and denominator of transfer function
    float num = ((m_a0 * m_a2 * (Phi * Phi)) + std::pow(m_a0 + m_a1 + m_a2, 2.0) - (((m_a0 * m_a1) + (4 * m_a0 * m_a2) + (m_a1 * m_a2)) * Phi));
    float denum = (m_a4 * std::pow(Phi, 2.0)) + std::pow(m_a3 + m_a4 + 1, 2.0) - (((m_a3 * m_a4) + m_a3 + (4 * +m_a4)) * Phi);

    // Calculate gain in dB, with special handling for 2nd order filters
    if ((m_type == FilterType::LPF24) || (m_type == FilterType::HPF24)) {
        return (20 * std::log10(std::sqrt(num / denum))) + (20 * log10(std::sqrt(num / denum)));
    }
    else {
        return 20 * std::log10(std::sqrt(num / denum));
    }
}

//**********************************************************************************
// Signal processing
//**********************************************************************************

// -----------------------------------------------------------------------------
// Process single sample through biquad filter
// -----------------------------------------------------------------------------
float cBiQuad::Process(float sample, sFilterState &FilterState) {
    // Apply biquad filter difference equation:
    // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    float result = (m_a0 * sample)
                 + (m_a1 * FilterState.x1)
                 + (m_a2 * FilterState.x2)
                 - (m_a3 * FilterState.y1)
                 - (m_a4 * FilterState.y2);

    // Update filter state for next iteration
    // Shift input delay line
    FilterState.x2 = FilterState.x1;    // Move x[n-1] to x[n-2]
    FilterState.x1 = sample;            // Store current input as x[n-1]

    // Shift output delay line
    FilterState.y2 = FilterState.y1;    // Move y[n-1] to y[n-2]
    FilterState.y1 = result;            // Store current output as y[n-1]

    return result;
}

} // namespace DadDSP

//***End of file**************************************************************
