//==================================================================================
//==================================================================================
// File: cBypassOnOffManager.cpp
// Description: Implementation file for the bypass/on/off manager class.
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "cBypassOnOffManager.h"
#include "GUI_Event.h"
#include "cMidi.h"
#include "GUI_Defines.h"

// *****************************************************************************
// Global variables declarations
// *****************************************************************************
extern DadDrivers::cMidi __Midi;

namespace DadGUI {

extern DadGUI::GUI_EventManager __GUI_EventManager;

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
void cBypassOnOffManager::Initialize() {
    m_TargetState = eEffectState_t::bypass;         		// Initialize to bypass state
    m_InternalState = eEffectState_t::bypass;       		// Internal state starts as bypass
    m_OldEffectState = eEffectState_t::bypass;      		// Initialize previous effect state to bypass
    m_FadeInProcess = false;                                // No fade in progress at startup
    __GUI_EventManager.Subscribe_FastUpdate(this);  // Subscribe to GUI fast update events
    ResetPIN(ByPass);                                       // Ensure the bypass relay is in correct initial state
    SetPIN(AUDIO_MUTE);                                     // Effect is not muted at startup

    // Midi Callback
    __Midi.addControlChangeCallback(MIDI_CC_ON, (uint32_t) this, &MIDI_On_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_OFF, (uint32_t) this, &MIDI_Off_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_BYPASS, (uint32_t) this, &MIDI_ByPass_CallBack);
}

// -----------------------------------------------------------------------------
// Handles GUI fast update events.
// This function checks whether a fade process has completed and updates the
// effect state accordingly based on the internal transition state.
void cBypassOnOffManager::on_GUI_FastUpdate() {

    if(m_OldEffectState != m_TargetState) {
        m_OldEffectState = m_TargetState;                       // Store the previous effect state

        // Execute appropriate transition based on target state
        switch (m_TargetState) {
            case eEffectState_t::bypass:
                if (m_InternalState == eEffectState_t::on) OnToBypass();
                else if (m_InternalState == eEffectState_t::off) OffToBypass();
                break;
            case eEffectState_t::off:
                if (m_InternalState == eEffectState_t::on) OnToOff();
                break;
            case eEffectState_t::on:
                if (m_InternalState == eEffectState_t::bypass) BypassToOn();
                else if (m_InternalState == eEffectState_t::off) OffToOn();
                break;
        }
    }

    // Check if a fade is in progress and if it's complete
    if (m_FadeInProcess && (__DryWet.getState() == (uint8_t) m_TargetState)) {
        m_FadeInProcess = false;                               // Reset fade flag when complete
        m_InternalState = m_TargetState;                       // Update internal state to match target
        if(m_InternalState == eEffectState_t::bypass) {
            ResetPIN(ByPass);
        }
    }
}

//----------------------------------------------------------------------------
// MIDI_On_CallBack
//
// Description: MIDI callback for system ON command.
//----------------------------------------------------------------------------
void cBypassOnOffManager::MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypassOnOffManager* pThis = (cBypassOnOffManager* ) userData;
	pThis->setState(eEffectState_t::on);
}

//----------------------------------------------------------------------------
// MIDI_Off_CallBack
//
// Description: MIDI callback for system OFF command.
//----------------------------------------------------------------------------
void cBypassOnOffManager::MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypassOnOffManager* pThis = (cBypassOnOffManager* ) userData;
	pThis->setState(eEffectState_t::off);
}

//----------------------------------------------------------------------------
// MIDI_ByPass_CallBack
//
// Description: MIDI callback for system BYPASS command.
//----------------------------------------------------------------------------
void cBypassOnOffManager::MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypassOnOffManager* pThis = (cBypassOnOffManager* ) userData;
	pThis->setState(eEffectState_t::bypass);
}

} // namespace DadGUI

//***End of file***************************************************************
