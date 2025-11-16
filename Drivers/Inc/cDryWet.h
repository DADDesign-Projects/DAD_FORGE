//==================================================================================
//==================================================================================
// File: cDryWet.h
// Description: Dry/wet mix control class header with smooth crossfading
// The dry channel is managed by a hardware component PGA2310 PGA2311 controlled via an SPI bus.
// The wet channel is at the discretion of the user application through the getGainWet method.
// When the effect is off, the class sets the dry channel to MaxDry and the wet channel to 0.
// When the effect is on, both channels are mixed according to the value provided through setMix.
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "main.h"

namespace DadDrivers {

//**********************************************************************************
// cDryWet
//
// Helper class used to manage the dry/wet balance and smooth transitions
// during delay mix operations. Includes fading and incremental mix control.
//**********************************************************************************

class cDryWet {
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
    //   MinWet/MaxWet - Minimum and maximum wet gain in dB
    //   Increment - Mix increment per frame for smooth fade
    // -----------------------------------------------------------------------------
    void Init(float MinDry, float MaxDry, float Increment);

    // -----------------------------------------------------------------------------
    // Function: Process
    // Description: Updates internal mix state depending on effect On/Off status
    // -----------------------------------------------------------------------------
    void Process(bool On);

    // -----------------------------------------------------------------------------
    // Function: setMix
    // Description: Sets the mix level target (0.0 = full dry, 1.0 = full wet)
    // -----------------------------------------------------------------------------
    void setMix(float Mix);

    // -----------------------------------------------------------------------------
    // Function: setMix
    // Description: Force the mix level no fading (0.0 = full dry, 1.0 = full wet)
    // -----------------------------------------------------------------------------
    void forceMix(float Mix);

    // -----------------------------------------------------------------------------
    // Function: getGainWet
    // Description: Returns the current wet gain value
    // -----------------------------------------------------------------------------
    inline float getGainWet() {
        return m_GainWet;
    }

protected:
    // =============================================================================
    // Protected Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Function: computeMix
    // Description: Computes and updates the dry/wet gains based on current mix value
    // -----------------------------------------------------------------------------
    void computeMix();

    // =============================================================================
    // Member Variables
    // =============================================================================

    // Dry/Wet gain range limits
    float       m_MinDry;       // Minimum dry gain
    float       m_MaxDry;       // Maximum dry gain

    // Mix interpolation parameters
    float       m_Increment;    // Increment used for fade steps
    float       m_Mix;          // Current mix value
    float       m_MemMix;       // Previous mix value
    float       m_TargetMix;    // Target mix value
    float       m_OldMix;       // Cached old mix for interpolation

    // Computed results
    float       m_GainWet;      // Computed wet gain
    bool        m_ProcessFade;  // Indicates if a mix fade is active

    // State management
    bool        m_MemOn;        // Previous On/Off state for smooth transition
};

} // namespace DadUtilities
//***End of file**************************************************************
