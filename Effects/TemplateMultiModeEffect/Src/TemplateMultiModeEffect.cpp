//==================================================================================
//==================================================================================
// File: TemplateMultiModeEffect.cpp
// Description: Implementation of template multi-mode effects
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "EffectsConfig.h"
#ifdef TEMPLATE_MULTI_MODE_EFFECT
#include "TemplateMultiModeEffect.h"

namespace DadEffect {

//**********************************************************************************
// Class: cTemplateMultiModeEffect1
// Description: First template multi-mode effect implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// onInitialize
// Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect1::onInitialize(){
	m_pShortName = "Effect1";                              // Short name identifier
	m_pLongName	 = "Effect 1";                             // Long descriptive name
	m_ID = TEMPLATE_MULTI_1_ID;                            // Unique effect identifier

	// Initialize effect parameters with their ranges and default values
	m_ParameterDemo1.Init(TEMPLATE_MULTI_1_ID, 0.0f, 0.0f, 1.0f, 0.10f, 0.05f, nullptr, 0, .5, 20);
	m_ParameterDemo2.Init(TEMPLATE_MULTI_1_ID, 0.0f, -100.0f, 100.0f, 10.0f, 1.0f, nullptr, .5, 0, 21);
	m_ParameterDemo3.Init(TEMPLATE_MULTI_1_ID, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, nullptr, 0, 0, 22);

	// Configure parameter views for user interface
	m_ParameterDemo1View.Init(&m_ParameterDemo1, "Volume", "Volume", "%", "%");
	m_ParameterDemo2View.Init(&m_ParameterDemo2, "Param2", "Parameter 2", "U", "Unit");
	m_ParameterDemo3View.Init(&m_ParameterDemo3, "Discrete", "Discrete Value");

	// Add discrete values for parameter 3
	m_ParameterDemo3View.AddDiscreteValue("D1", "Discret Val1");
	m_ParameterDemo3View.AddDiscreteValue("D2", "Discret Val2");
	m_ParameterDemo3View.AddDiscreteValue("D3", "Discret Val3");
	m_ParameterDemo3View.AddDiscreteValue("D4", "Discret Val4");
	m_ParameterDemo3View.AddDiscreteValue("D5", "Discret Val5");

	// Initialize parameter panel and add to menu
	m_ParametrerDemoPanel.Init(&m_ParameterDemo1View, &m_ParameterDemo2View, &m_ParameterDemo3View);
	m_Menu.addMenuItem(&m_ParametrerDemoPanel, "Effect1");
}

// ---------------------------------------------------------------------------------
// onActivate
// Called when effect becomes active
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect1::onActivate(){
	// Activation logic placeholder
	// Activation logic placeholder
	__DryWet.setMix(100);
}

// ---------------------------------------------------------------------------------
// onDesactivate
// Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect1::onDesactivate(){
	// Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Process
// Audio processing method - applies effect to input buffer
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect1::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence){
	// Apply volume scaling when effect is active
	pOut->Left = pIn->Left * m_ParameterDemo1.getValue() * __DryWet.getGainWet();
	pOut->Right = pIn->Right * m_ParameterDemo1.getValue() * __DryWet.getGainWet();
}

//**********************************************************************************
// Class: cTemplateMultiModeEffect2
// Description: Second template multi-mode effect implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// onInitialize
// Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect2::onInitialize(){
	m_pShortName = "Effect2";                              // Short name identifier
	m_pLongName	 = "Effect 2";                             // Long descriptive name
	m_ID = TEMPLATE_MULTI_2_ID;                            // Unique effect identifier

	// Initialize effect parameters with their ranges and default values
	m_ParameterDemo1.Init(TEMPLATE_MULTI_2_ID, 0.0f, 0.0f, 1.0f, 0.10f, 0.05f, nullptr, 0, .5, 20);
	m_ParameterDemo2.Init(TEMPLATE_MULTI_2_ID, 0.0f, -100.0f, 100.0f, 10.0f, 1.0f, nullptr, .5, 0, 21);
	m_ParameterDemo3.Init(TEMPLATE_MULTI_2_ID, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, nullptr, 0, 0, 22);

	// Configure parameter views for user interface
	m_ParameterDemo1View.Init(&m_ParameterDemo1, "Volume", "Volume", "%", "%");
	m_ParameterDemo2View.Init(&m_ParameterDemo2, "Param2", "Parameter 2", "U", "Unit");
	m_ParameterDemo3View.Init(&m_ParameterDemo3, "Discrete", "Discrete Value");

	// Add discrete values for parameter 3
	m_ParameterDemo3View.AddDiscreteValue("D1", "Discret Val1");
	m_ParameterDemo3View.AddDiscreteValue("D2", "Discret Val2");
	m_ParameterDemo3View.AddDiscreteValue("D3", "Discret Val3");
	m_ParameterDemo3View.AddDiscreteValue("D4", "Discret Val4");
	m_ParameterDemo3View.AddDiscreteValue("D5", "Discret Val5");

	// Initialize parameter panel and add to menu
	m_ParametrerDemoPanel.Init(&m_ParameterDemo1View, &m_ParameterDemo2View, &m_ParameterDemo3View);
	m_Menu.addMenuItem(&m_ParametrerDemoPanel, "Effect2");
}

// ---------------------------------------------------------------------------------
// onActivate
// Called when effect becomes active
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect2::onActivate(){
	// Activation logic placeholder
	__DryWet.setMix(100);

}

// ---------------------------------------------------------------------------------
// onDesactivate
// Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect2::onDesactivate(){
	// Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Process
// Audio processing method - applies effect to input buffer
// ---------------------------------------------------------------------------------
void cTemplateMultiModeEffect2::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence){
	// Apply volume scaling when effect is active
	pOut->Left = pIn->Left * m_ParameterDemo1.getValue() * __DryWet.getGainWet();
	pOut->Right = pIn->Right * m_ParameterDemo1.getValue() * __DryWet.getGainWet();
}

//**********************************************************************************
// Class: cTemplateMainMultiModeEffect
// Description: Main multi-mode effect container and manager
//**********************************************************************************

// Storage buffers for effect instances with proper alignment
alignas(4) uint8_t TemplateMultiModeEffect1Buffer[sizeof(cTemplateMultiModeEffect1)];
alignas(4) uint8_t TemplateMultiModeEffect2Buffer[sizeof(cTemplateMultiModeEffect2)];

// ---------------------------------------------------------------------------------
// Initialize
// Initializes all effect instances and sets up the multi-mode system
// ---------------------------------------------------------------------------------
void cTemplateMainMultiModeEffect::Initialize(){
	// Create effect instances using placement new in pre-allocated buffers
	m_TabEffects[0] = new (TemplateMultiModeEffect1Buffer) cTemplateMultiModeEffect1();
	m_TabEffects[1] = new (TemplateMultiModeEffect2Buffer) cTemplateMultiModeEffect2();

	// Configure multi-mode effect system
	m_NbEffects = NB_EFFECTS;                              // Set number of available effects
	m_pActiveEffect = m_TabEffects[0];                     // Set default active effect
	m_pTabEffects = &m_TabEffects[0];                      // Set effects array pointer

	// Initialize base multi-mode effect functionality
	cMainMultiModeEffect::Initialize();
}

} // namespace DadEffect
#endif
//***End of file********************************************************************
