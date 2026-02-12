//==================================================================================
//==================================================================================
// File: cBypassOnOffManager.cpp
// Description: Implementation file for the bypass/on/off manager class.
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "cBypassOnOffManager.h"

namespace DadGUI {

//**********************************************************************************
// cBypassOnOffManager
// Manages the state transitions between bypass, off, and on for audio effects.
//
// This class handles the logic required to transition between different effect states
// such as bypassing the effect, turning it off, or activating it. It also manages
// fade-in/fade-out processes when transitioning between states.
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes the manager with default values.
// Sets initial states for target, internal, and effect states to 'bypass'.
// Ensures that the bypass relay is correctly initialized at startup.
void cBypassOnOffManager::Initialize(volatile uint8_t* pEffectState) {
    if(!pEffectState) Error_Handler();                      // Ensure valid pointer is provided
    m_pEffectState = pEffectState;                          // Store pointer to principal effect state variable
    m_TargetState = eInternalEffectState_t::bypass;         // Initialize to bypass state
    m_InternalState = eInternalEffectState_t::bypass;       // Internal state starts as bypass
    m_OldEffectState = eInternalEffectState_t::bypass;      // Initialize previous effect state to bypass
    m_FadeInProcess = false;                                // No fade in progress at startup
    __GUI_EventManager.Subscribe_FastUpdate(this);          // Subscribe to GUI fast update events
    ResetPIN(ByPass);                                       // Ensure the bypass relay is in correct initial state
    SetPIN(AUDIO_MUTE);                                     // Effect is not muted at startup
}

// -----------------------------------------------------------------------------
// Handles GUI fast update events.
// This function checks whether a fade process has completed and updates the
// effect state accordingly based on the internal transition state.
void cBypassOnOffManager::on_GUI_FastUpdate() {
    // Check if the global effect state has changed since last update
    // If so, capture the new value and update the target state
    // Then, perform the appropriate transition.
    __disable_irq();
    m_TargetState = (eInternalEffectState_t) *m_pEffectState;   // Update the target state based on global effect state
    __enable_irq();

    if(m_OldEffectState != m_TargetState) {
        m_OldEffectState = m_TargetState;                       // Store the previous effect state

        // Execute appropriate transition based on target state
        switch (m_TargetState) {
            case eInternalEffectState_t::bypass:
                if (m_InternalState == eInternalEffectState_t::on) OnToBypass();
                else if (m_InternalState == eInternalEffectState_t::off) OffToBypass();
                break;
            case eInternalEffectState_t::off:
                if (m_InternalState == eInternalEffectState_t::on) OnToOff();
                break;
            case eInternalEffectState_t::on:
                if (m_InternalState == eInternalEffectState_t::bypass) BypassToOn();
                else if (m_InternalState == eInternalEffectState_t::off) OffToOn();
                break;
        }
    }

    // Check if a fade is in progress and if it's complete
    if (m_FadeInProcess && (__DryWet.getState() == (uint8_t) m_TargetState)) {
        m_FadeInProcess = false;                               // Reset fade flag when complete
        m_InternalState = m_TargetState;                       // Update internal state to match target
        if(m_InternalState == eInternalEffectState_t::bypass) {
            ResetPIN(ByPass);
        }
    }
}

} // namespace DadGUI

//***End of file***************************************************************
