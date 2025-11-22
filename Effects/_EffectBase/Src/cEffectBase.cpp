//==================================================================================
//==================================================================================
// File: cEffectBase.cpp
// Description: Base effect implementation for DSP audio processing
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cEffectBase.h"
#include "GPIO.h"

namespace DadEffect {

// -----------------------------------------------------------------------------
// Initializes DSP components and user interface parameters
void cEffectBase::Initialize()
{
    m_Menu.Init();
    onInitialize();
    uint32_t EffectID = getEffectID();

    // Initialize parameter panels
    m_MemoryPanel.Init(EffectID);
    m_VuMeterPanel.Init();
    m_PanelOfSystemView.Initialize(EffectID);

    // Initialize UI components
    m_InfoView.Init();
    m_SwitchOnOff.Init(&__Switch1, EffectID);
    m_SwitchTempoMem.Init(&__Switch2, nullptr, EffectID, DadGUI::eTempoType::period);

    // Build menu structure
    m_Menu.addMenuItem(&m_MemoryPanel,           "Memory");
    m_Menu.addMenuItem(&m_VuMeterPanel,          "Vu-Meter");
    m_Menu.addMenuItem(&m_PanelOfSystemView,     "System");

    // Configure GUI identifiers and components
    __GUI.setUpdateID(EffectID);
    __GUI.setSerializeID(EffectID);
    __GUI.setRtProcessID(EffectID);

    __GUI.activeBackComponent(&m_InfoView);
    __GUI.activeMainComponent(&m_Menu);

    // Initialize audio processing settings
    __DryWet.setMix(1.0f);
    SetPIN(AUDIO_MUTE);
}

} // namespace DadEffect

//***End of file**************************************************************
