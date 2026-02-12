//==================================================================================
//==================================================================================
// File: cEffectBase.h
// Description: Base effect class declaration for DSP audio processing
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cPanelOfSystemView.h"
#include "cParameterInfoView.h"

namespace DadEffect {

//**********************************************************************************
// Class: cEffectBase
// Base effect class for audio processing with GUI interface
//**********************************************************************************
class cEffectBase : public DadGUI::iGUI_EventListener {
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
    // Returns the unique effect identifier
    virtual uint32_t getEffectID() = 0;

    // -----------------------------------------------------------------------------
    // Main Audio processing function
    void Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence);

    // -----------------------------------------------------------------------------
    // Child audio processing function: processes one input/output audio buffer
    virtual void onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) = 0;

    // -----------------------------------------------------------------------------
    // Periodically updates switch state and detects user actions
    void on_GUI_FastUpdate() override;

    // -----------------------------------------------------------------------------
    // Callback event for memory restore start event
    static void StartRestoreEvent(void *pID, uint32_t Data);

    // -----------------------------------------------------------------------------
    // Callback event for memory restore end event
    static void EndRestoreEvent(void *pID, uint32_t Data);

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
    DadGUI::cParameterInfoView          m_cParameterInfoView;  // GUI temporary display of parameter information
    DadGUI::cSwitchOnOff                m_SwitchOnOff;         // On/off switch control
    DadGUI::cTapTempoMemChange          m_SwitchTempoMem;      // Tap tempo and memory change control

    // =============================================================================
    // Menu declaration
    // =============================================================================
    DadGUI::cUIMenu                     m_Menu;                // Main menu structure

    // =============================================================================
    // TapTempo
    // =============================================================================
    DadDSP::cParameter*                 m_pTapTempoParameter;  // Parameter modified by the TapTempo
    DadGUI::eTempoType                 m_TempoType;           // Defines the output unit period for seconds or frequency or none

    // =============================================================================
    // Fade for change memory
    // =============================================================================
    float                              m_FadeIncrement = 0.0f; // Wet gain fade increment for memory switch
    float                              m_FadGain = 1.0f;       // Wet gain for memory switch fade
    bool                               m_ChangeEffect = false; // Effect change pending flag
    uint8_t                            m_TargetSlot = 0.0f;    // Target memory slot for switch

};

} // namespace DadEffect

//***End of file**************************************************************
