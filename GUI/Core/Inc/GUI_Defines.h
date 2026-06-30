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

// =============================================================================
// Color Palette Definition
// =============================================================================

// Structure defining a complete color palette for the GUI
struct sColorPalette {
    // Menu colors
    DadGFX::sColor MenuBack;        // Menu background color
    DadGFX::sColor MenuText;        // Menu text color
    DadGFX::sColor MenuActiveText;  // Active menu text color
    DadGFX::sColor MenuActive;      // Active menu item color
    DadGFX::sColor MenuArrow;       // Menu arrow color

    // Parameter area colors
    DadGFX::sColor ParameterBack;   // Parameter background color
    DadGFX::sColor ParameterName;   // Parameter name color
    DadGFX::sColor ParameterNum;    // Parameter number color
    DadGFX::sColor ParameterValue;  // Parameter value color
    DadGFX::sColor ParameterLines;  // Parameter lines color
    DadGFX::sColor ParameterCursor; // Parameter cursor color

    // Parameter info colors
    DadGFX::sColor ParamInfoBack;   // Parameter info background
    DadGFX::sColor ParamInfoName;   // Parameter info name color
    DadGFX::sColor ParamInfoValue;  // Parameter info value color

    // Main info colors
    DadGFX::sColor MainInfoBack;    // Main info background
    DadGFX::sColor MainInfoMem;     // Memory indicator color
    DadGFX::sColor MainInfoDirty;   // Dirty state indicator color
    DadGFX::sColor MainInfoState;   // System state color

    // Memory View colors
    DadGFX::sColor MemViewBack;     // Memory view background
    DadGFX::sColor MemViewText;     // Memory view text color
    DadGFX::sColor MemViewLine;     // Memory view line color
    DadGFX::sColor MemViewActive;   // Active memory item color

    // VuMeter View colors
    DadGFX::sColor VuMeterBack;     // VU meter background
    DadGFX::sColor VuMeterText;     // VU meter text color
    DadGFX::sColor VuMeterLine;     // VU meter line color
    DadGFX::sColor VuMeterCursor;   // VU meter cursor color
    DadGFX::sColor VuMeterPeak;     // VU meter peak indicator color
    DadGFX::sColor VuMeterClip;     // VU meter clip indicator color

    // Splash screen colors
    DadGFX::sColor SplatchBack;     // Splash screen background
    DadGFX::sColor SplatchText;     // Splash screen text color
};

// =============================================================================
// Font Shortcuts
// =============================================================================

#define FONTXXS    __GUI.GetFontXXS()    // Extra extra small font
#define FONTXXSB   __GUI.GetFontXXSB()   // Extra extra small bold font
#define FONTXS     __GUI.GetFontXS()     // Extra small font
#define FONTXSB    __GUI.GetFontXSB()    // Extra small bold font
#define FONTS      __GUI.GetFontS()      // Small font
#define FONTSB     __GUI.GetFontSB()     // Small bold font
#define FONTM      __GUI.GetFontM()      // Medium font
#define FONTMB     __GUI.GetFontMB()     // Medium bold font
#define FONTL      __GUI.GetFontL()      // Large font
#define FONTLB     __GUI.GetFontLB()     // Large bold font
#define FONTXL     __GUI.GetFontXL()     // Extra large font
#define FONTXLB    __GUI.GetFontXLB()    // Extra large bold font
#define FONTXXL    __GUI.GetFontXXL()    // Extra extra large font
#define FONTXXLB   __GUI.GetFontXXLB()   // Extra extra large bold font
#define FONTXXXL   __GUI.GetFontXXXL()   // Extra extra extra large font
#define FONTXXXLB  __GUI.GetFontXXXLB()  // Extra extra extra large bold font

} // namespace DadGUI

//***End of file**************************************************************
