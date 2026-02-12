//==================================================================================
//==================================================================================
// File: cFlanger.h
// Description: Flanger effect class declaration
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "MultiModeEffect.h"
#include "cModulator.h"

namespace DadEffect {

//**********************************************************************************
// Class: cFlanger
// Description: Implements flanger audio effect
//**********************************************************************************

constexpr uint32_t FLANGER_ID BUILD_ID('F', 'L', 'A', 'N');

class cFlanger : public cMultiModeEffectBase {
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
    // -----------------------------------------------------------------------------
    // Method: MixChange (Callback)
    // Description: Updates dry/wet mix parameter when changed by user
    // -----------------------------------------------------------------------------
    static void MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);


    // =============================================================================
    // USER INTERFACE COMPONENTS SECTION
    // =============================================================================

    DadGUI::cUIParameter m_Deep;               		// Effect depth parameter
    DadGUI::cUIParameter m_Feedback;                // Feedback
    DadGUI::cUIParameter m_DryWetMix;               // Dry/wet mix parameter

    DadGUI::cParameterNumNormalView m_DeepView;     // Depth parameter view
    DadGUI::cParameterNumNormalView m_FeedBackView; // Mode parameter view
    DadGUI::cParameterNumNormalView m_DryWetMixView;// Dry/wet mix parameter view

    DadGUI::cPanelOfParameterView m_PanelFlanger;   // Main Flanger parameters panel

    // =============================================================================
    // DSP COMPONENTS SECTION
    // =============================================================================

    DadDSP::cModulator m_ModulatorLeft;            // Modulator left channel
    DadDSP::cModulator m_ModulatorRight;           // Modulator right channel
};

} // namespace DadEffect

//***End of file**************************************************************
