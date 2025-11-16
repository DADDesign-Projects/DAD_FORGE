//==================================================================================
//==================================================================================
// File: SwitchManager.h
// Description: Header file defining footswitch management classes for effect
//              toggling (On/Off/Bypass) and tap-tempo/memory slot control.
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cParameter.h"

namespace DadGUI {

//**********************************************************************************
// Class: cSwitchOnOff
// Description: Handles simple On/Off/Bypass toggle using footswitch interactions
//**********************************************************************************
class cSwitchOnOff : public iUIComponent {
public:
    cSwitchOnOff() = default;
    virtual ~cSwitchOnOff() = default;

    //-----------------------------------------------------------------------------------
    // Function: Init
    // Description: Initializes the component with switch reference and effect ID
    //-----------------------------------------------------------------------------------
    void Init(DadDrivers::cSwitch* pFootSwitch, uint32_t EffectID);

    //-----------------------------------------------------------------------------------
    // Function: Update
    // Description: Periodically updates switch state and detects user actions
    //-----------------------------------------------------------------------------------
    void Update() override;

    //-----------------------------------------------------------------------------------
    // Function: Activate
    // Description: Called when component becomes active and visible
    //-----------------------------------------------------------------------------------
    void Activate() override {};

    //-----------------------------------------------------------------------------------
    // Function: Deactivate
    // Description: Called when component is deactivated or hidden
    //-----------------------------------------------------------------------------------
    void Deactivate() override {};

    //-----------------------------------------------------------------------------------
    // Function: Redraw
    // Description: Forces the component to redraw
    //-----------------------------------------------------------------------------------
    void Redraw() override {};

protected:
    // =============================================================================
    // Section: Member Variables
    // =============================================================================

    uint32_t m_OldPressCount = 0;                // Tracks previous press count for change detection
    uint32_t m_LastPressTime = 0;                // Timestamp of last press for double-tap detection
    DadDrivers::cSwitch* m_pFootSwitch = nullptr; // Pointer to physical footswitch hardware
};

//**********************************************************************************
// Enumeration: eTempoType
// Description: Defines tap-tempo output type for parameter control
//**********************************************************************************
enum class eTempoType {
    period,     // Sets tap-tempo as duration in milliseconds
    frequency,  // Sets tap-tempo as frequency in Hertz
    none        // Disables tap-tempo control
};

//**********************************************************************************
// Class: cTapTempoMemChange
// Description: Manages tap-tempo detection and memory slot changes via footswitch
//**********************************************************************************
class cTapTempoMemChange : public iUIComponent {
public:
    cTapTempoMemChange() = default;
    virtual ~cTapTempoMemChange() = default;

    //-----------------------------------------------------------------------------------
    // Function: Init
    // Description: Initializes component with switch, parameter, effect ID and tempo type
    //-----------------------------------------------------------------------------------
    void Init(DadDrivers::cSwitch* pFootSwitch,
              DadDSP::cParameter* pParameter,
              uint32_t EffectID,
              eTempoType TempoType = eTempoType::none);

    //-----------------------------------------------------------------------------------
    // Function: Update
    // Description: Periodically updates tempo calculation and memory change logic
    //-----------------------------------------------------------------------------------
    void Update() override;

    //-----------------------------------------------------------------------------------
    // Function: setParameter
    // Description: Sets the DSP parameter to control with tap tempo
    //-----------------------------------------------------------------------------------
    inline void setParameter(DadDSP::cParameter* pParameter) {
        m_pParameter = pParameter;
    }

    //-----------------------------------------------------------------------------------
    // Function: setTempoType
    // Description: Sets the type of tempo output (period, frequency, or none)
    //-----------------------------------------------------------------------------------
    inline void setTempoType(eTempoType TempoType) {
        m_TempoType = TempoType;
    }

    //-----------------------------------------------------------------------------------
    // Function: Activate
    // Description: Called when component becomes active and visible
    //-----------------------------------------------------------------------------------
    void Activate() override {};

    //-----------------------------------------------------------------------------------
    // Function: Deactivate
    // Description: Called when component is deactivated or hidden
    //-----------------------------------------------------------------------------------
    void Deactivate() override {};

    //-----------------------------------------------------------------------------------
    // Function: Redraw
    // Description: Forces the component to redraw
    //-----------------------------------------------------------------------------------
    void Redraw() override {};

protected:
    // =============================================================================
    // Section: Member Variables
    // =============================================================================

    uint32_t m_OldPressCount = 0;                    // Tracks previous press count for change detection
    uint32_t m_PeriodUpdateCount = 0;                // Tracks tempo period update count
    DadDrivers::cSwitch* m_pFootSwitch = nullptr;    // Pointer to physical footswitch hardware
    eTempoType m_TempoType = eTempoType::none;       // Type of tempo output calculation
    DadDSP::cParameter* m_pParameter = nullptr;      // Pointer to linked DSP parameter for tempo control
};

} // namespace DadGUI
//***End of file**************************************************************
