//==================================================================================
//==================================================================================
// File: cChorus.h
// Description: Chorus effect class declaration
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "MultiModeEffect.h"
#include "cModulator.h"
#include "cAudioFader.h"

namespace DadEffect {

//**********************************************************************************
// Class: cChorus
// Description: Implements chorus audio effect using multiple modulated delay lines
//**********************************************************************************

constexpr uint32_t CHORUS_ID BUILD_ID('C', 'H', 'O', 'R');

class cChorus : public cMultiModeEffect {
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

    // -----------------------------------------------------------------------------
    // Method: ModeChange (Callback)
    // Description: Handles mode changes between single and triple chorus
    // -----------------------------------------------------------------------------
    static void ModeChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

    // =============================================================================
    // USER INTERFACE COMPONENTS SECTION
    // =============================================================================

    DadGUI::cUIParameter m_Deep;                    // Effect depth parameter
    DadGUI::cUIParameter m_Mode;                    // Chorus mode parameter
    DadGUI::cUIParameter m_DryWetMix;               // Dry/wet mix parameter

    DadGUI::cParameterNumNormalView m_DeepView;     // Depth parameter view
    DadGUI::cParameterDiscretView m_ModeView;       // Mode parameter view
    DadGUI::cParameterNumNormalView m_DryWetMixView; // Dry/wet mix parameter view

    DadGUI::cPanelOfParameterView m_PanelChorus;    // Main chorus parameters panel
    DadGUI::cPanelOfParameterView m_PanelOptions;   // Options parameters panel

    // =============================================================================
    // DSP COMPONENTS SECTION
    // =============================================================================

    DadDSP::cModulator m_Modulator1Left;            // First modulator left channel
    DadDSP::cModulator m_Modulator1Right;           // First modulator right channel

    DadDSP::cModulator m_Modulator2Left;            // Second modulator left channel
    DadDSP::cModulator m_Modulator2Right;           // Second modulator right channel

    DadDSP::cModulator m_Modulator3Left;            // Third modulator left channel
    DadDSP::cModulator m_Modulator3Right;           // Third modulator right channel

    DadDSP::cAudioFader m_Fader;                    // Audio crossfader for mode transitions
};

} // namespace DadEffect

//***End of file**************************************************************
