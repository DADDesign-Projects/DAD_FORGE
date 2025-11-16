//==================================================================================
//==================================================================================
// File: cDryWet.cpp
// Description: Dry/wet mix control implementation with smooth crossfading
// The dry channel is managed by a hardware component PGA2310 PGA2311 controlled via an SPI bus.
// The wet channel is at the discretion of the user application through the getGainWet method.
// When the effect is off, the class sets the dry channel to MaxDry and the wet channel to 0.
// When the effect is on, both channels are mixed according to the value provided through setMix.

// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "cDryWet.h"
#include "cSoftSPI.h"
extern DadDrivers::cSoftSPI __SoftSPI;
namespace DadDrivers {

//**********************************************************************************
// cDryWet
//
// Handles dry/wet mix control with smooth fading between on/off states
//**********************************************************************************

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Function: Init
// Description: Initializes mix volume parameters and ranges
// -----------------------------------------------------------------------------
void cDryWet::Init(float MinDry, float MaxDry, float Increment) {
    // Clamp dry volume parameters to valid range
    if(MinDry > 31.5f) MinDry = 31.5f;
    if(MinDry < -95.5f) MinDry = -95.5f;
    if(m_MaxDry > 31.5f) MinDry = 31.5f;
    if(m_MaxDry < -95.5f) MinDry = -95.5f;

    // Convert dB values to hardware volume indices
	m_MinDry = 192 + (MinDry * 2);     // Minimum dry volume index
    m_MaxDry = 192 + (MaxDry * 2);     // Maximum dry volume index
    m_Increment = Increment;           // Fade step size

    // Initialize state variables
    m_MemOn = false;       // Initial bypass state
    m_Mix = m_MemMix = m_TargetMix = m_OldMix = 0.0f;  // Reset all mix levels
    m_ProcessFade = false;             // No fade active initially
    m_GainWet = 0.0f;                  // Initial wet gain
}

// -----------------------------------------------------------------------------
// Function: Process
// Description: Processes mix state transitions and computes current gains
// -----------------------------------------------------------------------------
void cDryWet::Process(bool On) {
    // Check for state change to trigger fade
    if (m_MemOn != On) {
        m_MemOn = On;            		// Update stored state

        // Set target mix based on new state
        if (On == true) {
            m_TargetMix = m_MemMix;    // Fade to stored mix level when turning on
        } else {
            m_TargetMix = 0.0f;        // Fade to silence when bypassing
        }
    }

	// Fade up towards target
	if (m_Mix < m_TargetMix) {
		m_Mix += m_Increment;
		if (m_Mix > m_TargetMix) {
			m_Mix = m_TargetMix;   // Clamp to avoid overshoot
		}
	}
	// Fade down towards target
	else if (m_Mix > m_TargetMix) {
		m_Mix -= m_Increment;
		if (m_Mix < m_TargetMix) {
			m_Mix = m_TargetMix;   // Clamp to avoid undershoot
		}
	}

    // Update gains if mix level changed
    if (m_Mix != m_OldMix) {
        m_OldMix = m_Mix;              // Store current mix for change detection
        computeMix();                  // Recalculate wet/dry gains
    }
}

// -----------------------------------------------------------------------------
// Function: setMix
// Description: Force the mix level no fading (0.0 = full dry, 1.0 = full wet)
// -----------------------------------------------------------------------------
void cDryWet::forceMix(float Mix){
	m_Mix = m_TargetMix = Mix /100;
	computeMix();
}

// -----------------------------------------------------------------------------
// Function: setMix
// Description: Sets the target mix level (0-100%)
// -----------------------------------------------------------------------------
void cDryWet::setMix(float Mix) {
    m_MemMix = Mix / 100;              // Convert percentage to 0-1 range

    // Apply immediately if currently active
    if (m_MemOn == true) {
    	m_TargetMix = m_MemMix;
    }
}

// =============================================================================
// Private Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Function: computeMix
// Description: Computes wet/dry gains based on current mix position
// -----------------------------------------------------------------------------
void cDryWet::computeMix() {
    // Calculate crossfade gains using trigonometric equal-power curves
    float wetGain = sinf(m_Mix * M_PI_2);  // Wet gain: sin(0 to π/2) = 0 to 1
    float dryGain = cosf(m_Mix * M_PI_2);  // Dry gain: cos(0 to π/2) = 1 to 0

    m_GainWet = wetGain;

    // Convert wet gain from linear to dB scale, then to linear amplitude
    float db = m_MinDry + (dryGain * (m_MaxDry - m_MinDry));
    // Calculate dry volume and send to hardware
    uint16_t dryVolumeIndex =  static_cast<uint16_t>(db + 0.5f); // Round to nearest integer
    __SoftSPI.Transmit(((dryVolumeIndex << 8) & 0x0000FF00) + dryVolumeIndex);
}

} // namespace DadUtilities
//***End of file**************************************************************
