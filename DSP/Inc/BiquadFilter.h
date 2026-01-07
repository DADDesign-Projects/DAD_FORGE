//==================================================================================
//==================================================================================
// File: BiquadFilter.h
// Description: Biquad filter class declaration with various filter types
//
// Implementation of coefficient calculations provided in the
// Cookbook formulae for audio EQ biquad filter coefficients
// by Robert Bristow-Johnson  <rbj@audioimagination.com>
//
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "Sections.h"
#include <cstdint>
#include <cmath>

namespace DadDSP {

//**********************************************************************************
// Constants and enumerations
//**********************************************************************************

constexpr float kNaturalLog2 = 0.69314718055994530942;  // Natural logarithm of 2
constexpr float kPi = 3.14159265358979323846;           // Value of PI

// -----------------------------------------------------------------------------
// Filter types enumeration
// -----------------------------------------------------------------------------
enum class FilterType : int {
    LPF = 0,   // Low pass filter
    LPF24,     // Low pass filter 24 dB
    HPF,       // High pass filter
    HPF24,     // High pass filter 24 dB
    BPF,       // Band pass filter
    NOTCH,     // Notch Filter
    PEQ,       // Peaking band EQ filter
    LSH,       // Low shelf filter
    HSH,       // High shelf filter
	AFP		   // All-pass filter
};

//**********************************************************************************
// Data structures
//**********************************************************************************

// -----------------------------------------------------------------------------
// Filter state structure for storing delay line values
// -----------------------------------------------------------------------------
struct sFilterState {
    float x1; // Previous input sample x[n-1]
    float x2; // Input sample x[n-2]
    float y1; // Previous output sample y[n-1]
    float y2; // Output sample y[n-2]
};

// -----------------------------------------------------------------------------
// Audio channel enumeration
// -----------------------------------------------------------------------------
enum class eChannel {
    Left,   // Left audio channel
    Right   // Right audio channel
};

//**********************************************************************************
// Biquad filter class
//**********************************************************************************

class cBiQuad {
public:
    // =============================================================================
    // Public interface
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Filter configuration
    // -----------------------------------------------------------------------------
    void Initialize(float sampleRate, float cutoffFreq, float gainDb, float bandwidth, FilterType type);

    // -----------------------------------------------------------------------------
    // Coefficient calculation
    // -----------------------------------------------------------------------------
    void CalculateParameters();

    // -----------------------------------------------------------------------------
    // Frequency response analysis
    // -----------------------------------------------------------------------------
    float GainDb(float freq);

    // -----------------------------------------------------------------------------
    // Signal processing - stereo channel processing
    // -----------------------------------------------------------------------------
    inline float Process(float sample, eChannel Channel) {
        // Process sample through appropriate filter chain based on channel
        switch(Channel) {
        case eChannel::Left:
            // For 24dB filters, cascade two biquad stages for left channel
            if ((m_type == FilterType::LPF24) || (m_type == FilterType::HPF24)) {
                sample = Process(sample, m_FilterState[0]); // First stage
            }
            return Process(sample, m_FilterState[1]);       // Second stage/main stage
            break;

        case eChannel::Right:
            // For 24dB filters, cascade two biquad stages for right channel
            if ((m_type == FilterType::LPF24) || (m_type == FilterType::HPF24)) {
                sample = Process(sample, m_FilterState[2]); // First stage
            }
            return Process(sample, m_FilterState[3]);       // Second stage/main stage
            break;
        }
        return sample; // Return original sample if no processing applied
    }

    // =============================================================================
    // Parameter setters
    // =============================================================================
    inline void setSampleRate(float sampleRate) { m_sampleRate = sampleRate; }        // Set sampling rate
    inline void setCutoffFreq(float cutoffFreq) { m_cutoffFreq = cutoffFreq; }        // Set cutoff frequency
    inline void setGainDb(float gainDb) { m_gainDb = gainDb; }                        // Set gain in dB
    inline void setBandwidth(float bandwidth) { m_bandwidth = bandwidth; }            // Set bandwidth
    inline void setType(FilterType type) { m_type = type; }                           // Set filter type

    // =============================================================================
    // Parameter getters
    // =============================================================================
    inline float getSampleRate() { return m_sampleRate; }                             // Get sampling rate
    inline float getCutoffFreq() { return m_cutoffFreq; }                             // Get cutoff frequency
    inline float getGainDb() { return m_gainDb; }                                     // Get gain in dB
    inline float getBandwidth() { return m_bandwidth; }                               // Get bandwidth
    inline FilterType getType() { return m_type; }                                    // Get filter type

protected:
    // =============================================================================
    // Protected methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Mono channel signal processing
    // -----------------------------------------------------------------------------
    ITCM float Process(float sample, sFilterState &FilterState);

    // =============================================================================
    // Filter parameters
    // =============================================================================
    float m_sampleRate;        // Sampling rate in Hz
    float m_cutoffFreq;        // Cutoff frequency in Hz
    float m_gainDb;            // Gain in dB
    float m_bandwidth;         // Bandwidth parameter
    FilterType m_type;         // Filter type

    // =============================================================================
    // Filter coefficients
    // =============================================================================
    volatile float m_a0 = 0;            // b0 coefficient normalized
    volatile float m_a1 = 0;            // b1 coefficient normalized
    volatile float m_a2 = 0;            // b2 coefficient normalized
    volatile float m_a3 = 0;            // a1 coefficient normalized
    volatile float m_a4 = 0;            // a2 coefficient normalized

    // =============================================================================
    // Filter state storage
    // =============================================================================
    sFilterState m_FilterState[4] = {}; // Filter states: [0]=Left stage1, [1]=Left stage2,
                                       // [2]=Right stage1, [3]=Right stage2
};

} // namespace DadDSP

//***End of file**************************************************************
