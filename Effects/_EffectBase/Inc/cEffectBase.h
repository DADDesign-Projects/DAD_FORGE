//==================================================================================
//==================================================================================
// File: cEffectBase.h
// Description: Base effect class declaration for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "GUI_Include.h"
#include "cPanelOfSystemView.h"

namespace DadEffect {

//**********************************************************************************
// Base effect class for audio processing with GUI interface
//**********************************************************************************
class cEffectBase {
public:
    // -----------------------------------------------------------------------------
    // Constructor / Destructor
	cEffectBase() = default;
	virtual ~cEffectBase() = default;

    // -----------------------------------------------------------------------------
    // Initializes DSP components and user interface parameters
    void Initialize();
    virtual void onInitialize() = 0;

    // -----------------------------------------------------------------------------
    //
    virtual uint32_t getEffectID() = 0;

	// -----------------------------------------------------------------------------
    // Audio processing function: processes one input/output audio buffer
    ITCM virtual void Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence);

	// -----------------------------------------------------------------------------
    // Audio processing function: processes one input/output audio buffer
    ITCM virtual void onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) = 0;


protected:
    // =============================================================================
    // Panel declarations
    // =============================================================================

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
