//==================================================================================
//==================================================================================
// File: cUniVibe.cpp
// Description: UniVibe audio effect implementation
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUniVibe.h"

// Frequency constants for LFO
constexpr float UN_LFO_FREQ_MAX = 10;
constexpr float UN_LFO_FREQ_MIN = 0.5;

// Frequency constants for APF
constexpr float UN_APF1_FREQ_MAX = 800;
constexpr float UN_APF1_FREQ_MIN = 100;

constexpr float UN_APF2_FREQ_MAX = 1500;
constexpr float UN_APF2_FREQ_MIN = 300;

constexpr float UN_APF3_FREQ_MAX = 3000;
constexpr float UN_APF3_FREQ_MIN = 800;

constexpr float UN_APF4_FREQ_MAX = 5000;
constexpr float UN_APF4_FREQ_MIN = 1500;

namespace DadEffect {

//**********************************************************************************
// Class: cUniVibe
// Description: Implements UniVibe phaser effect using modulated all-pass filters
//**********************************************************************************

// =============================================================================
// PUBLIC METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cUniVibe::onInitialize() {
    // Initialize effect identification
    m_pShortName = "UniVibe";  // Short name identifier
    m_pLongName  = "UniVibe";  // Long descriptive name
    m_ID = UNIVIBE_ID;         // Unique effect identifier

    // =============================================================================
    // DSP INITIALIZATION SECTION
    // =============================================================================

    // Initialize all-pass filters
    m_AllPass1.Initialize(SAMPLING_RATE);
    m_AllPass2.Initialize(SAMPLING_RATE);
    m_AllPass3.Initialize(SAMPLING_RATE);
    m_AllPass4.Initialize(SAMPLING_RATE);

    // Initialize LFO with frequency range
    m_LFO.Initialize(SAMPLING_RATE, UN_LFO_FREQ_MIN, UN_LFO_FREQ_MIN, UN_LFO_FREQ_MAX, 0.7f);

    // =============================================================================
    // PARAMETER INITIALIZATION SECTION
    // =============================================================================

    // Initialize effect depth parameter
    m_Deep.Init(UNIVIBE_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 0.8f * RT_RATE, 30);

    // Initialize effect speed parameter with callback
    m_Speed.Init(UNIVIBE_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, SpeedChange, 0, 2.0f * RT_RATE, 31);

    // Initialize dry/wet mix parameter with callback
    m_DryWetMix.Init(UNIVIBE_ID, 0.0f, 0.0f, 100.0f, 5.0f, 1.0f, MixChange, (uint32_t)this, 3.0f * RT_RATE, 32);

    // =============================================================================
    // VIEW SETUP SECTION
    // =============================================================================

    // Initialize parameter views for user interface
    m_DeepView.Init(&m_Deep, "Deep", "Deep", "%", "%");
    m_SpeedView.Init(&m_Speed, "Speed", "Speed", "%", "%");
    m_DryWetMixView.Init(&m_DryWetMix, "Mix", "Mix", "%", "%");

    // =============================================================================
    // MENU GROUPING SECTION
    // =============================================================================

    // Group parameters into panels for menu organization
    m_PanelUniVibe.Init(&m_DeepView, &m_SpeedView, &m_DryWetMixView);

    // =============================================================================
    // MAIN MENU CONFIGURATION SECTION
    // =============================================================================

    // Add panels to main menu
    m_Menu.addMenuItem(&m_PanelUniVibe, "UniVibe");
}

// ---------------------------------------------------------------------------------
// Method: onActivate
// Description: Called when effect becomes active
// ---------------------------------------------------------------------------------
void cUniVibe::onActivate() {
    // Set initial dry/wet mix when effect is activated
    __DryWet.setMix(m_DryWetMix.getValue());

    // Reset all-pass filter states for left channel
    m_APFStateL1 = {};
    m_APFStateL2 = {};
    m_APFStateL3 = {};
    m_APFStateL4 = {};

    // Reset all-pass filter states for right channel
    m_APFStateR1 = {};
    m_APFStateR2 = {};
    m_APFStateR3 = {};
    m_APFStateR4 = {};
}

// ---------------------------------------------------------------------------------
// Method: onDesactivate
// Description: Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cUniVibe::onDesactivate() {
    // Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Method: Process
// Description: Audio processing method - applies UniVibe phaser effect
// ---------------------------------------------------------------------------------
void cUniVibe::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff) {
    // Update LFO position
    m_LFO.Step();

    // Get LFO modulation value scaled by depth parameter
    float LFO = m_LFO.getSineValue() * m_Deep.getNormalizedValue();

    // Calculate modulated frequencies for each all-pass filter
    float f1 = UN_APF1_FREQ_MIN + (LFO * (UN_APF1_FREQ_MAX - UN_APF1_FREQ_MIN));
    float f2 = UN_APF2_FREQ_MIN + (LFO * (UN_APF2_FREQ_MAX - UN_APF2_FREQ_MIN));
    float f3 = UN_APF3_FREQ_MIN + (LFO * (UN_APF3_FREQ_MAX - UN_APF3_FREQ_MIN));
    float f4 = UN_APF4_FREQ_MIN + (LFO * (UN_APF4_FREQ_MAX - UN_APF4_FREQ_MIN));

    // Set new frequencies for all-pass filters
    m_AllPass1.SetFrequency(f1);
    m_AllPass2.SetFrequency(f2);
    m_AllPass3.SetFrequency(f3);
    m_AllPass4.SetFrequency(f4);

    // Process left channel through all-pass filter cascade
    float OutLeft = m_AllPass1.Process(pIn->Left, m_APFStateL1);
    OutLeft = m_AllPass2.Process(OutLeft, m_APFStateL2);
    OutLeft = m_AllPass3.Process(OutLeft, m_APFStateL3);
    OutLeft = m_AllPass4.Process(OutLeft, m_APFStateL4);

    // Process right channel through all-pass filter cascade
    float OutRight = m_AllPass1.Process(pIn->Right, m_APFStateR1);
    OutRight = m_AllPass2.Process(OutLeft, m_APFStateR2);
    OutRight = m_AllPass3.Process(OutLeft, m_APFStateR3);
    OutRight = m_AllPass4.Process(OutLeft, m_APFStateR4);

    // Apply wet gain to output signals
    pOut->Left = OutLeft * __DryWet.getGainWet();
    pOut->Right = OutRight * __DryWet.getGainWet();
}

// =============================================================================
// CALLBACK METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cUniVibe::MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    // Update dry/wet mix with current parameter value
    __DryWet.setMix(pParameter->getValue());
}

// ---------------------------------------------------------------------------------
// Method: SpeedChange (Callback)
// Description: Updates LFO speed parameter
// ---------------------------------------------------------------------------------
void cUniVibe::SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    cUniVibe* pthis = (cUniVibe*)CallbackUserData;  // Get class instance
    pthis->m_LFO.setNormalizedFreq(pParameter->getNormalizedValue());  // Update LFO frequency
}

} // namespace DadEffect

//***End of file**************************************************************
