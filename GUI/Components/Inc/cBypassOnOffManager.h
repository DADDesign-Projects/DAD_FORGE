#pragma once
//==================================================================================
//==================================================================================
// File: cBypassOnOffManager.h
// Description: Header file for the bypass/on/off manager class.
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "HardwareAndCo.h"
#include "GUI_Event.h"
#include "cDryWet.h"
#include "GPIO.h"

extern DadDrivers::cDryWet __DryWet;

namespace DadGUI {

// -----------------------------------------------------------------------------
// Enum representing internal state transitions for handling complex behavior during state changes
enum class eInternalEffectState_t : uint8_t {
    bypass = 0,
    off,
    on
};

//**********************************************************************************
// cBypassOnOffManager
// Manages the state transitions between bypass, off, and on for audio effects.
//
// This class handles the logic required to transition between different effect states
// such as bypassing the effect, turning it off, or activating it. It also manages
// fade-in/fade-out processes when transitioning between states.
//**********************************************************************************
class cBypassOnOffManager : public DadGUI::iGUI_EventListener {
public:
    // -----------------------------------------------------------------------------
    // Default constructor
    cBypassOnOffManager() = default;

    // -----------------------------------------------------------------------------
    // Destructor
    virtual ~cBypassOnOffManager() = default;

    // -----------------------------------------------------------------------------
    // Initializes the manager with default values.
    // Sets initial states for target, internal, and effect states to 'bypass'.
    // Ensures that the bypass relay is correctly initialized at startup.
    void Initialize(volatile uint8_t* pEffectState);

    // -----------------------------------------------------------------------------
    // Handles GUI fast update events.
    // This function checks whether a fade process has completed and updates the
    // effect state accordingly based on the internal transition state.
    void on_GUI_FastUpdate() override;

    // -----------------------------------------------------------------------------
    // Returns the current effect state.
    // Provides access to the current operational mode of the effect.
    // Useful for status reporting and conditional logic in GUI components.
    eInternalEffectState_t getState() const {
        return m_InternalState;  // Return internal state instead of m_EffecState
    }

protected:
    // -----------------------------------------------------------------------------
    // Transitions from bypass to on state.
    // Activates the dry/wet mixer to fade in and updates internal state.
    inline void BypassToOn() {
        SetPIN(ByPass);
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

private:
    // =============================================================================
    // Member variables
    // alignas(4) is used to workaround a compiler bug at -O3 optimization level.
    alignas(4) eInternalEffectState_t    m_TargetState;      // Target state for transition management
    alignas(4) eInternalEffectState_t    m_InternalState;    // Internal state tracking for transitions
    alignas(4) volatile uint8_t*         m_pEffectState;     // Pointer to the current effect state
    alignas(4) eInternalEffectState_t    m_OldEffectState;   // Previous effect state for comparison
    alignas(4) bool                      m_FadeInProcess;    // Flag to track if a fade is in progress
};

} // namespace DadGUI

//***End of file***************************************************************
