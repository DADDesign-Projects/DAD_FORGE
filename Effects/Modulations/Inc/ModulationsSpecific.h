//==============================================================================
// File        : ModulationsSpecific.h
// Description :
// Hardware and application configuration definitions used by the
// DAD_FORGE library.
//
// Copyright (c) 2026 DadDesign-Projects.
//==============================================================================
#pragma once
//**********************************************************************************
// Audio Manager
//**********************************************************************************
#define AUDIO_BUFFER_SIZE  4        // Audio buffer size in samples
#define SAMPLING_RATE      48000.0f // Audio sampling rate in Hz

// Real-time refresh rate derived from audio parameters, filters, etc.
constexpr float RT_RATE = SAMPLING_RATE / (float)AUDIO_BUFFER_SIZE;
constexpr float RT_TIME = (float)AUDIO_BUFFER_SIZE / SAMPLING_RATE;
