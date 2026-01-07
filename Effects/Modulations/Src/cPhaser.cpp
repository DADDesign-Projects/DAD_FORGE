//==================================================================================
//==================================================================================
// File: cPhaser.cpp
// Description: Phaser audio effect implementation
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cPhaser.h"
#include "math.h"

namespace DadEffect {

// =============================================================================
// CONSTANTS SECTION
// =============================================================================
constexpr float LFO_FREQ_MAX   = 4.0f;      // Maximum LFO frequency
constexpr float LFO_FREQ_MIN   = 0.1f;      // Minimum LFO frequency
constexpr float LFO_FREQ_INIT  = 0.5f;      // Initial LFO frequency
constexpr float LFO_OFFSET     = 1.0f;      // Right channel LFO offset factor
constexpr float FAD_STEP       = 1.0f / 10000; // Fade step for mode switching

// Mode parameters array definition
constexpr std::array<ModeParam, NB_PH_MODE> cPhaser::m_ModeParams;

//**********************************************************************************
// Class: cPhaser
// Description: Implements phaser audio effect
//**********************************************************************************

// =============================================================================
// PUBLIC METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cPhaser::onInitialize() {
    // Initialize effect identification
    m_pShortName = "Phaser";  // Short name identifier
    m_pLongName  = "Phaser";  // Long descriptive name
    m_ID = PHASER_ID;         // Unique effect identifier

    // =============================================================================
    // PARAMETER INITIALIZATION SECTION
    // =============================================================================

    // Initialize effect depth parameter
    m_Deep.Init(PHASER_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, DeepChange,
                (uint32_t)this, 0.5f * RT_RATE, 20);

    // Initialize effect speed parameter
    m_Speed.Init(PHASER_ID, LFO_FREQ_INIT, LFO_FREQ_MIN, LFO_FREQ_MAX, 0.1f, 0.05f,
                 SpeedChange, (uint32_t)this, 0.8f * RT_RATE, 21);

    // Initialize dry/wet mix parameter
    m_DryWetMix.Init(PHASER_ID, 38.0f, 0.0f, 100.0f, 5.0f, 1.0f, MixChange,
                     (uint32_t)this, 3.0f * RT_RATE, 22);

    // Initialize feedback parameter
    m_Feedback.Init(PHASER_ID, 15.0f, -100.0f, 100.0f, 5.0f, 1.0f,
                    nullptr, 0, 0.8f * RT_RATE, 23);

    // Initialize mode selection parameter
    m_Mode.Init(PHASER_ID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, ModeChange,
                (uint32_t)this, 0, 24);

    // =============================================================================
    // VIEW SETUP SECTION
    // =============================================================================

    // Initialize parameter views for user interface
    m_DeepView.Init(&m_Deep, "Deep", "Deep", "%", "%");
    m_SpeedView.Init(&m_Speed, "Speed", "LFO Frequency", "Hz", "Hz");
    m_DryWetMixView.Init(&m_DryWetMix, "Dry", "Dry", "%", "%");
    m_FeedbackView.Init(&m_Feedback, "Feedback", "Feedback", "%", "%");
    m_ModeView.Init(&m_Mode, "Mode", "Mode");

    // Add discrete mode values
    m_ModeView.AddDiscreteValue("1", "1");
    m_ModeView.AddDiscreteValue("2", "2");
    m_ModeView.AddDiscreteValue("3", "3");
    m_ModeView.AddDiscreteValue("4", "4");
    m_ModeView.AddDiscreteValue("5", "5");
    m_ModeView.AddDiscreteValue("6", "6");

    // =============================================================================
    // MENU GROUPING SECTION
    // =============================================================================

    // Group parameters into panels for menu organization
    m_PanelPhaser.Init(&m_DeepView, &m_SpeedView, &m_DryWetMixView);
    m_PanelOptions.Init(&m_FeedbackView, nullptr, &m_ModeView);

    // =============================================================================
    // MAIN MENU CONFIGURATION SECTION
    // =============================================================================

    // Add panels to main menu
    m_Menu.addMenuItem(&m_PanelPhaser, "Phaser");
    m_Menu.addMenuItem(&m_PanelOptions, "Options");

    // =============================================================================
    // DSP INITIALIZATION SECTION
    // =============================================================================

    // Initialize LFOs for both channels
    m_LeftLFO.Initialize(SAMPLING_RATE, 0.5f, LFO_FREQ_MIN, LFO_FREQ_MAX, 0.5f);
    m_RightLFO.Initialize(SAMPLING_RATE, 0.5f, LFO_FREQ_MIN, LFO_FREQ_MAX, 0.5f);
    m_LeftLFO.setFreq(LFO_FREQ_INIT);
    m_RightLFO.setFreq(LFO_FREQ_INIT * LFO_OFFSET);

    // Initialize all-pass filters for both channels
    for (std::size_t Index = 0; Index < NB_MAX_TOTAL_FILTERS; Index++) {
        m_AllPass[Index].Initialize(SAMPLING_RATE, &m_APFState[Index]);
        m_AllPass2[Index].Initialize(SAMPLING_RATE, &m_APF2State[Index]);
    }

    // Set initial filter frequencies
    for (std::size_t Index = 0; Index < NB_MAX_TOTAL_FILTERS; Index++) {
        setFilterFreq(Index);
    }

    // Set initial dry/wet mix
    __DryWet.setMix(50);

    // Initialize state variables
    m_LeftFeedback = 0.0f;
    m_RightFeedback = 0.0f;
    m_CtMaJFilter = 0;
    m_SwitchMode = 0;
    m_ActiveMode = 0;
    m_NewMode = 0;
    m_Fad = 1.0f;
    m_DeepValue = 0.0f;
}

