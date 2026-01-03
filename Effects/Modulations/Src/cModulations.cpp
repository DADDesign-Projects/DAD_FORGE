//==================================================================================
//==================================================================================
// File: cModulations.cpp
//
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "cModulations.h"
#include "cTremoloVibrato.h"
#include "cChorus.h"
#include "cFlanger.h"
#include "cUniVibe.h"
#include "cPhaser.h"

namespace DadEffect {
//**********************************************************************************
// Class: cMainModulation
// Description: Main multi-mode modulations effect
//**********************************************************************************

// Storage buffers for effect instances with proper alignment
alignas(cTremoloVibrato) uint8_t TremoloVibratoBuffer[sizeof(cTremoloVibrato)];
alignas(cChorus) uint8_t ChorusBuffer[sizeof(cChorus)];
alignas(cFlanger) uint8_t FlangerBuffer[sizeof(cFlanger)];
alignas(cUniVibe) uint8_t UniVibeBuffer[sizeof(cUniVibe)];
alignas(cPhaser) uint8_t PhaserBuffer[sizeof(cPhaser)];

// ---------------------------------------------------------------------------------
// Initialize
// Initializes all effect instances and sets up the multi-mode system
// ---------------------------------------------------------------------------------
void cMainModulations::Initialize(){
	// Create effect instances using placement new in pre-allocated buffers
	m_TabEffects[0] = new (PhaserBuffer) cPhaser();
	m_TabEffects[1] = new (TremoloVibratoBuffer) cTremoloVibrato();
	m_TabEffects[2] = new (ChorusBuffer) cChorus();
	m_TabEffects[3] = new (FlangerBuffer) cFlanger();
	m_TabEffects[4] = new (UniVibeBuffer) cUniVibe();

	// Configure multi-mode effect system
	m_NbEffects = NB_EFFECTS;                              // Set number of available effects
	m_pActiveEffect = m_TabEffects[0];                     // Set default active effect
	m_pTabEffects = &m_TabEffects[0];                      // Set effects array pointer

	// Initialize base multi-mode effect functionality
	cMainMultiModeEffect::Initialize();
}


}// DadEffect
//***End of file**************************************************************
