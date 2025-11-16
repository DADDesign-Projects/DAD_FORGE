//==================================================================================
//==================================================================================
// File: TemplateMultiModeEffect.h
// Description: Header file for template multi-mode effects
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "MultiModeEffect.h"

namespace DadEffect {

//**********************************************************************************
// Class: cTemplateMultiModeEffect1
// Description: First template multi-mode effect class
//**********************************************************************************

constexpr uint32_t TEMPLATE_MULTI_1_ID BUILD_ID('T', 'E', 'M', '1');

class cTemplateMultiModeEffect1 : public cMultiModeEffect {
public:
    // -----------------------------------------------------------------------------
    // onInitialize
    // Initializes effect parameters and configuration
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // onActivate
    // Called when effect becomes active
    // -----------------------------------------------------------------------------
    void onActivate() override;

    // -----------------------------------------------------------------------------
    // onDesactivate
    // Called when effect becomes inactive
    // -----------------------------------------------------------------------------
    void onDesactivate() override;

    // -----------------------------------------------------------------------------
    // Process
    // Audio processing method - applies effect to input buffer
    // -----------------------------------------------------------------------------
    void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff) override;

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    DadGUI::cUIParameter                 m_ParameterDemo1;              // Demo parameter 1 - volume control
    DadGUI::cUIParameter                 m_ParameterDemo2;              // Demo parameter 2 - range parameter
    DadGUI::cUIParameter                 m_ParameterDemo3;              // Demo parameter 3 - discrete values

    DadGUI::cParameterNumNormalView      m_ParameterDemo1View;          // View for parameter 1 - normal numeric
    DadGUI::cParameterNumLeftRightView   m_ParameterDemo2View;          // View for parameter 2 - left/right numeric
    DadGUI::cParameterDiscretView        m_ParameterDemo3View;          // View for parameter 3 - discrete selection

    DadGUI::cPanelOfParameterView        m_ParametrerDemoPanel;         // Panel containing all parameter views
};

//**********************************************************************************
// Class: cTemplateMultiModeEffect2
// Description: Second template multi-mode effect class
//**********************************************************************************

constexpr uint32_t TEMPLATE_MULTI_2_ID BUILD_ID('T', 'E', 'M', '2');

class cTemplateMultiModeEffect2 : public cMultiModeEffect {
public:
    // -----------------------------------------------------------------------------
    // onInitialize
    // Initializes effect parameters and configuration
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // onActivate
    // Called when effect becomes active
    // -----------------------------------------------------------------------------
    void onActivate() override;

    // -----------------------------------------------------------------------------
    // onDesactivate
    // Called when effect becomes inactive
    // -----------------------------------------------------------------------------
    void onDesactivate() override;

    // -----------------------------------------------------------------------------
    // Process
    // Audio processing method - applies effect to input buffer
    // -----------------------------------------------------------------------------
    void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff) override;

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    DadGUI::cUIParameter                 m_ParameterDemo1;              // Demo parameter 1 - volume control
    DadGUI::cUIParameter                 m_ParameterDemo2;              // Demo parameter 2 - range parameter
    DadGUI::cUIParameter                 m_ParameterDemo3;              // Demo parameter 3 - discrete values

    DadGUI::cParameterNumNormalView      m_ParameterDemo1View;          // View for parameter 1 - normal numeric
    DadGUI::cParameterNumLeftRightView   m_ParameterDemo2View;          // View for parameter 2 - left/right numeric
    DadGUI::cParameterDiscretView        m_ParameterDemo3View;          // View for parameter 3 - discrete selection

    DadGUI::cPanelOfParameterView        m_ParametrerDemoPanel;         // Panel containing all parameter views
};

//**********************************************************************************
// Class: cTemplateMainMultiModeEffect
// Description: Main multi-mode effect container and manager
//**********************************************************************************

constexpr uint8_t NB_EFFECTS = 2;                                       // Number of available effects

class cTemplateMainMultiModeEffect : public cMainMultiModeEffect {
public:
    // -----------------------------------------------------------------------------
    // Initialize
    // Initializes all effect instances and sets up the multi-mode system
    // -----------------------------------------------------------------------------
    void Initialize();

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    cMultiModeEffect* m_TabEffects[NB_EFFECTS];                         // Array of effect instances
};

} // namespace DadEffect

//***End of file********************************************************************
