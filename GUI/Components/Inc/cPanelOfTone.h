//==================================================================================
//==================================================================================
// File: cPanelOfTone.h
// Description: Managing tone output audio for bass, mid, treble
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cPanelOfParameters.h"
#include <stdint.h>
#include "BiquadFilter.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfTone
//
// Description: Panel for managing tone output audio (bass, mid, treble)
//
//**********************************************************************************
class cPanelOfTone : public cPanelOfParameterView, public iGUI_EventListener {
public:
    virtual ~cPanelOfTone() {};

    // -----------------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------------

    // Initializes the parameter views for color theme and MIDI channel
    void Initialize(uint32_t SerializeID);

    // Updates the panel state and processes parameter changes
    void Update() override;

    // Process audio
    void Process(AudioBuffer *pIn, AudioBuffer *pOut);

    // Callback for adjusting filters based on the GUI parameter.
    static void BassChange(DadDSP::cParameter* pParameter, uint32_t Data);
    static void MidChange(DadDSP::cParameter* pParameter, uint32_t Data);
    static void TrebleChange(DadDSP::cParameter* pParameter, uint32_t Data);

protected:
    // =============================================================================
    // Protected Member Variables
    // =============================================================================
    char dumm[40];
    // DSP components
    DadDSP::cBiQuad                 m_BassBiQuad;   // BiQuad filter for bass control
    DadDSP::cBiQuad                 m_TrebleBiQuad;	// BiQuad filter for treble control
    DadDSP::cBiQuad                 m_MidBiQuad;    // BiQuad filter for mid control

    // Parameters
    DadGUI::cUIParameter            m_Bass;         // Bass level parameter
    DadGUI::cUIParameter            m_Treble;       // Treble level parameter
    DadGUI::cUIParameter            m_Mid;          // Volume level parameter

    // Views
    DadGUI::cParameterNumLeftRightView  m_BassView;  // View for bass parameter display
    DadGUI::cParameterNumLeftRightView  m_TrebleView;// View for treble parameter display
    DadGUI::cParameterNumLeftRightView  m_MidView;   // View for volume parameter display

};

} // namespace DadGUI

//***End of file**************************************************************
