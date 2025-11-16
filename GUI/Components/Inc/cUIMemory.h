//==================================================================================
//==================================================================================
// File: cUIMemory.h
// Description: Memory management UI component definition
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "iUIComponent.h"
#include "cDisplay.h"

namespace DadGUI {

//**********************************************************************************
// Enumerations
//**********************************************************************************

// -----------------------------------------------------------------------------
// Enum: eMemState
// Description: Available memory operations
// -----------------------------------------------------------------------------
enum class eMemState {
    Save,    // Save current data into selected memory slot
    Load,    // Load data from selected memory slot
    Delete   // Delete data from selected memory slot
};

// -----------------------------------------------------------------------------
// Enum: eMemChoice
// Description: User confirmation choices
// -----------------------------------------------------------------------------
enum class eMemChoice {
    No = 0,        // User selected "No"
    Yes = 1,       // User selected "Yes"
    disabled       // Option temporarily disabled
};

//**********************************************************************************
// Class: cUIMemory
// Description: Handles memory management user interface and interactions
//**********************************************************************************
class cUIMemory : public iUIComponent {
public:
    virtual ~cUIMemory() {}

    // =============================================================================
    // Initialize memory UI system
    // =============================================================================
    void Init(uint32_t SerializeID);

    // =============================================================================
    // Activate memory UI component
    // =============================================================================
    void Activate() override;

    // =============================================================================
    // Deactivate memory UI component
    // =============================================================================
    void Deactivate() override;

    // =============================================================================
    // Main UI update routine
    // =============================================================================
    void Update() override;

    // =============================================================================
    // Force redraw of UI component
    // =============================================================================
    void Redraw() override;

    // -----------------------------------------------------------------------------
    // Set serialization ID for save/restore operations
    // -----------------------------------------------------------------------------
    inline void setSerializeID(uint32_t SerializeID) {
    	m_SerializeID = SerializeID;
    }

protected:
    // =============================================================================
    // Draw static visual elements
    // =============================================================================
    void drawStatLayer();

    // =============================================================================
    // Draw dynamic interface elements
    // =============================================================================
    void drawDynLayer();

protected:
    //**********************************************************************************
    // Member Variables
    //**********************************************************************************

    // -----------------------------------------------------------------------------
    // UI State Variables
    // -----------------------------------------------------------------------------
    bool                m_isActive;          // Indicates whether UI is active
    int16_t             m_MemorySlot;        // Currently selected memory slot
    eMemChoice          m_MemChoice;         // User confirmation choice
    eMemState           m_MemState;          // Current memory operation
    uint8_t             m_ActionExec;        // Action execution flag
    uint32_t            m_SerializeID;       // Serialization ID for operations

    // -----------------------------------------------------------------------------
    // Display Layer Pointers
    // -----------------------------------------------------------------------------
    DadGFX::cLayer*     m_pDynMemoryLayer;   // Dynamic display layer pointer
    DadGFX::cLayer*     m_pStatMemoryLayer;  // Static display layer pointer
};

} // namespace DadGUI

//***End of file**************************************************************
