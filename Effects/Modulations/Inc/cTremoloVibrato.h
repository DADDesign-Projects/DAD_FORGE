//==================================================================================
//==================================================================================
// File: cTremoloVibrato.h
// Description: Header file for Tremolo/Vibrato audio effect class
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "MultiModeEffect.h"
#include "cDCO.h"
#include "cDelayLine.h"

namespace DadEffect {

//**********************************************************************************
// Class: cTremoloVibrato
// Description: Implements tremolo and vibrato audio effects using LFO modulation
//**********************************************************************************

constexpr uint32_t TREMOLO_ID BUILD_ID('T', 'R', 'V', 'B');  // Unique effect identifier

class cTremoloVibrato : public cMultiModeEffectBase {
public:
    // -----------------------------------------------------------------------------
    // Method: onInitialize
    // Description: Initializes effect parameters and configuration
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // Method: onActivate
    // Description: Called when effect becomes active
    // -----------------------------------------------------------------------------
    void onActivate() override;

    // -----------------------------------------------------------------------------
    // Method: onDesactivate
    // Description: Called when effect becomes inactive
    // -----------------------------------------------------------------------------
    void onDesactivate() override;

    // -----------------------------------------------------------------------------
    // Method: Process
    // Description: Audio processing method - applies effect to input buffer
    // -----------------------------------------------------------------------------
    void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) override;

protected:
    // =============================================================================
    // UI CALLBACKS SECTION
    // =============================================================================

	// --------------------------------------------------------------------------
	// Method: SpeedChange (Callback)
	// Description: Triggered when LFO frequency parameter is changed by user
	// --------------------------------------------------------------------------
	static void SpeedChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

	// --------------------------------------------------------------------------
	// Method: RatioChange (Callback)
	// Description: Triggered when LFO duty cycle ratio parameter is changed by user
	// --------------------------------------------------------------------------
	static void RatioChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

	// --------------------------------------------------------------------------
	// Method: MixChange (Callback)
	// Description: Triggered when dry/wet mix parameter is changed by user
	// --------------------------------------------------------------------------
	static void MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

    // =============================================================================
    // USER INTERFACE COMPONENTS SECTION
    // =============================================================================

    // -----------------------------------------------------------------------------
    // DSP-LINKED PARAMETERS (User Controlled)
    // -----------------------------------------------------------------------------

	DadGUI::cUIParameter m_TremoloDeep;    // Tremolo depth (% volume modulation)
	DadGUI::cUIParameter m_VibratoDeep;    // Vibrato depth (% pitch modulation)
	DadGUI::cUIParameter m_DryWetMix;      // Dry/wet mix ratio
	DadGUI::cUIParameter m_LFOShape;       // LFO waveform shape (0: sine, 1: square)
	DadGUI::cUIParameter m_Freq;           // LFO frequency in Hz
	DadGUI::cUIParameter m_LFORatio;       // LFO duty cycle percentage
	DadGUI::cUIParameter m_StereoMode;     // Stereo processing mode

    // -----------------------------------------------------------------------------
    // PARAMETER VIEWS (UI Widgets to Display/Edit Parameters)
    // -----------------------------------------------------------------------------

	DadGUI::cParameterNumNormalView      m_FreqView;           // Frequency parameter view
	DadGUI::cParameterNumNormalView      m_TremoloDeepView;    // Tremolo depth parameter view
	DadGUI::cParameterNumNormalView      m_VibratoDeepView;    // Vibrato depth parameter view
	DadGUI::cParameterNumNormalView		 m_DryWetMixView;      // Dry/wet mix parameter view
	DadGUI::cParameterDiscretView        m_LFOShapeView;       // LFO shape parameter view
	DadGUI::cParameterNumLeftRightView   m_LFORatioView;       // LFO ratio parameter view
	DadGUI::cParameterDiscretView		 m_StereoModeView;     // Stereo mode parameter view

    // -----------------------------------------------------------------------------
    // PANEL COMPONENTS
    // -----------------------------------------------------------------------------

	DadGUI::cPanelOfParameterView m_ItemTremoloMenu;  // Tremolo parameters panel
	DadGUI::cPanelOfParameterView m_ItemLFOMenu;      // LFO parameters panel
	DadGUI::cPanelOfParameterView m_ItemStereoMode;   // Stereo mode parameters panel

    // =============================================================================
    // DSP COMPONENTS SECTION
    // =============================================================================

	DadDSP::cDCO m_LFOLeft;                       // Left channel Low-Frequency Oscillator
	DadDSP::cDCO m_LFORight;                      // Right channel Low-Frequency Oscillator

	DadDSP::cDelayLine m_ModulationLineRight;     // Right channel delay line for vibrato
	DadDSP::cDelayLine m_ModulationLineLeft;      // Left channel delay line for vibrato

	float m_CoefComp = 0.0f;                      // Compensation factor for vibrato depth consistency
};

} // namespace DadEffect

//***End of file**************************************************************
