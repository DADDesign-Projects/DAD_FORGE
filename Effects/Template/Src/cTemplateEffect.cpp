//==================================================================================
//==================================================================================
// File: cTemplateEffect.cpp
// Description: Template effect implementation for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "EffectsConfig.h"
#ifdef TEMPLATE_EFFECT
#include "cTemplateEffect.h"
#include "GPIO.h"

namespace DadEffect {

constexpr uint32_t TEMPLATE_ID BUILD_ID('T', 'E', 'M', 'P');

//**********************************************************************************
// Class: cTemplateEffect
//**********************************************************************************

// -----------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes DSP components and user interface parameters
// -----------------------------------------------------------------------------
void cTemplateEffect::onInitialize(){
    // Initialize gain parameter with DSP configuration
    m_ParameterGain.Init(TEMPLATE_ID, // SerializeID
                         50.0f,       // Initial Value
                         0.0f,        // Min Value
                         100.0f,      // Max Value
                         5.0f,        // Rapid Increment
                         1.0f,        // Slow Increment
                         nullptr,     // Callback
                         0,           // CallbackUserData
                         0.5f,        // Slope 0.5 seconds for change Min to Max
                         20);         // Midi CC

    // Initialize parameter views for GUI display
    m_ParameterGainView.Init(&m_ParameterGain,     // Parameter
                             "Gain",               // Parameter short name
                             "Gain",               // Parameter long name
                             "%",                  // Unit Short name
                             "percent");           // Unit Long name

    // Initialize parameter panels for user interface
    m_ParametrerDemoPanel.Init(&m_ParameterGainView,  // Parameter View 1
                               nullptr,               // Parameter View 2
                               nullptr);              // Parameter View 3

    // Add panel to menu system
    m_Menu.addMenuItem(&m_ParametrerDemoPanel,     // Panel
                       "Demo");                    // menu item name

}

// -----------------------------------------------------------------------------
// Method: getEffectID
// Description: Returns the unique effect identifier
// -----------------------------------------------------------------------------
uint32_t cTemplateEffect::getEffectID(){
    return TEMPLATE_ID;
}

// -----------------------------------------------------------------------------
// Method: Process
// Description: Audio processing function - processes one input/output audio buffer
// -----------------------------------------------------------------------------
void cTemplateEffect::onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence){
    // Retrieve current gain value from parameter system
    float gainValue = m_ParameterGain.getValue()/100;
    
    // Apply gain to both left and right audio channels
    pOut->Left = pIn->Left * gainValue;
    pOut->Right = pIn->Right * gainValue;
}

} // namespace DadEffect
#endif
//***End of file**************************************************************
