//==================================================================================
//==================================================================================
// File: cFlanger.cpp
// Description: Flanger audio effect implementation
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "EffectsConfig.h"
#ifdef MODULATIONS_EFFECT
#include "cFlanger.h"

// Modulator offset constants for different delay lines
constexpr float FL_MODULATOR_OFFSET_LEFT  = 0.005f;
constexpr float FL_MODULATOR_OFFSET_RIGHT = 0.0052f;

// Modulator frequency constants for different LFOs
constexpr float FL_MODULATOR_FREQ_LEFT  = 1.0f;
constexpr float FL_MODULATOR_FREQ_RIGHT = 1.1f;

// Modulator pitch range limits
constexpr float FL_MODULATOR_PLICH_MAX = 3.0f;
constexpr float FL_MODULATOR_PLICH_MIN = 1.5f;

// Feedback Range
constexpr float FL_FEEDBACK_MAX = 0.7f;
constexpr float FL_FEEDBACK_MIN = 0.3f;

// Feedback Range
constexpr float FL_DEEP_MAX = 1.0f;
constexpr float FL_DEEP_MIN = 0.2f;


//**********************************************************************************
// Utility function: ceil_to_uint
// Description: Rounds a float up to the next unsigned integer
//**********************************************************************************
constexpr uint32_t ceil_to_uint(float value) {
    return static_cast<uint32_t>(value + 0.999f);
}

// Compute delay buffer size based on sampling rate and max delay time
constexpr uint32_t DELAY_BUFFER_SIZE = 2000;

// Allocate modulation delay buffers in external SDRAM
SDRAM_SECTION float __FlangerModulatorBufferLeft[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __FlangerModulatorBufferRight[DELAY_BUFFER_SIZE];

namespace DadEffect {

//**********************************************************************************
// Class: cFlanger
// Description: Implements chorus audio effect using multiple modulated delay lines
//**********************************************************************************

// =============================================================================
// PUBLIC METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cFlanger::onInitialize() {
    // Initialize effect identification
    m_pShortName = "Flanger";  // Short name identifier
    m_pLongName  = "Flanger";  // Long descriptive name
    m_ID = FLANGER_ID;         // Unique effect identifier

    // =============================================================================
    // DSP INITIALIZATION SECTION
    // =============================================================================

    // Initialize modulator delay lines for left and right channels
    m_ModulatorLeft.Initialize(SAMPLING_RATE, __FlangerModulatorBufferLeft, DELAY_BUFFER_SIZE,
                               FL_MODULATOR_FREQ_LEFT, FL_MODULATOR_PLICH_MIN, FL_MODULATOR_PLICH_MAX,
                               FL_MODULATOR_OFFSET_LEFT);
    m_ModulatorRight.Initialize(SAMPLING_RATE, __FlangerModulatorBufferRight, DELAY_BUFFER_SIZE,
                                FL_MODULATOR_FREQ_RIGHT, FL_MODULATOR_PLICH_MIN, FL_MODULATOR_PLICH_MAX,
                                FL_MODULATOR_OFFSET_RIGHT);

    // =============================================================================
    // PARAMETER INITIALIZATION SECTION
    // =============================================================================

    // Initialize effect depth parameter
    m_Deep.Init(FLANGER_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 0.8f, 40);

    // Initialize effect Feedback parameter
    m_Feedback.Init(FLANGER_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 0.8f, 41);

    // Initialize dry/wet mix parameter with callback for mix changes
    m_DryWetMix.Init(FLANGER_ID, 50.0f, 0.0f, 100.0f, 5.0f, 1.0f, MixChange, (uint32_t) this, 3.0f, 42);

    // =============================================================================
    // VIEW SETUP SECTION
    // =============================================================================

    // Initialize parameter views for user interface
    m_DeepView.Init(&m_Deep, "Deep", "Deep", "%", "%");
    m_FeedBackView.Init(&m_Feedback, "Feedback", "Feedback", "%", "%");
    m_DryWetMixView.Init(&m_DryWetMix, "Mix", "Mix", "%", "%");

    // =============================================================================
    // MENU GROUPING SECTION
    // =============================================================================

    // Group parameters into panels for menu organization
    m_PanelFlanger.Init(&m_DeepView, &m_FeedBackView, &m_DryWetMixView);

    // =============================================================================
    // MAIN MENU CONFIGURATION SECTION
    // =============================================================================

    // Add panels to main menu
    m_Menu.addMenuItem(&m_PanelFlanger, "Flanger");

}

// ---------------------------------------------------------------------------------
// Method: onActivate
// Description: Called when effect becomes active
// ---------------------------------------------------------------------------------
void cFlanger::onActivate() {
    // Set initial dry/wet mix when effect is activated
    __DryWet.setMix(m_DryWetMix.getValue());
}

// ---------------------------------------------------------------------------------
// Method: onDesactivate
// Description: Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cFlanger::onDesactivate() {
    // Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Method: Process
// Description: Audio processing method - applies chorus effect to input buffer
// ---------------------------------------------------------------------------------
void cFlanger::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) {

    // Declare processing variables
    float OutLeft;   	// Single chorus output left channel
    float OutRight;  	// Single chorus output right channel

    // Get current effect parameters
    float Deep = FL_DEEP_MIN + ((FL_DEEP_MAX - FL_DEEP_MIN) * m_Deep.getNormalizedValue());  		// Modulation depth
    float Feedback = FL_FEEDBACK_MIN + ((FL_FEEDBACK_MAX - FL_FEEDBACK_MIN) * m_Feedback.getNormalizedValue());	// Feedback depth
    float WetGain = __DryWet.getGainWet(); 				// Wet signal gain

    // Process single chorus mode (first modulator only)
    OutLeft = m_ModulatorLeft.Process(pIn->Left, Deep, 1, Feedback, true);
    OutRight = m_ModulatorRight.Process(pIn->Right, Deep, 1, Feedback, false);

    // Apply wet gain to output signals
    pOut->Left = OutLeft * WetGain;
    pOut->Right = OutRight * WetGain;
}

// =============================================================================
// CALLBACK METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cFlanger::MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    // Update dry/wet mix with current parameter value
    __DryWet.setMix(pParameter->getValue());
}

} // namespace DadEffect
#endif
//***End of file**************************************************************
