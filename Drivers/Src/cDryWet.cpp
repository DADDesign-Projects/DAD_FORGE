//==================================================================================
//==================================================================================
// File: cDryWet.cpp
// Description: Dry/wet mix control implementation with smooth crossfading
// The dry channel is managed by a hardware component PGA2310 PGA2311 controlled via an SPI bus.
// The wet channel is at the discretion of the user application through the getGainWet method.
// When the effect is off, the class sets the dry channel to MaxDry and the wet channel to 0.
// When the effect is on, both channels are mixed according to the value provided through setMix.
// When the effect is bypassed, the wet channel is set to 0 and the dry channel to 0dB gain.
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "cDryWet.h"
#include "cSoftSPI.h"
#include "MainGUI.h"
#include "HardwareAndCo.h"
#include "DadUtilities.h"

extern DadDrivers::cSoftSPI __SoftSPI;

namespace DadDrivers {

//**********************************************************************************
// cDryWet
//
// Handles dry/wet mix control with smooth fading between on/off states
// IMPORTANT: Fading is done on the MIX parameter, not on individual gains
// This ensures proper volume progression during transitions
//**********************************************************************************

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Function: Init
// Description: Initializes mix volume parameters and ranges
// -----------------------------------------------------------------------------
void cDryWet::Init(float MinDry, float MaxDry, float TimeChange) {
    // Clamp dry volume parameters to valid range of -95.5dB to 31.5dB (PGA2310/11 range)
    DadClamp(MinDry, -95.5f, 31.5f);
    DadClamp(MaxDry, -95.5f, 31.5f);

    // Convert dB values to hardware volume indices
    // 0dB = 192 in hardware index (PGA2310/11)
    // 0.5dB = 1 step resolution (PGA2310/11)
    m_MinDry = 192.0f + (MinDry * 2.0f);
    m_MaxDry = 192.0f + (MaxDry * 2.0f);

    // Calculate mix increment for smooth transitions
    // RT_TIME is the real-time processing period
    m_MixIncrement = RT_TIME / (TimeChange);

    // Initialize mix parameters
    m_CurrentMix = 0.0f;        // Start at full dry
    m_PreviousMix = 0.1f;       // Set to 0.1 to force initial hardware transmission
    m_TargetMix = 0.0f;         // Target is also full dry
    m_UserMix = 0.0f;           // User hasn't set a mix yet

    // Initialize gains
    m_OldComputedDryGain = 0.0f;
    m_CurrentWetGain = 0.0f;    // Silent at start (bypass)
    m_CurrentDryGain = 0.0f;    // Silent at start (bypass)

    // State management - start in bypass
    m_State = m_CurrentState = eDryWetState_t::bypass;

    // Set initial hardware gain
    m_OldDryVolumeIndex = 0u;

    // Subscribe to GUI events
    __GUI_EventManager.Subscribe_FastUpdate(this);
    __GUI_EventManager.Subscribe_RT_Process(this);
}

// -----------------------------------------------------------------------------
// Function: on_GUI_FastUpdate
// Description: Processes mix state transitions and sets target mix values
// -----------------------------------------------------------------------------
void cDryWet::on_GUI_FastUpdate() {
    // Determine target mix based on current state
    if (m_State == eDryWetState_t::on) {
        // Effect is ON: use user-defined mix and fade in from bypass if needed
        m_TargetMix = m_UserMix;
    } else if (m_State == eDryWetState_t::off) {
        // Effect is OFF: full dry (mix = 0) and fade in from bypass if needed
        m_TargetMix = 0.0f;
    } else { // bypass
        // Effect is BYPASSED: fade out to silence
        m_TargetMix = 0.0f;
    }

    // Update current state when both fades are complete
    if (m_CurrentMix == m_TargetMix) {
        m_CurrentState = m_State;
    }
}

// -----------------------------------------------------------------------------
// Function: on_GUI_RT_Process
// Description: Performs smooth crossfading by fading the MIX parameter and BYPASS fade
// -----------------------------------------------------------------------------
void cDryWet::on_GUI_RT_Process() {
    // Update mix fade (smooth transition between on/off states)
    if (m_CurrentMix > m_TargetMix) {
        m_CurrentMix -= m_MixIncrement;
        if (m_CurrentMix < m_TargetMix) {
            m_CurrentMix = m_TargetMix;
        }
    } else if (m_CurrentMix < m_TargetMix) {
        m_CurrentMix += m_MixIncrement;
        if (m_CurrentMix > m_TargetMix) {
            m_CurrentMix = m_TargetMix;
        }
    }

    // Update hardware and wet gain only when mix changes to avoid unnecessary processing
    if (m_CurrentMix != m_PreviousMix) {
        m_PreviousMix = m_CurrentMix;

        // Calculate dry and wet gains from current mix value
        // Uses equal-power crossfade (sine/cosine) for constant perceived volume
        // This maintains constant perceived loudness during transitions
        m_CurrentWetGain = sinf(m_CurrentMix * M_PI_2);  // Wet: sin(0 to π/2) = 0 to 1
        m_CurrentDryGain = cosf(m_CurrentMix * M_PI_2); // Dry: cos(0 to π/2) = 1 to 0

        // Update hardware if dry gain changed
        if (m_CurrentDryGain != m_OldComputedDryGain) {
            m_OldComputedDryGain = m_CurrentDryGain;

            // Compute and transmit gain value to hardware (SPI) for dry signal
            // Map linear gain (0.0 to 1.0) to hardware volume index range
            float volumeIndex = m_MinDry + (m_CurrentDryGain * (m_MaxDry - m_MinDry));

            // Round to nearest integer and convert to uint16_t
            uint16_t dryVolumeIndex = static_cast<uint16_t>(volumeIndex + 0.5f);
            DadClampMAX(dryVolumeIndex, (uint16_t) 255);

            // Only update if changed (to avoid unnecessary SPI calls)
            if (dryVolumeIndex != m_OldDryVolumeIndex) {
                m_OldDryVolumeIndex = dryVolumeIndex;

                // Send to hardware via SPI (both channels get same value)
                __SoftSPI.Transmit(((dryVolumeIndex << 8) & 0x0000FF00) + dryVolumeIndex);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: setNormalizedMix
// Description: Sets the target mix level (0 to 1, i.e., 0 = 0%, 1 = 100%)
// -----------------------------------------------------------------------------
void cDryWet::setNormalizedMix(float Mix) {
    // Clamp mix to valid range
    DadClamp(Mix, 0.0f, 1.0f);

    // Store user mix setting
    m_UserMix = Mix;

    // If effect is currently ON, update target mix immediately
    if (m_State == eDryWetState_t::on) {
        m_TargetMix = m_UserMix;
    }
}

} // namespace DadDrivers

//***End of file**************************************************************
