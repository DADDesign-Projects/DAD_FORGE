//==================================================================================
//==================================================================================
// File: cTemplateEffect.cpp
// Description: Template effect implementation for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cTemplateEffect.h"
#include "GPIO.h"

namespace DadEffect {

constexpr uint32_t TEMPLATE_ID BUILD_ID('T', 'E', 'M', 'P');

// -----------------------------------------------------------------------------
// Initializes DSP components and user interface parameters
void cTemplateEffect::onInitialize(){
    // Initialize gain parameter
    m_ParameterGain.Init(TEMPLATE_ID, 			//  SerializeID
    		             TEMPLATE_ID, 			//  RTProcessID
						 50.0f,					//  Initial Value
						 0.0f,					//  Min Value
						 100.0f,				//  Max Value
						 5.0f,					//  Rapid Increment
						 1.0f,					//  Slow Increment
						 nullptr,				//  Callback
						 0,						//  CallbackUserData
						 0.5f*RT_RATE,			//  Slope 0.5 seconds for change Min to Max
						 20);					//  Midi CC

    // Initialize parameter views for GUI display
    m_ParameterGainView.Init(&m_ParameterGain,	// Parameter
    						 "Gain",			// Parameter short name
							 "Gain",			// Parameter long name
							 "%",				// Unit Short name
							 "percent");		// Unit Long name

    // Initialize parameter panels
    m_ParametrerDemoPanel.Init( &m_ParameterGainView,	// Parameter View 1
    							nullptr,				// Parameter View 2
								nullptr);				// Parameter View 3

    // Add panel to menu
    m_Menu.addMenuItem(&m_ParametrerDemoPanel,			// Panel
    				  "Demo");							// menu item name
}

// -----------------------------------------------------------------------------
//
uint32_t cTemplateEffect::getEffectID(){
	return TEMPLATE_ID;
}

// -----------------------------------------------------------------------------
// Audio processing function: processes one input/output audio buffer
ITCM void cTemplateEffect::Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff){
    // Apply dry/wet mix to both channels
    pOut->Left = pIn->Left * m_ParameterGain.getValue();
    pOut->Right = pIn->Right * m_ParameterGain.getValue();
}

} // namespace DadEffect

//***End of file**************************************************************
