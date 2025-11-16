//==================================================================================
//==================================================================================
// File: cInfoView.h
// Description: Declaration of the cInfoView class for displaying system information
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include <string>

namespace DadGUI {

//**********************************************************************************
// cInfoView class declaration
//**********************************************************************************

//==================================================================================
// Class: cInfoView
// Description: Provides a status area showing memory slot, dirty status, and
//              system state (ON/OFF/BYPASS). Updates automatically when related
//              parameters change.
// Inheritance: iUIComponent - base class providing activation/deactivation and
//              update lifecycle hooks for GUI components.
//==================================================================================
class cInfoView : public iUIComponent {
public:
    // -----------------------------------------------------------------------------
    // Constructor/Destructor section
    // -----------------------------------------------------------------------------

    cInfoView() = default;                    // Default constructor
    virtual ~cInfoView() = default;           // Default destructor

    // -----------------------------------------------------------------------------
    // Public methods section
    // -----------------------------------------------------------------------------

    // Initialize internal members and attach to the GUI layer system
    void Init();

    // Called when the component becomes active
    void Activate() override;

    // Called when the component becomes inactive
    void Deactivate() override;

    // Called periodically (each frame/tick) while active
    void Update() override;

    // Force to redraw UI component
    void Redraw() override;

    // Draw the complete info view (memory number, state, dirty flag)
    void ShowView(bool isDirty, uint8_t MemSlot, const std::string State);

protected:
    // -----------------------------------------------------------------------------
    // Protected members section
    // -----------------------------------------------------------------------------

    bool            m_isActive;               // Component activation state
    DadGFX::cLayer* m_pInfoLayer;             // Pointer to the dedicated display layer
    eOnOff          m_MemState;               // Cached ON/OFF/BYPASS state
    uint8_t         m_MemSlot;                // Cached memory slot index
    bool            m_MemDirty;               // Cached dirty flag
};

} // namespace DadGUI

//***End of file**************************************************************
