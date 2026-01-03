//==================================================================================
//==================================================================================
// File: cTemplateEffect.h
// Description: Template effect class declaration for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "cEffectBase.h"

namespace DadEffect {

//**********************************************************************************
// Class: cTemplateEffect
// Description: Template effect class for audio processing with GUI interface
//**********************************************************************************

class cTemplateEffect: public cEffectBase {
public:
    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor - initializes nothing by itself
    // -----------------------------------------------------------------------------
    cTemplateEffect() = default;

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
    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Parameter declarations
    // -----------------------------------------------------------------------------
    DadGUI::cUIParameter                m_ParameterGain;         // Gain control parameter

    // -----------------------------------------------------------------------------
    // Parameter view declarations
    // -----------------------------------------------------------------------------
    DadGUI::cParameterNumNormalView     m_ParameterGainView;     // GUI view for gain parameter

    // -----------------------------------------------------------------------------
    // Panel declarations
    // -----------------------------------------------------------------------------
    DadGUI::cPanelOfParameterView       m_ParametrerDemoPanel;   // Demo panel containing parameter views

};

} // namespace DadEffect

//***End of file**************************************************************
