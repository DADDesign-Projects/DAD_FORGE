//==================================================================================
//==================================================================================
// File: cAudioFader.cpp
// Description: Audio crossfader implementing equal-power fade curves for smooth transitions
// between two audio sources while maintaining consistent perceived loudness.
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cAudioFader.h"
#include <cmath>
namespace DadDSP {

//**********************************************************************************
// Class: cAudioFader
// Description: Handles smooth audio transitions between two sources using equal-power curves
//**********************************************************************************

//--------------------------------------------------------------------------------
// Initializes the audio fader with sample rate and fade duration
//--------------------------------------------------------------------------------
void cAudioFader::Initialize(uint32_t sampleRate, float fadeTimeSeconds) {
    m_sampleRate        = sampleRate;           // Audio sample rate in Hz
    m_fadeTimeSeconds   = fadeTimeSeconds;      // Fade duration in seconds
    m_totalFadeSamples  = static_cast<int>(m_sampleRate * m_fadeTimeSeconds);  // Total samples for fade
    m_currentFadeSample = 0;                    // Current fade position
    m_state             = NO_FADE;              // Current fade state
    m_lastState         = FADING_OUT_A;         // Last completed fade state
}

//--------------------------------------------------------------------------------
// Starts fade transition from source A to source B
//--------------------------------------------------------------------------------
void cAudioFader::startFadeInB() {
    m_currentFadeSample = 0;    // Reset fade counter
    m_state = FADING_IN_B;      // Set state to fading in B
}

//--------------------------------------------------------------------------------
// Starts fade transition from source B to source A
//--------------------------------------------------------------------------------
void cAudioFader::startFadeOutA() {
    m_currentFadeSample = 0;    // Reset fade counter
    m_state = FADING_OUT_A;     // Set state to fading out A
}

//--------------------------------------------------------------------------------
// Processes audio samples with crossfade between inputs A and B
//--------------------------------------------------------------------------------
void cAudioFader::Process(float inputA1, float inputA2, float inputB1, float inputB2,
                         float& output1, float& output2) {
    float gainA = 1.0f;     // Gain for source A
    float gainB = 0.0f;     // Gain for source B

    // Handle active fade states
    if (m_state != NO_FADE) {
        // Calculate fade progress with clamping
        float progress = static_cast<float>(m_currentFadeSample) / static_cast<float>(m_totalFadeSamples);
        if (progress < 0) progress = 0;
        if (progress > 1) progress = 1;

        // Apply equal-power crossfade curves
        if (m_state == FADING_IN_B) {
            // Fade from A to B: A decreases, B increases
            gainA = std::cos(progress * static_cast<float>(M_PI_2));  // Cosine fade: 1.0 → 0.0
            gainB = std::sin(progress * static_cast<float>(M_PI_2));  // Sine fade: 0.0 → 1.0
        }
        else if (m_state == FADING_OUT_A) {
            // Fade from B to A: A increases, B decreases
            gainA = std::sin(progress * static_cast<float>(M_PI_2));  // Sine fade: 0.0 → 1.0
            gainB = std::cos(progress * static_cast<float>(M_PI_2));  // Cosine fade: 1.0 → 0.0
        }

        // Update fade position and check for completion
        m_currentFadeSample++;
        if (m_currentFadeSample >= m_totalFadeSamples) {
            // Fade completed - set final gain values and update state
            if (m_state == FADING_IN_B) {
                gainA = 0.0f;
                gainB = 1.0f;
                m_lastState = FADING_IN_B;
            }
            else if (m_state == FADING_OUT_A) {
                gainA = 1.0f;
                gainB = 0.0f;
                m_lastState = FADING_OUT_A;
            }
            m_state = NO_FADE;  // Transition to no-fade state
        }
    }
    else {
        // No active fade - maintain final state from last transition
        if (m_lastState == FADING_IN_B) {
            // After fade to B: only source B active
            gainA = 0.0f;
            gainB = 1.0f;
        }
        else {
            // After fade to A or initial state: only source A active
            gainA = 1.0f;
            gainB = 0.0f;
        }
    }

    // Mix both audio sources with calculated gains
    output1 = inputA1 * gainA + inputB1 * gainB;
    output2 = inputA2 * gainA + inputB2 * gainB;
}

//--------------------------------------------------------------------------------
// Returns current fade progress as normalized value [0.0, 1.0]
//--------------------------------------------------------------------------------
float cAudioFader::getProgress() const {
    if (m_totalFadeSamples == 0) return 0.0f;  // Prevent division by zero

    float progress = static_cast<float>(m_currentFadeSample) / static_cast<float>(m_totalFadeSamples);
    if (progress < 0) progress = 0;    // Clamp to minimum
    if (progress > 1) progress = 1;    // Clamp to maximum
    return progress;
}

//--------------------------------------------------------------------------------
// Returns current fade state
//--------------------------------------------------------------------------------
cAudioFader::FadeState cAudioFader::getState() const {
    return m_state;     // Return current state
}

//--------------------------------------------------------------------------------
// Returns last completed fade state
//--------------------------------------------------------------------------------
cAudioFader::FadeState cAudioFader::getLastState() const {
    return m_lastState; // Return last state
}

} // namespace DadDSP

//***End of file**************************************************************