// ---------------------------------------------------------------------------------
// Method: onActivate
// Description: Called when effect becomes active
// ---------------------------------------------------------------------------------
void cPhaser::onActivate() {
    // Reset filter states when effect is activated
    for (std::size_t Index = 0; Index < NB_MAX_FILTERS; Index++) {
        m_APFState[Index].Reset();
        m_APF2State[Index].Reset();
    }

    // Set initial dry/wet mix from parameter
    __DryWet.setMix(m_DryWetMix.getValue());
}

// ---------------------------------------------------------------------------------
// Method: onDesactivate
// Description: Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cPhaser::onDesactivate() {
    // Currently no deactivation logic required
}

// ---------------------------------------------------------------------------------
// Method: Process
// Description: Audio processing method - applies phaser effect to input buffer
// ---------------------------------------------------------------------------------
void cPhaser::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) {
    // Step 1: Update LFOs
    m_LeftLFO.Step();
    m_RightLFO.Step();

    // Step 2: Handle mode switching with fade
    switch (m_SwitchMode) {
    case 0:
        // Check if mode change is requested
        if (m_ActiveMode != m_NewMode) {
            m_SwitchMode = 1;  // Start fade out
        }
        break;

    case 1:
        // Fade out current mode
        if (m_Fad <= 0.0f) {
            m_ActiveMode = m_NewMode;  // Switch to new mode
            m_SwitchMode = 2;          // Start fade in
        } else {
            m_Fad -= FAD_STEP;         // Continue fading out
        }
        break;

    case 2:
        // Fade in new mode
        if (m_Fad >= 1.0f) {
            m_SwitchMode = 0;          // Fade complete
        } else {
            m_Fad += FAD_STEP;         // Continue fading in
        }
        break;
    }

    // Step 3: Initialize processing buffers
    float OutLeft = pIn->Left;
    float OutRight = pIn->Right;
    float OutLeftTemp = OutLeft;
    float OutRightTemp = OutRight;
    float OutLeftTemp2 = OutLeft;
    float OutRightTemp2 = OutRight;

    // Step 4: Apply all-pass filter cascade
    std::size_t NbFilter = m_ModeParams[m_ActiveMode].m_NbFilter - 1;
    for (std::size_t Index = 0; Index < NB_MAX_FILTERS; Index++) {
        std::size_t IndexRight = Index + NB_MAX_FILTERS;

        // Process through first-order filters
        OutLeftTemp = m_AllPass[Index].Process(OutLeftTemp);
        OutRightTemp = m_AllPass[IndexRight].Process(OutRightTemp);

        // Process through second-order filters
        OutLeftTemp2 = m_AllPass2[Index].Process(OutLeftTemp);
        OutRightTemp2 = m_AllPass2[IndexRight].Process(OutRightTemp);

        // Select output based on filter order and number of filters
        if (Index == NbFilter) {
            if (m_ModeParams[m_ActiveMode].m_APFOrder == 1) {
                OutLeft = OutLeftTemp;
                OutRight = OutRightTemp;
            } else {
                OutLeft = OutLeftTemp2;
                OutRight = OutRightTemp2;
            }
        }
    }

    // Step 5: Update one filter frequency per call (round-robin)
    setFilterFreq(m_CtMaJFilter);

    // Increment filter update counter with wrap-around
    if (++m_CtMaJFilter >= NB_MAX_TOTAL_FILTERS) {
        m_CtMaJFilter = 0;
    }

    // Step 6: Apply feedback
    float fb = m_Feedback.getNormalizedValue() * 0.6f;
    OutLeft += m_LeftFeedback * fb;
    OutRight += m_RightFeedback * fb;
    m_LeftFeedback = OutLeft;
    m_RightFeedback = OutRight;

    // Step 7: Apply wet gain and fade to output signals
    float WetGain = __DryWet.getGainWet()*0.4f;
    pOut->Left = OutLeft * WetGain * m_Fad;
    pOut->Right = OutRight * WetGain * m_Fad;
}

// =============================================================================
// CALLBACK METHODS SECTION
// =============================================================================

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cPhaser::MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    // Update dry/wet mix with current parameter value
    __DryWet.setMix(pParameter->getValue());
}

// ---------------------------------------------------------------------------------
// Method: SpeedChange (Callback)
// Description: Updates LFO speed parameter
// ---------------------------------------------------------------------------------
void cPhaser::SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    // Get pointer to phaser instance
    cPhaser* pthis = reinterpret_cast<cPhaser*>(CallbackUserData);

    // Update LFO frequencies with offset for right channel
    float Freq = pParameter->getValue();
    pthis->m_LeftLFO.setFreq(Freq);
    pthis->m_RightLFO.setFreq(Freq * LFO_OFFSET);
}

// ---------------------------------------------------------------------------------
// Method: ModeChange (Callback)
// Description: Handles mode change parameter updates
// ---------------------------------------------------------------------------------
void cPhaser::ModeChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    // Get pointer to phaser instance
    cPhaser* pthis = reinterpret_cast<cPhaser*>(CallbackUserData);

    // Store requested mode (will be applied with fade)
    pthis->m_NewMode = (uint8_t)pParameter->getValue();
}

// ---------------------------------------------------------------------------------
// Method: DeepChange (Callback)
// Description: Handles depth parameter updates
// ---------------------------------------------------------------------------------
void cPhaser::DeepChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    // Get pointer to phaser instance
    cPhaser* pthis = reinterpret_cast<cPhaser*>(CallbackUserData);

    // Apply nonlinear curve to depth parameter for smoother effect
    float Deep = pParameter->getValue() / 100.0f;
    pthis->m_DeepValue = 1.0f - ((1.0f - Deep) * (1.0f - Deep));
}

} // namespace DadEffect

//***End of file**************************************************************
