//==================================================================================
//==================================================================================
// File: cPanelOfTone.cpp
// Description: Managing output audio for bass, mid, treble
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cPanelOfTone.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfTone
//
// Description: Panel for managing output audio (bass, mid, treble)
//
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes the three parameter views
// -----------------------------------------------------------------------------
void cPanelOfTone::Initialize(uint32_t SerializeID) {

    // Initialize tone BiQuad filters
    m_BassBiQuad.Initialize(  SAMPLING_RATE,  150, 0.0f, 1.8f, DadDSP::FilterType::LSH);
    m_MidBiQuad.Initialize(   SAMPLING_RATE,  800, 0.0f, 2.3f, DadDSP::FilterType::PEQ);
    m_TrebleBiQuad.Initialize(SAMPLING_RATE, 6500, 0.0f, 1.8f, DadDSP::FilterType::HSH);

    // Initialize tone control filters
    m_Bass.Init(  SerializeID, 0, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, BassCallback,   (uint32_t) this, 0, 100);
    m_Mid.Init(   SerializeID, 0, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, MidCallback,    (uint32_t) this, 0, 101);
    m_Treble.Init(SerializeID, 0, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, TrebleCallback, (uint32_t) this, 0, 102);

    // Initialize tone view
    m_BassView.Init(&m_Bass, "Bass", "Bass", "dB", "dB");
    m_MidView.Init(&m_Mid, "Mid", "Mid", "dB", "dB");
    m_TrebleView.Init(&m_Treble, "Treble", "Treble", "dB", "dB");

    // Initialize the parameter view with theme and MIDI controls
    cPanelOfParameterView::Init(&m_BassView, &m_MidView, &m_TrebleView);
}

// -----------------------------------------------------------------------------
// Adds serializable objects to the serialization family
// -----------------------------------------------------------------------------
void cPanelOfTone::addToSerializeFamily(uint32_t SerializeID) {
    __GUI.addSerializeObject(&m_Bass, SerializeID);
    __GUI.addSerializeObject(&m_Mid, SerializeID);
    __GUI.addSerializeObject(&m_Treble, SerializeID);
}

// -----------------------------------------------------------------------------
// Updates the panel state
// -----------------------------------------------------------------------------
void cPanelOfTone::Update() {
    if (m_isActive) {
    	//m_Bass.Process();
    	//m_Mid.Process();
    	//m_Treble.Process();
        cPanelOfParameterView::Update(); // Update base class functionality
    }
}

// -----------------------------------------------------------------------------
// Process audio
// -----------------------------------------------------------------------------
ITCM void cPanelOfTone::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff){
	float OutLeft;
	float OutRight;

	OutLeft = m_BassBiQuad.Process(pIn->Left, DadDSP::eChannel::Left);
	OutLeft = m_MidBiQuad.Process(OutLeft, DadDSP::eChannel::Left);
	pOut->Left =  m_TrebleBiQuad.Process(OutLeft, DadDSP::eChannel::Left);

	OutRight = m_BassBiQuad.Process(pIn->Right, DadDSP::eChannel::Right);
	OutRight = m_MidBiQuad.Process(OutRight, DadDSP::eChannel::Right);
	pOut->Right =  m_TrebleBiQuad.Process(OutRight, DadDSP::eChannel::Right);

}

// -----------------------------------------------------------------------------
// Callback for bass parameter changes
// -----------------------------------------------------------------------------
void cPanelOfTone::BassCallback(DadDSP::cParameter* pParameter, uint32_t Context){
	cPanelOfTone *pthis = (cPanelOfTone *)Context;  // Get class instance
    pthis->m_BassBiQuad.setGainDb(pParameter->getValue());
    pthis->m_BassBiQuad.CalculateParameters();
}

// -----------------------------------------------------------------------------
// Callback for mid parameter changes
// -----------------------------------------------------------------------------
void cPanelOfTone::MidCallback(DadDSP::cParameter* pParameter, uint32_t Context){
	cPanelOfTone *pthis = (cPanelOfTone *)Context;  // Get class instance
    pthis->m_MidBiQuad.setGainDb(pParameter->getValue());
    pthis->m_MidBiQuad.CalculateParameters();
}

// -----------------------------------------------------------------------------
// Callback for treble parameter changes
// -----------------------------------------------------------------------------
void cPanelOfTone::TrebleCallback(DadDSP::cParameter* pParameter, uint32_t Context){
	cPanelOfTone *pthis = (cPanelOfTone *)Context;  // Get class instance
    pthis->m_TrebleBiQuad.setGainDb(pParameter->getValue());
    pthis->m_TrebleBiQuad.CalculateParameters();
}

} // namespace DadGUI
//***End of file**************************************************************
