//==================================================================================
//==================================================================================
// File: cAllPass.h
// Description: First-order all-pass filter implementation
//
// Copyright (c) 2024 DadDSP.
//==================================================================================
//==================================================================================

#pragma once
#include "math.h"

namespace DadDSP {

//**********************************************************************************
// All-pass filter state structure
//**********************************************************************************
struct sAPFState {
    float x1 = 0.0f;  // Previous input sample
    float y1 = 0.0f;  // Previous output sample
};

//**********************************************************************************
// First-order all-pass filter class
//**********************************************************************************
class cAllPass {
public:
    // -----------------------------------------------------------------------------
    // Public methods
    // -----------------------------------------------------------------------------

    // Brief: Initializes filter with specified sample rate
    void Initialize(float sampleRate) {
        m_sampleRate = sampleRate;
    }

    // Brief: Sets cutoff frequency and recalculates filter coefficient
    inline void SetFrequency(float freq) {
        // Calculate coefficient using bilinear transform
        float k = tanf(M_PI * freq / m_sampleRate);
        m_a = (1.0f - k) / (1.0f + k);
    }

    // Brief: Processes single sample through all-pass filter
    inline float Process(float x, sAPFState &s) {
        // Apply all-pass filter difference equation: y[n] = -a*x[n] + x[n-1] + a*y[n-1]
        float y = -m_a * x + s.x1 + m_a * s.y1;

        // Update filter state for next iteration
        s.x1 = x;
        s.y1 = y;

        return y;
    }

private:
    // -----------------------------------------------------------------------------
    // Private member variables
    // -----------------------------------------------------------------------------
    float m_sampleRate = 48000.0f;  // Current sample rate in Hz
    float m_a = 0.0f;               // All-pass filter coefficient
};

} // namespace DadDSP

//***End of file**************************************************************
