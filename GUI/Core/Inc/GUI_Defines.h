//==================================================================================
//==================================================================================
// File: GUI_Define.h
// Description: GUI constants and structures
// 
// Copyright (c) 2025-2026 DadDesign-Projects.
//==================================================================================
//==================================================================================

#pragma once
#include "main.h"
#include "cDisplay.h"

namespace DadGUI {

#if __has_include("GUI_UserDefine.h")
#include "GUI_UserDefine.h"
#else
// Memory manager constants
constexpr uint8_t MAX_SLOT = 10;  // Maximum number of memory slots

// MIDI Control Change definitions
#define MIDI_CC_PRESET_UP       82  // Preset up command
#define MIDI_CC_PRESET_DOWN     83  // Preset down command
#define MIDI_CC_ON              85  // Turn on command
#define MIDI_CC_OFF             86  // Turn off command
#define MIDI_CC_BYPASS          87  // Bypass command
#define MIDI_CC_EFFECT_PARAM    12  // Effect parameter control

// Number of palettes
#define NB_PALETTE 8
#endif


//**********************************************************************************
// GUI Layout Overview
//
// Screen Layout:
//   _____________________________________________________________
//  |                           MenuArea                           |
//  |______________________________________________________________|
//  |   Parameter1Area  |    Parameter2Area   |   Parameter3Area   |
//  |                   |                     |                    |
//  |                   |                     |                    |
//  |                   |                     |                    |
//  |___________________|_____________________|____________________|
//  |                          InfoArea                            |
//  |                                                              |
//  |______________________________________________________________|
//
// Each zone corresponds to a dedicated component with its own visual and
// interactive logic managed by the DadGUI controller.
//**********************************************************************************

// =============================================================================
// Screen and Layout Constants
// =============================================================================

#define SCREEN_WIDTH        320     // Total screen width in pixels
#define SCREEN_HEIGHT       240     // Total screen height in pixels

#define MENU_HEIGHT         22      // Height of the menu area
#define MENU_EDGE           10      // Edge margin for menu items
#define NB_MENU_ITEM        4       // Number of menu items
#define MENU_ITEM_WIDTH     ((SCREEN_WIDTH - (2*MENU_EDGE)) / NB_MENU_ITEM)  // Width per menu item

#define MAIN_WIDTH          SCREEN_WIDTH                    // Main area width
#define PARAM_HEIGHT        128                             // Height of parameter area
#define NB_PARAM_ITEM       3                               // Number of parameter items
#define PARAM_WIDTH         (MAIN_WIDTH / NB_PARAM_ITEM)    // Width per parameter item
#define PARAM_NAME_HEIGHT   32                              // Height for parameter names
#define PARAM_FORM_HEIGHT   64                              // Height for parameter forms
#define PARAM_VAL_HEIGHT    32                              // Height for parameter values

#define INFO_HEIGHT         (SCREEN_HEIGHT - (MENU_HEIGHT + PARAM_HEIGHT))  // Calculated info area height


} // namespace DadGUI

//***End of file**************************************************************
