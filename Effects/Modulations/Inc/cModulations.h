//==================================================================================
//==================================================================================
// File: cModulations.h
//
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "MultiModeEffect.h"

namespace DadEffect {
//**********************************************************************************
// Class: cMainModulations
// Description: Main multi-mode modulations effect
//**********************************************************************************

constexpr uint8_t NB_EFFECTS = 5;                                       // Number of available effects

class cMainModulations : public cMainMultiModeEffect {
public:
    // -----------------------------------------------------------------------------
    // Initialize
    // Initializes all effect instances and sets up the multi-mode system
    // -----------------------------------------------------------------------------
    void Initialize();

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    cMultiModeEffect* m_TabEffects[NB_EFFECTS];                         // Array of effect instances
};

}// DadEffect
//***End of file**************************************************************
