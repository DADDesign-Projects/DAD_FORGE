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
#include "cEffectBase.h"
#include "cPanelOfSystemView.h"
#include "cDCO.h"
#include "BiquadFilter.h"
#include "cDelayLine.h"

namespace DadEffect {
constexpr uint32_t DELAY_ID BUILD_ID('D', 'E', 'L', 'A');

//**********************************************************************************
// cDelay
//
// Implements a complete stereo delay effect with user interface bindings.
// Features include dual delay lines with feedback, LFO modulation of delay time,
// tone shaping using high-pass and low-pass filters, and comprehensive UI control.
//**********************************************************************************

class cDelay : public cEffectBase{
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
    void onInitialize();

    // -----------------------------------------------------------------------------
    // Returns the unique effect identifier
    // -----------------------------------------------------------------------------
    uint32_t getEffectID() override{
    	return DELAY_ID;
    }

    // -----------------------------------------------------------------------------
    // Function: onProcess
    // Description: Processes a stereo audio buffer through the delay effect
    // Parameters:
    //   pIn - Pointer to input audio buffer
    //   pOut - Pointer to output audio buffer
    //   OnOff - Effect state (bypassed or active)
    // -----------------------------------------------------------------------------
    void onProcess(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence);

    // -----------------------------------------------------------------------------
    // Static callbacks (UI parameter change handlers)
    // -----------------------------------------------------------------------------
    static void SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);   // LFO speed change callback
    static void BassChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);    // Bass control callback
    static void TrebleChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);  // Treble control callback
    static void SatChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);     // Saturation control callback

protected:

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
    DadGUI::cUIParameter                 m_Saturation;		  // Saturation control parameter

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
    DadGUI::cParameterNumNormalView		 m_BassView;          // Bass parameter view
    DadGUI::cParameterNumNormalView      m_TrebleView;        // Treble parameter view
    DadGUI::cParameterNumNormalView      m_SaturationView;    // Saturation parameter view
    DadGUI::cParameterNumNormalView      m_ModulationDeepView; // Modulation depth view
    DadGUI::cParameterNumNormalView      m_ModulationSpeedView; // Modulation speed view

    // UI panels and interface elements
    DadGUI::cPanelOfParameterView        m_PanelDelay1;    	  // Delay 1 Panel
    DadGUI::cPanelOfParameterView        m_PanelDelay2;       // Delay 2 Panel
    DadGUI::cPanelOfParameterView        m_PanelTone;         // Tone Panel
    DadGUI::cPanelOfParameterView        m_PanelLFO;          // LFO Panel

    // =============================================================================
    // DSP Components
    // =============================================================================

    // Modulation and filtering
    DadDSP::cDCO                         m_LFO;               // LFO generator for delay modulation

    // Stereo BiQuad filter
    DadDSP::cBiQuad                      m_BassFilter1;       // High-pass filter for delay 1
    DadDSP::cBiQuad                      m_TrebleFilter1;     // Low-pass filter for delay 1
    DadDSP::cBiQuad                      m_BassFilter2;       // High-pass filter for delay 2
    DadDSP::cBiQuad                      m_TrebleFilter2;     // Low-pass filter for delay 2

    // Stereo delay lines
    DadDSP::cDelayLine                   m_Delay1LineRight;   // Delay line 1 - Right channel
    DadDSP::cDelayLine                   m_Delay1LineLeft;    // Delay line 1 - Left channel
    DadDSP::cDelayLine                   m_Delay2LineRight;   // Delay line 2 - Right channel
    DadDSP::cDelayLine                   m_Delay2LineLeft;    // Delay line 2 - Left channel

    //
    float m_SatDrive;

    // Smoothed Time values
    float 								m_PrevTime;
    float 								m_PrevSubDelayR;
    float 								m_PrevSubDelayL;
};

}  // namespace DadEffect

//***End of file**************************************************************
