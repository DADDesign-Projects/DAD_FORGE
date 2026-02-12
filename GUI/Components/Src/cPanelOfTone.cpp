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
    m_Bass.Init(  SerializeID, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, BassChange, (uint32_t) this , 0.5f, 100);
    m_Mid.Init(   SerializeID, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, MidChange, (uint32_t) this , 0.5f, 101);
    m_Treble.Init(SerializeID, 0.0f, -10.0f, +10.0f, 1.0f, 0.5f, TrebleChange, (uint32_t) this , 0.5f, 102);

    // Initialize tone view
    m_BassView.Init(&m_Bass, "Bass", "Bass", "dB", "dB");
    m_MidView.Init(&m_Mid, "Mid", "Mid", "dB", "dB");
    m_TrebleView.Init(&m_Treble, "Treble", "Treble", "dB", "dB");

    // Subscribe to the RT_ProcessOut event
    __GUI_EventManager.Subscribe_RT_ProcessOut(this);

    // Initialize cPanelOfParameterView
    cPanelOfParameterView::Init(&m_BassView, &m_MidView, &m_TrebleView);
}

// -----------------------------------------------------------------------------
// Updates the panel state
// -----------------------------------------------------------------------------
void cPanelOfTone::Update() {
    if (m_isActive) {
        cPanelOfParameterView::Update(); // Update base class functionality
    }
}

// -----------------------------------------------------------------------------
// Process audio
// -----------------------------------------------------------------------------
void cPanelOfTone::Process(AudioBuffer *pIn, AudioBuffer *pOut){
	m_BassBiQuad.ProcessFlast12dbStereo(pIn, pOut);
	m_MidBiQuad.ProcessFlast12dbStereo(pOut, pOut);
	m_TrebleBiQuad.ProcessFlast12dbStereo(pOut, pOut);
}

// -----------------------------------------------------------------------------
// Callback for adjusting filters based on the GUI parameter.
// -----------------------------------------------------------------------------
void cPanelOfTone::BassChange(DadDSP::cParameter* pParameter, uint32_t Data){
	cPanelOfTone* pthis = (cPanelOfTone*) Data;
    pthis->m_BassBiQuad.setGainDb(pParameter->getValue());
    pthis->m_BassBiQuad.CalculateParameters();
}

void cPanelOfTone::MidChange(DadDSP::cParameter* pParameter, uint32_t Data){
	cPanelOfTone* pthis = (cPanelOfTone*) Data;
    pthis->m_MidBiQuad.setGainDb(pParameter->getValue());
    pthis->m_MidBiQuad.CalculateParameters();
}

void cPanelOfTone::TrebleChange(DadDSP::cParameter* pParameter, uint32_t Data){
	cPanelOfTone* pthis = (cPanelOfTone*) Data;
    pthis->m_TrebleBiQuad.setGainDb(pParameter->getValue());
    pthis->m_TrebleBiQuad.CalculateParameters();
}

} // namespace DadGUI
//***End of file**************************************************************
