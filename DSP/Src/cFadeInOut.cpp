// ======================================================================
// ======================================================================
// File: cFadeInOut.cpp
// Description: Implementation of the cFadeInOut class.
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

#include "cFadeInOut.h"

#include <cmath>

namespace
{
    // Floor used for the dB-based "audio" curve. -60 dB is inaudible in
    // practice; adjust here if you need a different taper depth.
    constexpr float kMinDb     = -60.0f;
    const float     kFloorGain = powf(10.0f, kMinDb / 20.0f);
}

namespace DadDSP {

//==============================================================================
// Construction
//==============================================================================

cFadeInOut::cFadeInOut()
    : m_SampleRate(48000.0f)
    , m_FadeInStep(0.0f)
    , m_FadeOutStep(0.0f)
    , m_FadeInEndCallback(nullptr)
    , m_FadeOutEndCallback(nullptr)
    , m_CallbackUserData(nullptr)
    , m_State(EFadeState::Idle)
    , m_CurrentGain(0.0f)
    , m_CurrentStep(0.0f)
{
    // Default 1 s fades at 48 kHz
    SetFadeInTime(1000.0f);
    SetFadeOutTime(1000.0f);
}

// -----------------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------------

void cFadeInOut::Init(float        fadeInTimeMs,
                      float        fadeOutTimeMs,
                      float        sampleRate,
                      FadeCallback fadeInEndCallback,
                      FadeCallback fadeOutEndCallback,
		              void*        callbackContext,
		    		  void*        callbackUserData){
    m_SampleRate         = sampleRate;
    m_FadeInEndCallback  = fadeInEndCallback;
    m_FadeOutEndCallback = fadeOutEndCallback;
    m_CallbackContext	 = callbackContext;
    m_CallbackUserData   = callbackUserData;

    m_State        = EFadeState::Idle;
    m_CurrentGain  = 0.0f;
    m_CurrentStep  = 0.0f;

    SetFadeInTime(fadeInTimeMs);
    SetFadeOutTime(fadeOutTimeMs);
}

//==============================================================================
// Triggers
//==============================================================================

void cFadeInOut::TriggerFadeIn()
{
    // Fade-in cut by another fade-in: ignored
    if (m_State == EFadeState::FadeInInProgress)
        return;

    // Zero-duration or already at target → snap immediately
    if (m_FadeInStep <= 0.0f || m_CurrentGain >= 1.0f)
    {
        m_CurrentGain = 1.0f;
        m_CurrentStep = 0.0f;
        m_State       = EFadeState::FadeInComplete;

        if (m_FadeInEndCallback != nullptr)
            m_FadeInEndCallback(m_CallbackContext, m_CallbackUserData);

        return;
    }

    m_CurrentStep = m_FadeInStep;
    m_State       = EFadeState::FadeInInProgress;
}

void cFadeInOut::TriggerFadeOut()
{
    // Fade-out cut by another fade-out: ignored
    if (m_State == EFadeState::FadeOutInProgress)
        return;

    // Zero-duration or already at target → snap immediately
    if (m_FadeOutStep >= 0.0f || m_CurrentGain <= 0.0f)
    {
        m_CurrentGain = 0.0f;
        m_CurrentStep = 0.0f;
        m_State       = EFadeState::FadeOutComplete;

        if (m_FadeOutEndCallback != nullptr)
            m_FadeOutEndCallback(m_CallbackContext, m_CallbackUserData);

        return;
    }

    m_CurrentStep = m_FadeOutStep;
    m_State       = EFadeState::FadeOutInProgress;
}

//==============================================================================
// Time base
//==============================================================================

void cFadeInOut::Process()
{
    if (m_State != EFadeState::FadeInInProgress && m_State != EFadeState::FadeOutInProgress)
        return;

    m_CurrentGain += m_CurrentStep;

    if (m_State == EFadeState::FadeInInProgress)
    {
        if (m_CurrentGain >= 1.0f)
        {
            m_CurrentGain = 1.0f;
            m_CurrentStep = 0.0f;
            m_State       = EFadeState::FadeInComplete;

            if (m_FadeInEndCallback != nullptr)
                m_FadeInEndCallback(m_CallbackContext, m_CallbackUserData);
        }
    }
    else // FadeOutInProgress
    {
        if (m_CurrentGain <= 0.0f)
        {
            m_CurrentGain = 0.0f;
            m_CurrentStep = 0.0f;
            m_State       = EFadeState::FadeOutComplete;

            if (m_FadeOutEndCallback != nullptr)
                m_FadeOutEndCallback(m_CallbackContext, m_CallbackUserData);
        }
    }
}

//==============================================================================
// Gain queries
//==============================================================================

float cFadeInOut::GetGainAudio() const
{
	static float memCurrentGain = 0.0f;
	static float memAudioGain = 0.0f;
	if(memCurrentGain != m_CurrentGain){
		memCurrentGain = m_CurrentGain;
		// Constant-rate-of-change-in-dB taper, normalized to [0, 1]
		float linearAtGain = powf(10.0f, (kMinDb * (1.0f - m_CurrentGain)) / 20.0f);
		memAudioGain =  (linearAtGain - kFloorGain) / (1.0f - kFloorGain);
	}
	return memAudioGain;
}

//==============================================================================
// Internal helpers
//==============================================================================

uint32_t cFadeInOut::MsToSamples(float timeMs, float sampleRate)
{
    if (timeMs <= 0.0f)
        return 0;

    return static_cast<uint32_t>((timeMs * 0.001f) * sampleRate + 0.5f);
}

float cFadeInOut::ComputeStep(float fullRange, uint32_t durationSamples) const
{
    if (durationSamples == 0)
        return 0.0f;   // special-cased in Trigger*

    return fullRange / static_cast<float>(durationSamples);
}

//==============================================================================
// Configuration helpers
//==============================================================================

void cFadeInOut::SetFadeInTime(float fadeInTimeMs)
{
    const uint32_t samples = MsToSamples(fadeInTimeMs, m_SampleRate);
    m_FadeInStep = ComputeStep(+1.0f, samples);
}

void cFadeInOut::SetFadeOutTime(float fadeOutTimeMs)
{
    const uint32_t samples = MsToSamples(fadeOutTimeMs, m_SampleRate);
    m_FadeOutStep = ComputeStep(-1.0f, samples);
}

} // DadDSP

//***End of file**************************************************************
