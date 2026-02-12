//==================================================================================
//==================================================================================
// File: cPanelOfEffectChoice.h
// Description: Effect choice panel header
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "iUIComponent.h"
#include "cUIParameter.h"
#include "ParameterViews.h"
#include "cPanelOfParameters.h"
#include "cDisplay.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfEffectChoice
// Description: Panel for selecting effects in the user interface
//**********************************************************************************
using EffectChangeCallback_t = void (*)(DadDSP::cParameter*, uint32_t);

class cPanelOfEffectChoice :
		public cPanelOfParameterView
{
public:
    virtual ~cPanelOfEffectChoice() {}

    // -----------------------------------------------------------------------------
    // Initializes the effect choice panel
    // -----------------------------------------------------------------------------
    void Initialize(uint32_t SerializeID, EffectChangeCallback_t Callback, uint32_t ContextCallback);

    // -----------------------------------------------------------------------------
    // Adds an effect to the choice list
    // -----------------------------------------------------------------------------
    void addEffect(const char* ShortName, const char* LongName);

    // -----------------------------------------------------------------------------
    // Get the current selected effect
    // -----------------------------------------------------------------------------
    uint8_t getEffect(){
    	return (uint8_t) m_ParameterChoice.getTargetValue();
    }

    // -----------------------------------------------------------------------------
    // Activates and displays the panel
    // -----------------------------------------------------------------------------
    void Activate() override;

    // -----------------------------------------------------------------------------
    // Deactivates and hides the panel
    // -----------------------------------------------------------------------------
    void Deactivate() override;

    // -----------------------------------------------------------------------------
    // Updates panel state and parameters
    // -----------------------------------------------------------------------------
    void Update() override;

    // -----------------------------------------------------------------------------
    // Forces redraw of UI component
    // -----------------------------------------------------------------------------
    void Redraw() override;

protected:
    // -----------------------------------------------------------------------------
    // Draws the panel contents on the display layer
    // -----------------------------------------------------------------------------
    void Draw();

protected:
    //**********************************************************************************
    // Member Variables
    //**********************************************************************************

    // =============================================================================
    // State Management
    // =============================================================================
    bool                m_isActive;          // Indicates whether the UI is active

    // =============================================================================
    // Display System
    // =============================================================================
    DadGFX::cLayer*     m_pLayer;            // Main display layer for the panel

    // =============================================================================
    // Parameter Management
    // =============================================================================
    cUIParameter        m_ParameterChoice;   // Parameter for effect choice selection
    cParameterDiscretView m_ParameterChoiceView; // View for displaying parameter choices

    // =============================================================================
    // Callback System
    // =============================================================================
    DadDSP::CallbackType m_Callback;         // Callback function for parameter changes
    uint32_t            m_ContextCallback;   // Context for callback function

};

} // namespace DadGUI

//*** End of file ************************************************************
