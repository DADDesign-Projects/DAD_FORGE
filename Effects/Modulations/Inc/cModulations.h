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

#define DECLARE_EFFECT DadEffect::cMainModulations __Effect
#define EFFECT_NAME "Modulations"
#define EFFECT_VERSION "Version 1.0"
#define EFFECT_SPLATCH_SCREEN "Modulations.png"
constexpr uint32_t EFFECT_BUILD = BUILD_ID('M', 'O', 'D', '1');

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

    cMultiModeEffectBase* m_TabEffects[NB_EFFECTS];                         // Array of effect instances
};

}// DadEffect
//***End of file**************************************************************
