//==================================================================================
//==================================================================================
// File: cUniVibe.h
// Description: UniVibe effect class declaration
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "MultiModeEffect.h"
#include "cDCO.h"
#include "cAllPass.h"

namespace DadEffect {

//**********************************************************************************
// Class: cUniVibe
// Description: Implements UniVibe audio effect
//**********************************************************************************

constexpr uint32_t UNIVIBE_ID BUILD_ID('U', 'N', 'V', 'B');

class cUniVibe : public cMultiModeEffect {
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
    // Description: Audio processing method - applies UniVibe effect to input buffer
    // -----------------------------------------------------------------------------
    void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) override;

protected:
    // -----------------------------------------------------------------------------
    // Method: MixChange (Callback)
    // Description: Updates dry/wet mix parameter when changed by user
    // -----------------------------------------------------------------------------
    static void MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // Method: SpeedChange (Callback)
    // Description: Updates LFO speed when changed by user
    // -----------------------------------------------------------------------------
    static void SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // =============================================================================
    // USER INTERFACE COMPONENTS SECTION
    // =============================================================================

    DadGUI::cUIParameter m_Deep;                    // Effect depth parameter
    DadGUI::cUIParameter m_Speed;                   // LFO speed parameter
    DadGUI::cUIParameter m_DryWetMix;               // Dry/wet mix parameter

    DadGUI::cParameterNumNormalView m_DeepView;     // Depth parameter view
    DadGUI::cParameterNumNormalView m_SpeedView;    // Speed parameter view
    DadGUI::cParameterNumNormalView m_DryWetMixView; // Dry/wet mix parameter view

    DadGUI::cPanelOfParameterView m_PanelUniVibe;   // Main UniVibe parameters panel

    // =============================================================================
    // DSP COMPONENTS SECTION
    // =============================================================================

    DadDSP::cAllPass m_AllPass1;    // First all-pass filter
    DadDSP::cAllPass m_AllPass2;    // Second all-pass filter
    DadDSP::cAllPass m_AllPass3;    // Third all-pass filter
    DadDSP::cAllPass m_AllPass4;    // Fourth all-pass filter

    DadDSP::sAPFState m_APFStateL1; // State for left channel first all-pass filter
    DadDSP::sAPFState m_APFStateL2; // State for left channel second all-pass filter
    DadDSP::sAPFState m_APFStateL3; // State for left channel third all-pass filter
    DadDSP::sAPFState m_APFStateL4; // State for left channel fourth all-pass filter

    DadDSP::sAPFState m_APFStateR1; // State for right channel first all-pass filter
    DadDSP::sAPFState m_APFStateR2; // State for right channel second all-pass filter
    DadDSP::sAPFState m_APFStateR3; // State for right channel third all-pass filter
    DadDSP::sAPFState m_APFStateR4; // State for right channel fourth all-pass filter

    DadDSP::cDCO m_LFO;             // Low Frequency Oscillator for modulation

};

} // namespace DadEffect

//***End of file**************************************************************
