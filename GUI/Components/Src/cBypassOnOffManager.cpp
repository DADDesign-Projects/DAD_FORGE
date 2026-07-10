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

// -----------------------------------------------------------------------------
// Initializes the manager with default values.
void cOn_Off_Manager::Initialize(){
    m_TargetState = eEffectState_t::off;         		// Initialize to off state
    m_InternalState = eEffectState_t::off;       		// Internal state starts as off
    m_OldEffectState = eEffectState_t::off;      		// Initialize previous effect state to off
    m_FadeInProcess = false;                            // No fade in progress at startup
    __GUI_EventManager.Subscribe_FastUpdate(this);  	// Subscribe to GUI fast update events
    __DryWet.setNormalizedMix(1.0f);

    // Midi Callback
    __Midi.addControlChangeCallback(MIDI_CC_ON, (uint32_t) this, &MIDI_On_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_OFF, (uint32_t) this, &MIDI_Off_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_BYPASS, (uint32_t) this, &MIDI_ByPass_CallBack);
}

// -----------------------------------------------------------------------------
// Handles GUI fast update events.
// This function checks whether a fade process has completed and updates the
// effect state accordingly based on the internal transition state.
void cOn_Off_Manager::on_GUI_FastUpdate(){
    if(m_OldEffectState != m_TargetState) {
        m_OldEffectState = m_TargetState;                       // Store the previous effect state

        // Execute appropriate transition based on target state
        switch (m_TargetState) {
            case eEffectState_t::bypass:
                if (m_InternalState == eEffectState_t::on) OnToOff();
                break;
            case eEffectState_t::off:
                if (m_InternalState == eEffectState_t::on) OnToOff();
                break;
            case eEffectState_t::on:
                if (m_InternalState == eEffectState_t::off) OffToOn();
                break;
        }
    }

    // Check if a fade is in progress and if it's complete
    if (m_FadeInProcess && (__DryWet.getState() == (uint8_t) m_TargetState)) {
        m_FadeInProcess = false;                               // Reset fade flag when complete
        m_InternalState = m_TargetState;                       // Update internal state to match target
    }
}

// -----------------------------------------------------------------------------
// Set state of effect
void cOn_Off_Manager::setState(eEffectState_t State){
    switch (State) {
        case eEffectState_t::bypass:
        	m_TargetState = eEffectState_t::off;
            break;
        case eEffectState_t::off:
        	m_TargetState = eEffectState_t::off;
        	break;
        case eEffectState_t::on:
        	m_TargetState = eEffectState_t::on;
        	break;
    }
}


// -------------------------------------------------------------------------
// MIDI_On_CallBack
//
// Description: MIDI callback for system ON command.
// -------------------------------------------------------------------------
void cOn_Off_Manager::MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cOn_Off_Manager* pThis = (cOn_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::on);
}

// -------------------------------------------------------------------------
// MIDI_Off_CallBack
//
// Description: MIDI callback for system OFF command.
// -------------------------------------------------------------------------
void cOn_Off_Manager::MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cOn_Off_Manager* pThis = (cOn_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::off);
}

// -------------------------------------------------------------------------
// MIDI_ByPass_CallBack
//
// Description: MIDI callback for system BYPASS command.
// -------------------------------------------------------------------------
void cOn_Off_Manager::MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cOn_Off_Manager* pThis = (cOn_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::off);
}


//**********************************************************************************
// cBypass_On_Off_Manager
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
void cBypass_On_Off_Manager::Initialize() {
    m_TargetState = eEffectState_t::bypass;         		// Initialize to bypass state
    m_InternalState = eEffectState_t::bypass;       		// Internal state starts as bypass
    m_OldEffectState = eEffectState_t::bypass;      		// Initialize previous effect state to bypass
    m_FadeInProcess = false;                                // No fade in progress at startup
    __GUI_EventManager.Subscribe_FastUpdate(this);  // Subscribe to GUI fast update events
#ifdef ByPass_Pin
    ResetPIN(ByPass);                                       // Ensure the bypass relay is in correct initial state
#endif
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
void cBypass_On_Off_Manager::on_GUI_FastUpdate() {

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
#ifdef ByPass_Pin
            ResetPIN(ByPass);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Set state of effect
void cBypass_On_Off_Manager::setState(eEffectState_t State){
	m_TargetState = State;
}

//----------------------------------------------------------------------------
// MIDI_On_CallBack
//
// Description: MIDI callback for system ON command.
//----------------------------------------------------------------------------
void cBypass_On_Off_Manager::MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypass_On_Off_Manager* pThis = (cBypass_On_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::on);
}

//----------------------------------------------------------------------------
// MIDI_Off_CallBack
//
// Description: MIDI callback for system OFF command.
//----------------------------------------------------------------------------
void cBypass_On_Off_Manager::MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypass_On_Off_Manager* pThis = (cBypass_On_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::off);
}

//----------------------------------------------------------------------------
// MIDI_ByPass_CallBack
//
// Description: MIDI callback for system BYPASS command.
//----------------------------------------------------------------------------
void cBypass_On_Off_Manager::MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData){
	cBypass_On_Off_Manager* pThis = (cBypass_On_Off_Manager* ) userData;
	pThis->setState(eEffectState_t::bypass);
}

} // namespace DadGUI

//***End of file***************************************************************
