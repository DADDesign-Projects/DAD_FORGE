//==================================================================================
//==================================================================================
// File: cAllPass.h
// Description: First-order and second-order all-pass filter implementations
//
// Copyright (c) 2026 Dad Design
//==================================================================================
//==================================================================================

#pragma once
#include "math.h"

namespace DadDSP {

//==================================================================================
//
// First-order all-pass filter
//
//==================================================================================

//**********************************************************************************
// All-pass filter state structure
// Stores previous input/output samples
//**********************************************************************************
struct sAPFState {
    float x1 = 0.0f;  // Previous input sample (n-1)
    float y1 = 0.0f;  // Previous output sample (n-1)

    // -----------------------------------------------------------------------------
    // Resets filter state
    void Reset(){
        x1 = 0.0f;
        y1 = 0.0f;
    }
};

//**********************************************************************************
// First-order all-pass filter class
//**********************************************************************************
class cAllPass {
public:
    // -----------------------------------------------------------------------------
    // Public methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Initializes filter with given sample rate
    // Optionally attaches an external state structure
    void Initialize(float sampleRate, sAPFState *pState=nullptr) {
        m_sampleRate = sampleRate;
        m_pState = pState;
        if(m_pState){
        	m_pState->Reset();
        }
    }

    // -----------------------------------------------------------------------------
    // Sets cutoff frequency and updates filter coefficient
    inline void SetFrequency(float freq) {
        // Clamp frequency to a safe range
        if (freq < 1.0f) freq = 1.0f;

        // Compute coefficient using bilinear transform
        float k = tanf(M_PI * freq / m_sampleRate);
        m_a = (1.0f - k) / (1.0f + k);
    }

    // -----------------------------------------------------------------------------
    // Processes one sample using provided state
    inline float Process(float x, sAPFState &s) {
        // First-order all-pass difference equation:
        // y[n] = -a*x[n] + x[n-1] + a*y[n-1]
        float y = -m_a * x + s.x1 + m_a * s.y1;

        // Update state
        s.x1 = x;
        s.y1 = y;

        return y;
    }

    // -----------------------------------------------------------------------------
    // Processes one sample using internal state (duplicate overload)
    inline float Process(float x) {
    	if(m_pState){
    		return Process(x, *m_pState);
    	}else{
    		return 0.0f;
    	}
    }

    // -----------------------------------------------------------------------------
    // Reset sate
    inline void ResetState(){
    	if(m_pState){
    		m_pState->Reset();
    	}
    }

private:
    // -----------------------------------------------------------------------------
    // Private member variables
    // -----------------------------------------------------------------------------
    float 		m_sampleRate = 48000.0f;  // Sample rate in Hz
    float 		m_a = 0.0f;               // All-pass coefficient
    sAPFState* 	m_pState = nullptr;       // Optional external state pointer
};

//==================================================================================
//
// Second-order all-pass filter
//
//==================================================================================

//**********************************************************************************
// Second-order all-pass filter state structure
//**********************************************************************************
struct sAPF2State {
    float x1 = 0.0f;  // Input sample (n-1)
    float x2 = 0.0f;  // Input sample (n-2)
    float y1 = 0.0f;  // Output sample (n-1)
    float y2 = 0.0f;  // Output sample (n-2)

    // -----------------------------------------------------------------------------
    // Resets filter state
    void Reset(){
        x1 = 0.0f;
        x2 = 0.0f;
        y1 = 0.0f;
        y2 = 0.0f;
    }
};

//**********************************************************************************
// Second-order all-pass filter class
//**********************************************************************************
class cAllPass2 {
public:
    // -----------------------------------------------------------------------------
    // Public methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
	// Initializes filter with sample rate and default Q
    // Optionally attaches an external state structure
    void Initialize(float sampleRate, sAPF2State* pState=nullptr) {
        m_sampleRate = sampleRate;
        m_Q = 0.707f;
        m_pState = pState;
        if(m_pState){
        	m_pState->Reset();
        }
    }

    // -----------------------------------------------------------------------------
    // Sets center frequency and Q factor
    // Recomputes filter coefficients
    inline void SetParameters(float freq, float Q) {
        // Clamp frequency
        if (freq < 1.0f) freq = 1.0f;

        // Clamp Q for stability
        if (Q < 0.1f) Q = 0.1f;
        if (Q > 20.0f) Q = 20.0f;

        m_Q = Q;

        // Compute angular frequency
        float w0 = 2.0f * M_PI * freq / m_sampleRate;
        float sinw0 = sinf(w0);
        float cosw0 = cosf(w0);
        float alpha = sinw0 / (2.0f * Q);

        // Normalization
        float a0 = 1.0f + alpha;
        float a0_inv = 1.0f / a0;

        // Second-order all-pass coefficients
        m_a1 = -2.0f * cosw0 * a0_inv;
        m_a2 = (1.0f - alpha) * a0_inv;
    }

    // -----------------------------------------------------------------------------
    // Sets center frequency only (keeps current Q)
    inline void SetFrequency(float freq) {
        SetParameters(freq, m_Q);
    }

    // -----------------------------------------------------------------------------
    // Sets Q factor only (keeps current frequency)
    inline void SetQ(float Q) {
        SetParameters(m_freq, Q);
    }

    // -----------------------------------------------------------------------------
    // Processes one sample using provided state
    inline float Process(float x, sAPF2State &s) {
        // Second-order all-pass difference equation:
        // y[n] = -a1*y[n-1] - a2*y[n-2] + a2*x[n] + a1*x[n-1] + x[n-2]
        float y = -m_a1 * s.y1 - m_a2 * s.y2 + m_a2 * x + m_a1 * s.x1 + s.x2;

        // Update state
        s.x2 = s.x1;
        s.x1 = x;
        s.y2 = s.y1;
        s.y1 = y;

        return y;
    }

    // -----------------------------------------------------------------------------
    // Processes one sample using internal state (if available)
    inline float Process(float x) {
    	if(m_pState){
    		return Process(x, *m_pState);
    	}else{
    		return 0.0f;
    	}
    }

    // -----------------------------------------------------------------------------
    // Reset sate
    inline void ResetState(){
    	if(m_pState){
    		m_pState->Reset();
    	}
    }

private:
    // -----------------------------------------------------------------------------
    // Private member variables
    // -----------------------------------------------------------------------------
    float 		m_sampleRate = 48000.0f;  // Sample rate in Hz
    float 		m_Q = 0.707f;             // Q factor
    float 		m_freq = 1000.0f;         // Center frequency
    float 		m_a1 = 0.0f;              // All-pass coefficient a1
    float 		m_a2 = 0.0f;              // All-pass coefficient a2
    sAPF2State* m_pState=nullptr;		  // Optional external state pointer
};

} // namespace DadDSP

//***End of file**************************************************************
