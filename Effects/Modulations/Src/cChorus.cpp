//==================================================================================
//==================================================================================
// File: cChorus.cpp
// Description: Chorus audio effect implementation
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cChorus.h"

// Modulator offset constants for different delay lines
constexpr float MODULATOR_OFFSET_LEFT_1  = 0.0002f;
constexpr float MODULATOR_OFFSET_RIGHT_1 = 0.001f;
constexpr float MODULATOR_OFFSET_LEFT_2  = 0.005f;
constexpr float MODULATOR_OFFSET_RIGHT_2 = 0.007f;
constexpr float MODULATOR_OFFSET_LEFT_3  = 0.01f;
constexpr float MODULATOR_OFFSET_RIGHT_3 = 0.02f;

//constexpr float MODULATOR_OFFSET_LEFT_1  = 0.00002f;
//constexpr float MODULATOR_OFFSET_RIGHT_1 = 0.00001f;
//constexpr float MODULATOR_OFFSET_LEFT_2  = 0.0005f;
//constexpr float MODULATOR_OFFSET_RIGHT_2 = 0.0007f;
//constexpr float MODULATOR_OFFSET_LEFT_3  = 0.001f;
//constexpr float MODULATOR_OFFSET_RIGHT_3 = 0.002f;

// Modulator frequency constants for different LFOs
constexpr float MODULATOR_FREQ_LEFT_1  = 2.0f;
constexpr float MODULATOR_FREQ_RIGHT_1 = 2.1f;
constexpr float MODULATOR_FREQ_LEFT_2  = 2.5f;
constexpr float MODULATOR_FREQ_RIGHT_2 = 2.6f;
constexpr float MODULATOR_FREQ_LEFT_3  = 4.0f;
constexpr float MODULATOR_FREQ_RIGHT_3 = 4.5f;

// Modulator pitch range limits
constexpr float MODULATOR_PLICH_MAX = 4.5f;
constexpr float MODULATOR_PLICH_MIN = 2.5f;

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
SDRAM_SECTION float __ChorusModulatorBuffer1LeftA[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __ChorusModulatorBuffer1RightA[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __ChorusModulatorBuffer2LeftA[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __ChorusModulatorBuffer2RightA[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __ChorusModulatorBuffer3LeftA[DELAY_BUFFER_SIZE];
SDRAM_SECTION float __ChorusModulatorBuffer3RightA[DELAY_BUFFER_SIZE];

namespace DadEffect {

//**********************************************************************************
// Class: cChorus
// Description: Implements chorus audio effect using multiple modulated delay lines
//**********************************************************************************

// =============================================================================
// PUBLIC METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cChorus::onInitialize() {
    // Initialize effect identification
    m_pShortName = "Chorus";  // Short name identifier
    m_pLongName  = "Chorus";  // Long descriptive name
    m_ID = CHORUS_ID;         // Unique effect identifier

    // =============================================================================
    // DSP INITIALIZATION SECTION
    // =============================================================================

    // Initialize crossfader for smooth transitions
    m_Fader.Initialize(SAMPLING_RATE, 0.5f);

    // Initialize modulator delay lines for left and right channels
    m_Modulator1Left.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer1LeftA, DELAY_BUFFER_SIZE,
                               MODULATOR_FREQ_LEFT_1, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                               MODULATOR_OFFSET_LEFT_1);
    m_Modulator1Right.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer1RightA, DELAY_BUFFER_SIZE,
                                MODULATOR_FREQ_RIGHT_1, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                                MODULATOR_OFFSET_RIGHT_1);
    m_Modulator2Left.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer2LeftA, DELAY_BUFFER_SIZE,
                               MODULATOR_FREQ_LEFT_2, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                               MODULATOR_OFFSET_LEFT_2);
    m_Modulator2Right.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer2RightA, DELAY_BUFFER_SIZE,
                                MODULATOR_FREQ_RIGHT_2, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                                MODULATOR_OFFSET_RIGHT_2);
    m_Modulator3Left.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer3LeftA, DELAY_BUFFER_SIZE,
                               MODULATOR_FREQ_LEFT_3, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                               MODULATOR_OFFSET_LEFT_3);
    m_Modulator3Right.Initialize(SAMPLING_RATE, __ChorusModulatorBuffer3RightA, DELAY_BUFFER_SIZE,
                                MODULATOR_FREQ_RIGHT_3, MODULATOR_PLICH_MIN, MODULATOR_PLICH_MAX,
                                MODULATOR_OFFSET_RIGHT_3);

    // =============================================================================
    // PARAMETER INITIALIZATION SECTION
    // =============================================================================

    // Initialize effect depth parameter
    m_Deep.Init(CHORUS_ID, 45.0f, 0.0f, 100, 5, 1, nullptr, 0, 0.8f * RT_RATE, 30);

    // Initialize mode parameter with callback for mode changes
    m_Mode.Init(CHORUS_ID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, ModeChange, (uint32_t) this, 0.0f, 31);

    // Initialize dry/wet mix parameter with callback for mix changes
    m_DryWetMix.Init(CHORUS_ID, 50.0f, 0.0f, 100.0f, 5, 1, MixChange, (uint32_t) this, 3.0f * RT_RATE, 32);

    // =============================================================================
    // VIEW SETUP SECTION
    // =============================================================================

    // Initialize parameter views for user interface
    m_DeepView.Init(&m_Deep, "Deep", "Deep", "%", "%");

    m_ModeView.Init(&m_Mode, "Mode", "Mode");
    m_ModeView.AddDiscreteValue("Single", "Single");
    m_ModeView.AddDiscreteValue("Triple", "Triple");

    m_DryWetMixView.Init(&m_DryWetMix, "Mix", "Mix", "%", "%");

    // =============================================================================
    // MENU GROUPING SECTION
    // =============================================================================

    // Group parameters into panels for menu organization
    m_PanelChorus.Init(&m_DeepView, &m_ModeView, &m_DryWetMixView);

    // =============================================================================
    // MAIN MENU CONFIGURATION SECTION
    // =============================================================================

    // Add panels to main menu
    m_Menu.addMenuItem(&m_PanelChorus, "Chorus");
}

