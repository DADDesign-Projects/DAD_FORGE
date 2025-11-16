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
void cTemplateEffect::Initialize()
{
    // Initialize demo parameters with various ranges and characteristics
    m_ParameterDemo1.Init(TEMPLATE_ID, TEMPLATE_ID, 0.0f, 0.0f, 1.0f, 0.10f, 0.05f, nullptr, 0, .5, 20);  // Parameter 1: 0-1 range
    m_ParameterDemo2.Init(TEMPLATE_ID, TEMPLATE_ID, 0.0f, -100.0f, 100.0f, 10.0f, 1.0f, nullptr, .5, 0, 21); // Parameter 2: -100 to 100 range
    m_ParameterDemo3.Init(TEMPLATE_ID, TEMPLATE_ID, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, nullptr, 0, 0, 22);    // Discrete parameter
    m_ParameterMix.Init(TEMPLATE_ID, TEMPLATE_ID, 50.0f, 0.0f, 100.0f, 10.0f, 1.0f, MixCallBack, (uint32_t) this, .5, 23); // Mix parameter with callback

    // Initialize parameter views for GUI display
    m_ParameterDemo1View.Init(&m_ParameterDemo1, "Param1", "Parameter 1", "U", "Unit");
    m_ParameterDemo2View.Init(&m_ParameterDemo2, "Param2", "Parameter 2", "U", "Unit");
    m_ParameterDemo3View.Init(&m_ParameterDemo3, "Discrete", "Discrete Value");

    // Add discrete values for parameter 3
    m_ParameterDemo3View.AddDiscreteValue("D1", "DiscretVal1");
    m_ParameterDemo3View.AddDiscreteValue("D2", "DiscretVal2");
    m_ParameterDemo3View.AddDiscreteValue("D3", "DiscretVal3");
    m_ParameterDemo3View.AddDiscreteValue("D4", "DiscretVal4");
    m_ParameterDemo3View.AddDiscreteValue("D5", "DiscretVal5");

    m_ParameterMixView.Init(&m_ParameterMix, "Mix", "Mix", "%", "%", 3);

    // Initialize parameter panels
    m_ParametrerDemoPanel.Init(&m_ParameterDemo1View, &m_ParameterDemo2View, &m_ParameterDemo3View);
    m_ParametrerMixPanel.Init(&m_ParameterMixView, nullptr, nullptr);
    m_MemoryPanel.Init(TEMPLATE_ID);
    m_VuMeterPanel.Init();
    m_PanelOfSystemView.Initialize(TEMPLATE_ID);

    // Initialize UI components
    m_InfoView.Init();
    m_SwitchOnOff.Init(&__Switch1, TEMPLATE_ID);
    m_SwitchTempoMem.Init(&__Switch2, &m_ParameterDemo1, TEMPLATE_ID, DadGUI::eTempoType::period);

    // Build menu structure
    m_Menu.Init();
    m_Menu.addMenuItem(&m_ParametrerDemoPanel,   "Menu1");
    m_Menu.addMenuItem(&m_ParametrerMixPanel,    "Mix");
    m_Menu.addMenuItem(&m_MemoryPanel,           "Memory");
    m_Menu.addMenuItem(&m_VuMeterPanel,          "Vu-Meter");
    m_Menu.addMenuItem(&m_PanelOfSystemView,     "System");

    // Configure GUI identifiers and components
    __GUI.setUpdateID(TEMPLATE_ID);
    __GUI.setSerializeID(TEMPLATE_ID);
    __GUI.setRtProcessID(TEMPLATE_ID);

    __GUI.activeBackComponent(&m_InfoView);
    __GUI.activeMainComponent(&m_Menu);

    // Initialize audio processing settings
    __DryWet.setMix(1.0f);
    SetPIN(AUDIO_MUTE);
}

// -----------------------------------------------------------------------------
// Audio processing function: processes one input/output audio buffer
ITCM void cTemplateEffect::Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff)
{
    // Apply dry/wet mix to both channels
    pOut->Left = pIn->Left * __DryWet.getGainWet();
    pOut->Right = pIn->Right * __DryWet.getGainWet();
}

// -----------------------------------------------------------------------------
// Mix callback function - updates dry/wet mix based on parameter changes
void cTemplateEffect::MixCallBack(DadDSP::cParameter* pParameter, uint32_t Context)
{
    // Update dry/wet mix with new parameter value
    __DryWet.setMix(pParameter->getValue());
}

} // namespace DadEffect

//***End of file**************************************************************
