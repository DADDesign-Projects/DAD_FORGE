// ======================================================================
// ======================================================================
// File: cFadeInOut.h
// Description: Fade-in / fade-out gain generator, sample-accurate. Provides
//              linear and audio (dB-taper) gain queries.
//
// Copyright(c) 2026 DadDesign-Projects
//
// SPDX-License-Identifier: Apache-2.0
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// ======================================================================
// ======================================================================

#pragma once

#include <cstdint>

namespace DadDSP {

//**********************************************************************************
// class cFadeInOut
//**********************************************************************************
// Fade-in / fade-out gain generator, sample-accurate.
//
// Usage pattern (real-time audio callback, called once per sample):
//
//     fade.Process();                     // advance envelope by 1 sample
//     float g = fade.GetGainAudio();      // or GetGainLinear()
//     outputSample = inputSample * g;
//
// Process() is the only method that advances time and updates the current
// linear gain. GetGainLinear() just returns that current gain.
// GetGainAudio() returns the audio (dB-taper) gain computed from that same
// current gain, with no additional state of its own.
//
// Retrigger rules:
//   - FadeIn cut by another FadeIn   -> ignored, current fade continues
//   - FadeOut cut by another FadeOut -> ignored, current fade continues
//   - FadeIn cut by a FadeOut (or vice versa) -> new fade starts immediately
//     from the gain value currently in effect (no jump/click)
class cFadeInOut
{
public:

    // -----------------------------------------------------------------------------
    // Types
    // -----------------------------------------------------------------------------

    enum class EFadeState
    {
        Idle,                   // Nothing triggered yet since construction
        FadeInInProgress,
        FadeOutInProgress,
        FadeInComplete,         // Fade-in finished, gain stable at 1.0f
        FadeOutComplete         // Fade-out finished, gain stable at 0.0f
    };

    // End-of-fade callback signature. userData is whatever pointer was
    // passed to the constructor (avoids heap allocation / std::function overhead)
    typedef void (*FadeCallback)(void* Context, void* userData);

    // -----------------------------------------------------------------------------
    // Construction
    // -----------------------------------------------------------------------------

    cFadeInOut();

    // -----------------------------------------------------------------------------
    // Initialization
    // -----------------------------------------------------------------------------
    void Init(float           fadeInTimeMs,
              float           fadeOutTimeMs,
              float           sampleRate,
              FadeCallback    fadeInEndCallback  = nullptr,
              FadeCallback    fadeOutEndCallback = nullptr,
              void*           callbackContext    = nullptr,
    		  void*           callbackUserData   = nullptr);

    // -----------------------------------------------------------------------------
    // Triggers
    // -----------------------------------------------------------------------------

    // Start a fade-in. No effect if a fade-in is already in progress.
    void TriggerFadeIn();

    // Start a fade-out. No effect if a fade-out is already in progress.
    void TriggerFadeOut();

    // -----------------------------------------------------------------------------
    // Time base
    // -----------------------------------------------------------------------------

    // Advances the envelope by one sample. Call this once per sample at
    // m_SampleRate. Updates the current linear gain, detects fade
    // completion and fires the corresponding end-of-fade callback once.
    void Process();

    // -----------------------------------------------------------------------------
    // Gain queries (read-only, do not advance time)
    // -----------------------------------------------------------------------------

    float GetGainLinear() const { return m_CurrentGain; }
    float GetGainAudio()  const;

    // -----------------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------------

    EFadeState GetFadeState() const { return m_State; }

    // -----------------------------------------------------------------------------
    // Configuration
    // -----------------------------------------------------------------------------

    void SetFadeInTime (float fadeInTimeMs);
    void SetFadeOutTime(float fadeOutTimeMs);

private:

    // -----------------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------------

    static uint32_t MsToSamples(float timeMs, float sampleRate);
    float           ComputeStep(float fullRange, uint32_t durationSamples) const;

    // =============================================================================
    // Configuration
    // =============================================================================

    float        m_SampleRate;
    float        m_FadeInStep;              // Positive step (gain per sample) for full 0→1 fade-in
    float        m_FadeOutStep;             // Negative step (gain per sample) for full 1→0 fade-out
    FadeCallback m_FadeInEndCallback;
    FadeCallback m_FadeOutEndCallback;
    void*		 m_CallbackContext;
    void*        m_CallbackUserData;

    // =============================================================================
    // Runtime state
    // =============================================================================

    EFadeState   m_State;
    float        m_CurrentGain;
    float        m_CurrentStep;             // Active step used by Process() (0 when idle/complete)
};

} // DadDSP

//***End of file**************************************************************
