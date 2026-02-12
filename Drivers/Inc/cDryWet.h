//==================================================================================
//==================================================================================
// File: cDryWet.h
// Description: Dry/wet mix control class header with smooth crossfading
// The dry channel is managed by a hardware component PGA2310 PGA2311 controlled via an SPI bus.
// The wet channel is at the discretion of the user application through the getGainWet method.
// When the effect is off, the class sets the dry channel to MaxDry and the wet channel to 0.
// When the effect is on, both channels are mixed according to the value provided through setMix.
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#include "main.h"
#include "sections.h"
#include "GUI_Event.h"

// State definitions for Dry/Wet control
enum class eDryWetState_t : uint8_t {
    bypass = 0,
    off,
    on
};

namespace DadDrivers {

//**********************************************************************************
// cDryWet
//
// Helper class used to manage the dry/wet balance and smooth transitions
// during delay mix operations. Includes fading and incremental mix control.
//**********************************************************************************

class alignas(4) cDryWet : public DadGUI::iGUI_EventListener {
public:
    // =============================================================================
    // Constructor / Destructor
    // =============================================================================

    cDryWet() = default;
    ~cDryWet() = default;

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Function: Init
    // Description: Initializes mix parameters for dry/wet control
    // Parameters:
    //   MinDry/MaxDry - Minimum and maximum dry gain in dB
    //   TimeChange - Time in seconds for a full mix transition
    // -----------------------------------------------------------------------------
    void Init(float MinDry, float MaxDry, float TimeChange);

    // -----------------------------------------------------------------------------
    // Function: on_GUI_FastUpdate
    // Description: Updates internal mix state depending on effect On/Off status
    // -----------------------------------------------------------------------------
    void on_GUI_FastUpdate() override;

    // -----------------------------------------------------------------------------
    // Function: on_GUI_RT_Process
    // Description: Performs smooth crossfading between dry and wet signals
    // -----------------------------------------------------------------------------
    void on_GUI_RT_Process() override;

    // -----------------------------------------------------------------------------
    // Function: setMix
    // Description: Sets the mix level target (0.0 = full dry, 100.0 = full wet)
    // -----------------------------------------------------------------------------
    inline void setMix(float Mix) {
        setNormalizedMix(Mix * 0.01f); // Convert percentage to 0-1 range
    }

    // -----------------------------------------------------------------------------
    // Function: setNormalizedMix
    // Description: Sets the mix level target (0.0 = full dry, 1.0 = full wet)
    // -----------------------------------------------------------------------------
    void setNormalizedMix(float Mix);

    // -----------------------------------------------------------------------------
    // Function: FadeToOn
    // Description: Start fade to ON state (effect active with current mix)
    // -----------------------------------------------------------------------------
    inline void FadeToOn() {
        m_State = eDryWetState_t::on;
    }

    // -----------------------------------------------------------------------------
    // Function: FadeToOff
    // Description: Start fade to OFF state (effect bypassed, full dry)
    // -----------------------------------------------------------------------------
    inline void FadeToOff() {
        m_State = eDryWetState_t::off;
    }

    // -----------------------------------------------------------------------------
    // Function: FadeToBypass
    // Description: Start fade to BYPASS state (silent)
    // -----------------------------------------------------------------------------
    inline void FadeToBypass() {
        m_State = eDryWetState_t::bypass;
    }

    // -----------------------------------------------------------------------------
    // Function: getState
    // Description: Returns current state
    // -----------------------------------------------------------------------------
    inline uint8_t getState() {
        return (uint8_t) m_CurrentState;
    }

    // -----------------------------------------------------------------------------
    // Function: getGainWet
    // Description: Returns the current wet gain value
    // -----------------------------------------------------------------------------
    inline float getGainWet() {
        return m_CurrentWetGain;
    }

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    // Current gains (computed from current mix)
    float       m_OldComputedDryGain;   // Previous dry gain for change detection
    float       m_CurrentWetGain;       // Current wet gain (0.0 to 1.0)
    float       m_CurrentDryGain;       // Current dry gain (0.0 to 1.0)

    // Mix parameters (0.0 = full dry, 1.0 = full wet)
    float       m_CurrentMix;           // Current mix value (fades smoothly)
    float       m_TargetMix;            // Target mix value
    float       m_UserMix;             // User-defined mix setting (when effect is ON)
    float       m_PreviousMix;         // Previous mix for change detection in RT processing
    float       m_MixIncrement;        // Mix increment per sample for smooth transitions

    // Dry gain range limits (used for hardware gain control)
    float       m_MinDry;              // Minimum dry gain (hardware index)
    float       m_MaxDry;              // Maximum dry gain (hardware index)

    uint32_t    m_OldDryVolumeIndex;   // Previous hardware volume index (avoids unnecessary updates)

    // State management
    eDryWetState_t m_State;            // Target state
    eDryWetState_t m_CurrentState;     // Current state (when fade is complete)
};

} // namespace DadDrivers

//***End of file**************************************************************
