//==================================================================================
//==================================================================================
// File: Delay.h
// Description: Declaration of stereo delay effect with modulation and tone control
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cPanelOfSystemView.h"
#include "cDCO.h"
#include "BiquadFilter.h"
#include "cDelayLine.h"

namespace DadEffect {

//**********************************************************************************
// cDelay
//
// Implements a complete stereo delay effect with user interface bindings.
// Features include dual delay lines with feedback, LFO modulation of delay time,
// tone shaping using high-pass and low-pass filters, and comprehensive UI control.
//**********************************************************************************

class cDelay {
public:
    // -----------------------------------------------------------------------------
    // Constructor
    // Description: Default constructor; does not initialize DSP or UI components
    // -----------------------------------------------------------------------------
    cDelay() = default;  // Default constructor

    // -----------------------------------------------------------------------------
    // Function: Initialize
    // Description: Initializes DSP components, UI parameters, and links interface elements
    // -----------------------------------------------------------------------------
    void Initialize();

    // -----------------------------------------------------------------------------
    // Function: Process
    // Description: Processes a stereo audio buffer through the delay effect
    // Parameters:
    //   pIn - Pointer to input audio buffer
    //   pOut - Pointer to output audio buffer
    //   OnOff - Effect state (bypassed or active)
    // -----------------------------------------------------------------------------
    ITCM void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff);

    // -----------------------------------------------------------------------------
    // Static callbacks (UI parameter change handlers)
    // -----------------------------------------------------------------------------
    static void SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);   // LFO speed change callback
    static void BassChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);    // Bass control callback
    static void TrebleChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);  // Treble control callback

protected:
    // -----------------------------------------------------------------------------
    // Function: getLogFrequency
    // Description: Maps a normalized value [0.0, 1.0] to a logarithmic frequency range
    // Used to provide natural scaling for tone controls
    // -----------------------------------------------------------------------------
    float getLogFrequency(float normValue, float freqMin, float freqMax) const;

    // =============================================================================
    // User Interface Components
    // =============================================================================

    // Main delay parameters
    DadGUI::cUIParameter                 m_Time;              // Main delay time parameter
    DadGUI::cUIParameter                 m_Repeat;            // Feedback amount for delay 1
    DadGUI::cUIParameter                 m_Mix;               // Dry/Wet mix control parameter

    // Secondary delay parameters
    DadGUI::cUIParameter                 m_SubDelay;          // Subdivision ratio for secondary delay
    DadGUI::cUIParameter                 m_RepeatDelay2;      // Feedback amount for delay 2
    DadGUI::cUIParameter                 m_BlendD1D2;         // Blend ratio between delay 1 and 2

    // Tone control parameters
    DadGUI::cUIParameter                 m_Bass;              // Bass tone control parameter
    DadGUI::cUIParameter                 m_Treble;            // Treble tone control parameter

    // Modulation parameters
    DadGUI::cUIParameter                 m_ModulationDeep;    // LFO modulation depth parameter
    DadGUI::cUIParameter                 m_ModulationSpeed;   // LFO modulation rate parameter

    // Parameter views for UI display
    DadGUI::cParameterNumNormalView      m_TimeView;          // Time parameter view
    DadGUI::cParameterNumNormalView      m_RepeatView;        // Repeat parameter view
    DadGUI::cParameterNumNormalView      m_MixView;           // Mix parameter view
    DadGUI::cParameterDiscretView        m_SubDelayView;      // Subdivision parameter view
    DadGUI::cParameterNumNormalView      m_RepeatDelay2View;  // Delay 2 repeat view
    DadGUI::cParameterNumNormalView      m_BlendD1D2View;     // Blend parameter view
    DadGUI::cParameterNumNormalView      m_BassView;          // Bass parameter view
    DadGUI::cParameterNumNormalView      m_TrebleView;        // Treble parameter view
    DadGUI::cParameterNumNormalView      m_ModulationDeepView; // Modulation depth view
    DadGUI::cParameterNumNormalView      m_ModulationSpeedView; // Modulation speed view

    // UI panels and interface elements
    DadGUI::cPanelOfParameterView        m_ItemDelay1Menu;    // Delay 1 parameter menu
    DadGUI::cPanelOfParameterView        m_ItemDelay2Menu;    // Delay 2 parameter menu
    DadGUI::cPanelOfParameterView        m_ItemToneMenu;      // Tone EQ menu
    DadGUI::cPanelOfParameterView        m_ItemLFOMenu;       // LFO modulation menu
    DadGUI::cUIMemory                    m_ItemMenuMemory;    // UI persistent memory
    DadGUI::cUIVuMeter                   m_UIVuMeter;         // Visual VU Meter
    DadGUI::cPanelOfSystemView           m_PanelOfSystemView; // System-level view container
    DadGUI::cInfoView                    m_InfoView;          // Information/Help display
    DadGUI::cSwitchOnOff                 m_SwitchOnOff;       // On/Off switch
    DadGUI::cTapTempoMemChange           m_SwitchTempoMem;    // Tap tempo / memory switch
    DadGUI::cUIMenu                      m_Menu;              // Main user interface menu

    // =============================================================================
    // DSP Components
    // =============================================================================

    // Modulation and filtering
    DadDSP::cDCO                         m_LFO;               // LFO generator for delay modulation
    DadDSP::cBiQuad                      m_BassFilter1;       // High-pass filter for left delay
    DadDSP::cBiQuad                      m_TrebleFilter1;     // Low-pass filter for left delay
    DadDSP::cBiQuad                      m_BassFilter2;       // High-pass filter for right delay
    DadDSP::cBiQuad                      m_TrebleFilter2;     // Low-pass filter for right delay

    // Stereo delay lines
    DadDSP::cDelayLine                   m_Delay1LineRight;   // Delay line 1 - Right channel
    DadDSP::cDelayLine                   m_Delay1LineLeft;    // Delay line 1 - Left channel
    DadDSP::cDelayLine                   m_Delay2LineRight;   // Delay line 2 - Right channel
    DadDSP::cDelayLine                   m_Delay2LineLeft;    // Delay line 2 - Left channel

};

}  // namespace DadEffect

//***End of file**************************************************************
