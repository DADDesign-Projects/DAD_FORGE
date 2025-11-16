//==================================================================================
//==================================================================================
// File: cTemplateEffect.h
// Description: Template effect class declaration for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cPanelOfSystemView.h"

namespace DadEffect {

//**********************************************************************************
// Template effect class for audio processing with GUI interface
//**********************************************************************************
class cTemplateEffect
{
public:
    // -----------------------------------------------------------------------------
    // Constructor (initializes nothing by itself)
    cTemplateEffect() = default;

    // -----------------------------------------------------------------------------
    // Initializes DSP components and user interface parameters
    void Initialize();

    // -----------------------------------------------------------------------------
    // Audio processing function: processes one input/output audio buffer
    ITCM void Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff);

    // -----------------------------------------------------------------------------
    // Mix callback function - updates dry/wet mix based on parameter changes
    static void MixCallBack(DadDSP::cParameter* pParameter, uint32_t Context);

protected:
    // =============================================================================
    // Parameter declarations
    // =============================================================================

    DadGUI::cUIParameter                m_ParameterDemo1;  // Demo parameter 1
    DadGUI::cUIParameter                m_ParameterDemo2;  // Demo parameter 2
    DadGUI::cUIParameter                m_ParameterDemo3;  // Demo parameter 3 (discrete)
    DadGUI::cUIParameter                m_ParameterMix;    // Mix parameter with callback

    // =============================================================================
    // Parameter view declarations
    // =============================================================================

    DadGUI::cParameterNumNormalView     m_ParameterDemo1View;  // Normal numeric parameter view
    DadGUI::cParameterNumLeftRightView  m_ParameterDemo2View;  // Left/right numeric parameter view
    DadGUI::cParameterDiscretView       m_ParameterDemo3View;  // Discrete parameter view
    DadGUI::cParameterNumLeftRightView  m_ParameterMixView;    // Mix parameter view

    // =============================================================================
    // Panel declarations
    // =============================================================================

    DadGUI::cPanelOfParameterView       m_ParametrerDemoPanel; // Demo parameters panel
    DadGUI::cPanelOfParameterView       m_ParametrerMixPanel;  // Mix parameters panel
    DadGUI::cUIMemory                   m_MemoryPanel;         // Memory management panel
    DadGUI::cUIVuMeter                  m_VuMeterPanel;        // VU meter display panel
    DadGUI::cPanelOfSystemView          m_PanelOfSystemView;   // System view panel

    // =============================================================================
    // UI component declarations
    // =============================================================================

    DadGUI::cInfoView                   m_InfoView;            // Information display view
    DadGUI::cSwitchOnOff                m_SwitchOnOff;         // On/off switch control
    DadGUI::cTapTempoMemChange          m_SwitchTempoMem;      // Tap tempo and memory change control

    // =============================================================================
    // Menu declaration
    // =============================================================================

    DadGUI::cUIMenu                     m_Menu;                // Main menu structure
};

} // namespace DadEffect

//***End of file**************************************************************
