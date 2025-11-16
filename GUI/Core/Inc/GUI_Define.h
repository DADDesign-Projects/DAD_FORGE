//==================================================================================
//==================================================================================
// File: GUI_Define.h
// Description: GUI constants and MIDI control definitions
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

//**********************************************************************************
//**********************************************************************************
// Namespace: DadGUI
// Description: Contains GUI constants and MIDI control definitions
//**********************************************************************************
//**********************************************************************************

#include <stdint.h>

namespace DadGUI {

// Memory manager constants
constexpr uint8_t MAX_SLOT = 10;  // Maximum number of memory slots

// MIDI Control Change definitions
#define MIDI_CC_PRESET_UP       82  // Preset up command
#define MIDI_CC_PRESET_DOWN     83  // Preset down command
#define MIDI_CC_ON              85  // Turn on command
#define MIDI_CC_OFF             86  // Turn off command
#define MIDI_CC_BYPASS          87  // Bypass command
#define MIDI_CC_EFFECT_PARAM    12  // Effect parameter control

} // namespace DadGUI

//***End of file**************************************************************