// ---------------------------------------------------------------------------------
// Method: onActivate
// Description: Called when effect becomes active
// ---------------------------------------------------------------------------------
void cChorus::onActivate() {
    // Set initial dry/wet mix when effect is activated
    __DryWet.setMix(m_DryWetMix.getValue());
}

// ---------------------------------------------------------------------------------
// Method: onDesactivate
// Description: Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cChorus::onDesactivate() {
    // Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Method: Process
// Description: Audio processing method - applies chorus effect to input buffer
// ---------------------------------------------------------------------------------
void cChorus::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff) {

    // Declare processing variables
    float OutSingleLeft;   // Single chorus output left channel
    float OutSingleRight;  // Single chorus output right channel
    float OutTripleLeft;   // Triple chorus output left channel
    float OutTripleRight;  // Triple chorus output right channel
    float Out;             // Intermediate processing variable

    // Get current effect parameters
    float Deep = m_Deep.getNormalizedValue();     // Modulation depth
    float WetGain = __DryWet.getGainWet(); // Wet signal gain

    // Process single chorus mode (first modulator only)
    OutSingleLeft = m_Modulator1Left.Process(pIn->Left, Deep);
    OutSingleRight = m_Modulator1Right.Process(pIn->Right, Deep);

    // Process triple chorus mode (cascaded modulators)
    Out = m_Modulator2Left.Process(OutSingleLeft, Deep);
    OutTripleLeft = m_Modulator3Left.Process(Out, Deep);

    Out = m_Modulator2Right.Process(OutSingleRight, Deep);
    OutTripleRight = m_Modulator3Right.Process(Out, Deep);

    // Declare crossfade output variables
    float OutFadeLeft;   // Crossfaded output left channel
    float OutFadeRight;  // Crossfaded output right channel

    // Apply crossfade between single and triple chorus modes
    m_Fader.Process(OutSingleLeft, OutSingleRight, OutTripleLeft, OutTripleRight,
                   OutFadeLeft, OutFadeRight);

    // Apply wet gain to output signals
    pOut->Left = OutFadeLeft * WetGain;
    pOut->Right = OutFadeRight * WetGain;
}

// =============================================================================
// CALLBACK METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cChorus::MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    // Update dry/wet mix with current parameter value
    __DryWet.setMix(pParameter->getValue());
}

// ---------------------------------------------------------------------------------
// Method: ModeChange (Callback)
// Description: Handles mode changes between single and triple chorus
// ---------------------------------------------------------------------------------
void cChorus::ModeChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    // Get pointer to chorus instance
    cChorus *pthis = reinterpret_cast<cChorus *>(CallbackUserData);

    // Handle mode transition with crossfade
    if (0 == pParameter->getValue()) {
        // Switch to single chorus mode
        pthis->m_Fader.startFadeOutA();
    } else {
        // Switch to triple chorus mode
        pthis->m_Fader.startFadeInB();
    }
}

} // namespace DadEffect

//***End of file**************************************************************
