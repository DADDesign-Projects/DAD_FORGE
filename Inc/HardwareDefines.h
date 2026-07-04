//==============================================================================
// File        : HardwareDefines.h
// Description :
// Default hardware and application configuration definitions used by the
// DAD_FORGE library.
//
// This file is part of the library and should not be modified directly.
//
// To customize the hardware configuration for a specific application, copy
// this file into your project directory and rename it to
// 'HardwareAndCoDefines.h'. The library will automatically use this file
// instead of the default one.
//
// Copyright (c) 2026 DadDesign-Projects.
//==============================================================================
#pragma once

#if __has_include("HardwareAndCoDefines.h")
#include "HardwareAndCoDefines.h"
#else
#include "main.h"
#define  DEFAULT_INCLUDE

//**********************************************************************************
// General defines
//**********************************************************************************
#define GUI_UPDATE_MS      300     // GUI update interval in milliseconds
#define GUI_FAST_UPDATE_MS 10      // GUI fast process update interval in milliseconds
#define MONITOR_UPDATE_MS  200     // Monitor update interval in milliseconds
#define GENERAL_UPDATE_MS  100     // General system update interval in milliseconds

//**********************************************************************************
// DryWet Parameter
//**********************************************************************************
constexpr float MIN_DRY = -45.0f;  // Minimum dry signal level cDryWet
constexpr float MAX_DRY  = 0.0f;   // Maximum dry signal level cDryWet
constexpr float FAD_TIME = 5.0f;   // Dry/Wet Fading time in second

//**********************************************************************************
// Audio Manager
//**********************************************************************************
#define AUDIO_BUFFER_SIZE  4        // Audio buffer size in samples
#define SAMPLING_RATE      48000.0f // Audio sampling rate in Hz

// Real-time refresh rate derived from audio parameters, filters, etc.
constexpr float RT_RATE = SAMPLING_RATE / (float)AUDIO_BUFFER_SIZE;
constexpr float RT_TIME = (float)AUDIO_BUFFER_SIZE / SAMPLING_RATE;

//**********************************************************************************
// Memory Section Definitions
//**********************************************************************************

// SDRAM memory section attribute
#define SDRAM_SECTION __attribute__((section(".SDRAM_Section")))

// QSPI Flash sections for different storage purposes
//#define QFLASH_FLASHER __attribute__((section(".QFLASH_FlasherStorage")))
//#define QFLASH_TABSAVE __attribute__((section(".QFLASH_TabSaveBlock")))
#define QFLASH_LOADER __attribute__((section(".QFLASH_LoaderInfo")))
#define QFLASH_SECTION __attribute__((section(".QFLASH_Section")))

// RAM section without cache for DMA or critical operations
#define NO_CACHE_RAM __attribute__((section(".RAM_NO_CACHE_Section")))

// Instruction TCM (Tightly Coupled Memory) for fast code execution
// (Provisional?) halt of ITCM memory mapping for critical sections due to unresolved malfunctions
//#define ITCM __attribute__((section(".moveITCM")))

#define RAM_D1 __attribute__((section(".RAM_D1_Section")))

#endif
