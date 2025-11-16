//==================================================================================
//==================================================================================
// File: cAudioFader.h
// Description: Audio crossfader class providing smooth transitions between two audio
// sources using equal-power crossfade curves to maintain consistent perceived loudness.
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#include "main.h"

namespace DadDSP {

//**********************************************************************************
// Class: cAudioFader
// Description: Implements equal-power audio crossfading between two audio sources
//**********************************************************************************

class cAudioFader
{
public:
    // -----------------------------------------------------------------------------
    // Fade State Enumeration
    // Defines possible states of the audio fade process
    //--------------------------------------------------------------------------------
    enum FadeState
    {
        NO_FADE = 0,        // No fade active - steady state
        FADING_IN_B,        // Transition from source A to source B
        FADING_OUT_A        // Transition from source B to source A
    };

    //--------------------------------------------------------------------------------
    // Initializes the audio fader with sample rate and fade duration
    //--------------------------------------------------------------------------------
    void Initialize(uint32_t sampleRate, float fadeTimeSeconds);

    //--------------------------------------------------------------------------------
    // Starts fade transition from source A to source B
    //--------------------------------------------------------------------------------
    void startFadeInB();

    //--------------------------------------------------------------------------------
    // Starts fade transition from source B to source A
    //--------------------------------------------------------------------------------
    void startFadeOutA();

    //--------------------------------------------------------------------------------
    // Processes audio samples with crossfade between inputs A and B
    //--------------------------------------------------------------------------------
    void Process(float inputA1, float inputA2, float inputB1, float inputB2,
                 float& output1, float& output2);

    //--------------------------------------------------------------------------------
    // Returns current fade progress as normalized value [0.0, 1.0]
    //--------------------------------------------------------------------------------
    float getProgress() const;

    //--------------------------------------------------------------------------------
    // Returns current fade state
    //--------------------------------------------------------------------------------
    FadeState getState() const;

    //--------------------------------------------------------------------------------
    // Returns last completed fade state
    //--------------------------------------------------------------------------------
    FadeState getLastState() const;

    //--------------------------------------------------------------------------------
    // Checks if a fade operation is currently active
    //--------------------------------------------------------------------------------
    bool isFading() const { return (m_state != NO_FADE); }

private:
    // =============================================================================
    // Private Member Variables Section
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Fade Configuration
    // -----------------------------------------------------------------------------
    uint32_t m_sampleRate;          // Audio sample rate in Hz
    float    m_fadeTimeSeconds;     // Fade duration in seconds
    int      m_totalFadeSamples;    // Total number of samples for complete fade

    // -----------------------------------------------------------------------------
    // Fade State Tracking
    // -----------------------------------------------------------------------------
    int      m_currentFadeSample;   // Current sample position in fade sequence
    FadeState m_state;              // Current active fade state
    FadeState m_lastState;          // Last completed fade state for steady state
};

} // namespace DadDSP

//***End of file**************************************************************
