//==================================================================================
//==================================================================================
// File: cTestEffect.h
// Description: Template effect class declaration for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "cEffectBase.h"

namespace DadEffect {

//**********************************************************************************
// Class: cTestEffect
// Description: Template effect class for audio processing with GUI interface
//**********************************************************************************

class cTestEffect: public cEffectBase {
public:
    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor - initializes nothing by itself
    // -----------------------------------------------------------------------------
    cTestEffect() = default;

    // -----------------------------------------------------------------------------
    // Initializes DSP components and user interface parameters
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // Returns the unique effect identifier
    // -----------------------------------------------------------------------------
    uint32_t getEffectID() override;

    // -----------------------------------------------------------------------------
    // Audio processing function - processes one input/output audio buffer
    // -----------------------------------------------------------------------------
    ITCM void onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) override;

protected:
    // -----------------------------------------------------------------------------
    // Method: MixChange (Callback)
    // Description: Updates dry/wet mix parameter when changed by user
    // -----------------------------------------------------------------------------
    static void MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Parameter declarations
    // -----------------------------------------------------------------------------
    DadGUI::cUIParameter                m_ParameterMix;         // Gain control parameter

    // -----------------------------------------------------------------------------
    // Parameter view declarations
    // -----------------------------------------------------------------------------
    DadGUI::cParameterNumNormalView     m_ParameterMixView;     // GUI view for gain parameter

    // -----------------------------------------------------------------------------
    // Panel declarations
    // -----------------------------------------------------------------------------
    DadGUI::cPanelOfParameterView       m_ParametrerTestPanel;   // Demo panel containing parameter views

};

} // namespace DadEffect

//***End of file**************************************************************
