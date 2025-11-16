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
#include "cDisplay.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfEffectChoice
// Description: Panel for selecting effects in the user interface
//**********************************************************************************
class cPanelOfEffectChoice : public cPanelOfParameterView, public DadPersistentStorage::cSerializedObject {
public:
    virtual ~cPanelOfEffectChoice() {}

    // -----------------------------------------------------------------------------
    // Initializes the effect choice panel
    // -----------------------------------------------------------------------------
    void Initialize(uint32_t SerializeID, DadDSP::CallbackType Callback, uint32_t ContextCallback);

    // -----------------------------------------------------------------------------
    // Adds an effect to the choice list
    // -----------------------------------------------------------------------------
    void addEffect(const char* ShortName, const char* LongName);

    // -----------------------------------------------------------------------------
    // Adds the panel to serialization family
    // -----------------------------------------------------------------------------
    void addToSerializeFamily(uint32_t SerializeID);

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

    // -----------------------------------------------------------------------------
    // Serializes the object (empty implementation)
    // -----------------------------------------------------------------------------
    void Save(DadPersistentStorage::cSerialize* pSerializer) override {};

    // -----------------------------------------------------------------------------
    // Deserializes the object
    // -----------------------------------------------------------------------------
    void Restore(DadPersistentStorage::cSerialize* pSerializer) override;

    // -----------------------------------------------------------------------------
    // Checks if the object is dirty
    // -----------------------------------------------------------------------------
    bool isDirty() override { return false; }

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
    bool                m_CallSending;       // Flag indicating callback is being sent
    bool                m_NeedCallbackSend;  // Flag indicating callback needs to be sent
};

} // namespace DadGUI

//*** End of file ************************************************************
