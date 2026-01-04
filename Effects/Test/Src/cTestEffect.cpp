//==================================================================================
//==================================================================================
// File: cTestEffect.cpp
// Description: Template effect implementation for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cTestEffect.h"
#include "GPIO.h"

namespace DadEffect {

constexpr uint32_t TEST_ID BUILD_ID('T', 'E', 'S', 'T');

//**********************************************************************************
// Class: cTestEffect
//**********************************************************************************

// -----------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes DSP components and user interface parameters
// -----------------------------------------------------------------------------
void cTestEffect::onInitialize(){
    // Initialize gain parameter with DSP configuration
    m_ParameterMix.Init(TEST_ID,              // SerializeID
                         0.0f,                    // Initial Value
                         0.0f,                     // Min Value
                         200.0f,                   // Max Value
                         10.0f,                    // Rapid Increment
                         1.0f,                     // Slow Increment
                         MixChange,                // Callback
                         (uint32_t)this,           // CallbackUserData
                         0,                        // Slope 0.5 seconds for change Min to Max
                         20);                      // Midi CC

    // Initialize parameter views for GUI display
    m_ParameterMixView.Init(&m_ParameterMix,     // Parameter
                             "Mix",               // Parameter short name
                             "Mix",               // Parameter long name
                             "%",                  // Unit Short name
                             "percent");           // Unit Long name

    // Initialize parameter panels for user interface
    m_ParametrerTestPanel.Init(&m_ParameterMixView,  // Parameter View 1
                               nullptr,               // Parameter View 2
                               nullptr);              // Parameter View 3

    // Add panel to menu system
    m_Menu.addMenuItem(&m_ParametrerTestPanel,     // Panel
                       "Test");                    // menu item name

}

// -----------------------------------------------------------------------------
// Method: getEffectID
// Description: Returns the unique effect identifier
// -----------------------------------------------------------------------------
uint32_t cTestEffect::getEffectID(){
    return TEST_ID;
}

// -----------------------------------------------------------------------------
// Method: Process
// Description: Audio processing function - processes one input/output audio buffer
// -----------------------------------------------------------------------------
ITCM void cTestEffect::onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence){
    // Retrieve current gain value from parameter system
    float gainValue = __DryWet.getGainWet();
    
    // Apply gain to both left and right audio channels
    pOut->Left = pIn->Left * gainValue;
    pOut->Right = pIn->Right * gainValue;
}

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cTestEffect::MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    // Update dry/wet mix with current parameter value
    //__DryWet.setMix(pParameter->getValue());
    //cTestEffect *pthis = reinterpret_cast<cTestEffect *>(CallbackUserData);
	uint16_t Value = pParameter->getValue();
	uint32_t Gain = Value + (Value << 8);
	__SoftSPI.Transmit(Gain);
}

} // namespace DadEffect

//***End of file**************************************************************
