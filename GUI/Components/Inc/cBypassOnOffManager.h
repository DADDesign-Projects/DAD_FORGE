#pragma once
//==================================================================================
//==================================================================================
// File: cBypassOnOffManager.h
// Description: Header file for the bypass/on/off manager class.
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "GUI_Event.h"
#include "cDryWet.h"
#include "GPIO.h"

// *****************************************************************************
// Global variables declarations
// *****************************************************************************
extern DadDrivers::cDryWet __DryWet;

namespace DadGUI {

// -----------------------------------------------------------------------------
// Enum representing internal state
enum class eEffectState_t : uint8_t {
    bypass = 0,
    off,
    on
};

//**********************************************************************************
// iBypassOnOffManager
//**********************************************************************************
class iBypassOnOffManager {
public:
    // -----------------------------------------------------------------------------
    // Initializes the manager with default values.
    virtual void Initialize()=0;

    // -----------------------------------------------------------------------------
    // Set state of effect
    virtual void setState(eEffectState_t State) = 0;

    // -----------------------------------------------------------------------------
    // Returns the current effect state.
    eEffectState_t getTargetState() const {
        return m_TargetState;
    }

    // -----------------------------------------------------------------------------
    // Returns the current effect state.
    eEffectState_t getState() const {
        return m_InternalState;  // Return internal state
    }

protected :
    eEffectState_t    	m_TargetState;      // Target state for transition management
    eEffectState_t    	m_InternalState;    // Internal state tracking for transitions
};

//**********************************************************************************
// cOn_Off_Manager
// Manages the state transitions between off, and on for audio effects.
//**********************************************************************************
class cOn_Off_Manager : public iBypassOnOffManager, public DadGUI::iGUI_EventListener {
public:
    // -----------------------------------------------------------------------------
    // Default constructor
	cOn_Off_Manager() = default;

    // -----------------------------------------------------------------------------
    // Destructor
    virtual ~cOn_Off_Manager() = default;

    // -----------------------------------------------------------------------------
    // Initializes the manager with default values.
    void Initialize() override;

    // -----------------------------------------------------------------------------
    // Handles GUI fast update events.
    // This function checks whether a fade process has completed and updates the
    // effect state accordingly based on the internal transition state.
    void on_GUI_FastUpdate() override;

    // -----------------------------------------------------------------------------
    // Set state of effect
    void setState(eEffectState_t State) override ;

protected:
    // -----------------------------------------------------------------------------
    // Transitions from on to off state.
    // Activates the dry/wet mixer to fade out and updates internal state.
    inline void OnToOff() {
        __DryWet.FadeToOff();
        m_FadeInProcess = true;
    }

    // -----------------------------------------------------------------------------
    // Transitions from off to on state.
    // Activates the dry/wet mixer to fade in and updates internal state.
    inline void OffToOn() {
        __DryWet.FadeToOn();
        m_FadeInProcess = true;
    }

    // -------------------------------------------------------------------------
    // MIDI_On_CallBack
    //
    // Description: MIDI callback for system ON command.
    // -------------------------------------------------------------------------
    static void MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_Off_CallBack
    //
    // Description: MIDI callback for system OFF command.
    // -------------------------------------------------------------------------
    static void MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_ByPass_CallBack
    //
    // Description: MIDI callback for system BYPASS command.
    // -------------------------------------------------------------------------
    static void MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData);

private:
    // =============================================================================
    // Member variables
    eEffectState_t    	m_OldEffectState;   // Previous effect state for comparison
    bool                m_FadeInProcess;    // Flag to track if a fade is in progress
};

//**********************************************************************************
// cBypass_On_Off_Manager
// Manages the state transitions between bypass, off, and on for audio effects.
//
// This class handles the logic required to transition between different effect states
// such as bypassing the effect, turning it off, or activating it. It also manages
// fade-in/fade-out processes when transitioning between states.
//**********************************************************************************
class cBypass_On_Off_Manager : public iBypassOnOffManager, public DadGUI::iGUI_EventListener {
public:
    // -----------------------------------------------------------------------------
    // Default constructor
	cBypass_On_Off_Manager() = default;

    // -----------------------------------------------------------------------------
    // Destructor
    virtual ~cBypass_On_Off_Manager() = default;

    // -----------------------------------------------------------------------------
    // Initializes the manager with default values.
    void Initialize() override;

    // -----------------------------------------------------------------------------
    // Handles GUI fast update events.
    // This function checks whether a fade process has completed and updates the
    // effect state accordingly based on the internal transition state.
    void on_GUI_FastUpdate() override;

    // -----------------------------------------------------------------------------
    // Set state of effect
    void setState(eEffectState_t State) override ;

protected:
    // -----------------------------------------------------------------------------
    // Transitions from bypass to on state.
    // Activates the dry/wet mixer to fade in and updates internal state.
    inline void BypassToOn() {
#ifdef ByPass_Pin
    	SetPIN(ByPass);
#endif
        __DryWet.FadeToOn();
        m_FadeInProcess = true;
    }

    // -----------------------------------------------------------------------------
    // Transitions from on to bypass state.
    // Activates the dry/wet mixer to fade out and updates internal state.
    inline void OnToBypass() {
        __DryWet.FadeToBypass();
        m_FadeInProcess = true;
    }

    // -----------------------------------------------------------------------------
    // Transitions from off to bypass state.
    // Activates the dry/wet mixer to fade out and updates internal state.
    inline void OffToBypass() {
        __DryWet.FadeToBypass();
        m_FadeInProcess = true;
    }

    // -----------------------------------------------------------------------------
    // Transitions from on to off state.
    // Activates the dry/wet mixer to fade out and updates internal state.
    inline void OnToOff() {
        __DryWet.FadeToOff();
        m_FadeInProcess = true;
    }

    // -----------------------------------------------------------------------------
    // Transitions from off to on state.
    // Activates the dry/wet mixer to fade in and updates internal state.
    inline void OffToOn() {
        __DryWet.FadeToOn();
        m_FadeInProcess = true;
    }

    // -------------------------------------------------------------------------
    // MIDI_On_CallBack
    //
    // Description: MIDI callback for system ON command.
    // -------------------------------------------------------------------------
    static void MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_Off_CallBack
    //
    // Description: MIDI callback for system OFF command.
    // -------------------------------------------------------------------------
    static void MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_ByPass_CallBack
    //
    // Description: MIDI callback for system BYPASS command.
    // -------------------------------------------------------------------------
    static void MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData);

private:
    // =============================================================================
    // Member variables
    eEffectState_t    	m_OldEffectState;   // Previous effect state for comparison
    bool                m_FadeInProcess;    // Flag to track if a fade is in progress
};

} // namespace DadGUI

//***End of file***************************************************************
