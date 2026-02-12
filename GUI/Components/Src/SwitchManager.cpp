//==================================================================================
//==================================================================================
// File: SwitchManager.cpp
// Description: Implementation of footswitch management classes for GUI control,
//              including On/Off/Bypass toggling and tap-tempo/memory slot control.
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "SwitchManager.h"

namespace DadGUI {

//**********************************************************************************
// Class: cSwitchOnOff
// Description: Handles a footswitch controlling On/Off/Bypass states
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Section: Public Methods
// ---------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Function: Init
// Description: Initializes the footswitch reference and registers the component
//-----------------------------------------------------------------------------------
void cSwitchOnOff::Init(DadDrivers::cSwitch* pFootSwitch, uint32_t EffectID) {
    m_pFootSwitch = pFootSwitch;  // Store footswitch reference
    m_OldPressCount = 0;          // Initialize press count tracking
    m_LastPressTime = 0;          // Reset last press timestamp
    __GUI_EventManager.Subscribe_FastUpdate(this, EffectID); // Register with GUI update system
}
//-----------------------------------------------------------------------------------
// Function: Update
// Description:
//   - Detects single, and long presses on the footswitch
//   - Short press toggles On/Off states
//   - Long press (within .6 second) triggers Bypass mode
//-----------------------------------------------------------------------------------
void cSwitchOnOff::on_GUI_FastUpdate() {
    float 		PressDuration;                          			  // Duration of current press
    uint8_t	 	SwitchState = m_pFootSwitch->getState(PressDuration); // Current switch state
    uint32_t 	PressCount = m_pFootSwitch->getPressCount();          // Total press count

    // Check if a new press has occurred and is not a long press (long press is handled by the switch driver)
    // If a new press is detected and it's not a long press, toggle On/Off state
    // If a long press occurs, set bypass mode
    if (m_OldPressCount != PressCount) {
        if (SwitchState == 0) {
        	m_OldPressCount = PressCount;
        	if(PressDuration < 1.0f){
                if (__OnOffCmd == eOnOff::Off || __OnOffCmd == eOnOff::ByPass) {
                    __OnOffCmd = eOnOff::On;   // Turn on if currently off or in bypass
                } else if (__OnOffCmd == eOnOff::On) {
                    __OnOffCmd = eOnOff::Off;  // Turn off if currently on
                }
        	}
        }else if(PressDuration > 1.0f ){
        		__OnOffCmd = eOnOff::ByPass;
        }
    }
}

//**********************************************************************************
// Class: cTapTempoMemChange
// Description: Handles tempo tapping and memory slot changes with a single footswitch
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Section: Public Methods
// ---------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// Function: Init
// Description: Initializes switch, parameter, and effect linkage
//-----------------------------------------------------------------------------------
void cTapTempoMemChange::Init(DadDrivers::cSwitch* pFootSwitch,
                              DadDSP::cParameter* pParameter,
                              uint32_t EffectID,
                              eTempoType TempoType) {
    m_pFootSwitch = pFootSwitch;  // Store footswitch reference
    m_TempoType = TempoType;      // Set tempo calculation type
    m_pParameter = pParameter;    // Store parameter to control
    m_PeriodUpdateCount = 0;      // Initialize period update tracking
    __GUI_EventManager.Subscribe_FastUpdate(this, EffectID); // Register with GUI update system
}

//-----------------------------------------------------------------------------------
// Function: Update
// Description:
//   - Long press (≥250 ms): increment memory slot
//   - Tap tempo: update parameter based on tap frequency or period
//-----------------------------------------------------------------------------------
void cTapTempoMemChange::on_GUI_FastUpdate() {
    float PressDuration;                                  // Duration of current press
    uint32_t PeriodUpdateCount = m_pFootSwitch->getPeriodUpdateCount();  // Period update counter
    uint8_t SwitchState = m_pFootSwitch->getState(PressDuration);        // Current switch state
    uint32_t PressCount = m_pFootSwitch->getPressCount();                // Total press count

    // Check for new press events
    if (m_OldPressCount != PressCount) {
        if (SwitchState == 0) {
            // Button released - update tracking
            m_OldPressCount = PressCount;
        } else {
            // Long press detection for memory slot change
            if (PressDuration >= 0.500f) {
                __MemoryManager.IncrementSlot(1);      // Advance to next memory slot
                m_OldPressCount = PressCount;          // Update press count tracking
                m_PeriodUpdateCount = PeriodUpdateCount;  // Sync period update counter
            }
        }

        // Tap tempo processing - update parameter when period changes
        if ((m_TempoType != eTempoType::none) && (PeriodUpdateCount != 0) && (m_PeriodUpdateCount != PeriodUpdateCount)) {
            // Calculate and set parameter based on tempo type
            if (m_TempoType == eTempoType::frequency) {
                m_pParameter->setValue(1.0f / m_pFootSwitch->getPressPeriod());  // Convert period to frequency
            } else {
                m_pParameter->setValue(m_pFootSwitch->getPressPeriod());  // Use period directly
            }
            m_PeriodUpdateCount = PeriodUpdateCount;  // Update period tracking
        }
    }
}

} // namespace DadGUI
//***End of file**************************************************************
