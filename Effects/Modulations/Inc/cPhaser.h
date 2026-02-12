//==================================================================================
//==================================================================================
// File: cPhaser.h
// Description: Phaser effect class declaration
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "MultiModeEffect.h"
#include <array>
#include "cDCO.h"
#include "cAllPass.h"

namespace DadEffect {

//**********************************************************************************
// Class: cPhaser
// Description: Implements phaser audio effect
//**********************************************************************************

constexpr uint32_t      PHASER_ID              = BUILD_ID('P', 'H', 'A', 'S');  // Phaser effect ID
constexpr std::size_t   NB_MAX_FILTERS         = 6;                             // Maximum number of filters per channel
constexpr std::size_t   NB_MAX_TOTAL_FILTERS   = NB_MAX_FILTERS * 2;            // Total filters (both channels)
constexpr uint8_t       NB_PH_MODE             = 6;                             // Number of phaser modes

//**********************************************************************************
// Structure: ModeParam
// Description: Parameters for each phaser mode
//**********************************************************************************
struct ModeParam {
    uint8_t m_NbFilter;     // Number of filters to use
    uint8_t m_APFOrder;     // All-pass filter order (1 or 2)
    float   m_SpreadMid;    // Mid frequency spread factor
    float   m_SpreadDelta;  // Frequency spread delta factor
    float   m_FreqMid;      // Center frequency
    float   m_FreqDelta;    // Frequency modulation range
};

//**********************************************************************************
// Function: MakeModeParam
// Description: Creates a ModeParam structure with calculated frequency values
//**********************************************************************************
constexpr ModeParam MakeModeParam(uint8_t nbFilter, uint8_t apfOrder,
                                  float spreadMid, float spreadDelta,
                                  float fMin, float fMax) {
    return {nbFilter, apfOrder,
            spreadMid, spreadDelta,
            (fMin + fMax) * 0.5f, (fMax - fMin) * 0.5f};
}

//**********************************************************************************
// Class: cPhaser
// Description: Implements phaser audio effect
//**********************************************************************************
class cPhaser : public cMultiModeEffectBase {
public:
    // -----------------------------------------------------------------------------
    // Method: onInitialize
    // Description: Initializes effect parameters and configuration
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // Method: onActivate
    // Description: Called when effect becomes active
    // -----------------------------------------------------------------------------
    void onActivate() override;

    // -----------------------------------------------------------------------------
    // Method: onDesactivate
    // Description: Called when effect becomes inactive
    // -----------------------------------------------------------------------------
    void onDesactivate() override;

    // -----------------------------------------------------------------------------
    // Method: Process
    // Description: Audio processing method - applies effect to input buffer
    // -----------------------------------------------------------------------------
    void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) override;

protected:
    // -----------------------------------------------------------------------------
    // Method: MixChange (Callback)
    // Description: Updates dry/wet mix parameter when changed by user
    // -----------------------------------------------------------------------------
    static void MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // Method: SpeedChange (Callback)
    // Description: Updates LFO speed parameter when changed by user
    // -----------------------------------------------------------------------------
    static void SpeedChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // Method: ModeChange (Callback)
    // Description: Handles mode change parameter updates
    // -----------------------------------------------------------------------------
    static void ModeChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // Method: DeepChange (Callback)
    // Description: Handles depth parameter updates
    // -----------------------------------------------------------------------------
    static void DeepChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // Method: setFilterFreq
    // Description: Sets frequency for a specific filter based on LFO and mode params
    // -----------------------------------------------------------------------------
    inline void setFilterFreq(uint8_t Numfilter) {
        // Determine channel and filter index
        const bool isLeftChannel = (m_CtMaJFilter < NB_MAX_FILTERS);
        const int FilterIndex = isLeftChannel ? Numfilter : (Numfilter - NB_MAX_FILTERS);

        // Get LFO value for appropriate channel
        float LFOValue = isLeftChannel ? m_LeftLFO.getSymetricalSineValue() : m_RightLFO.getSymetricalSineValue();

        // Calculate frequency offset based on filter position and spread parameters
        const float StageOffsetMid = 1.0f + (m_ModeParams[m_ActiveMode].m_SpreadMid * FilterIndex);
        const float StageOffsetDelta = 1.0f + (m_ModeParams[m_ActiveMode].m_SpreadDelta * FilterIndex);

        // Calculate final filter frequency with LFO modulation
        const float FilterFreq = (m_ModeParams[m_ActiveMode].m_FreqMid * StageOffsetMid) +
                                 (m_ModeParams[m_ActiveMode].m_FreqDelta * StageOffsetDelta * LFOValue * m_DeepValue);

        // Apply frequency to appropriate filter type based on order
        if (m_ModeParams[m_ActiveMode].m_APFOrder == 1) {
            m_AllPass[Numfilter].SetFrequency(FilterFreq);
        } else {
            m_AllPass2[Numfilter].SetFrequency(FilterFreq);
        }
    }

    // =============================================================================
    // USER INTERFACE COMPONENTS SECTION
    // =============================================================================
    DadGUI::cUIParameter m_Deep;                           // Effect depth parameter
    DadGUI::cUIParameter m_Speed;                          // LFO speed parameter
    DadGUI::cUIParameter m_DryWetMix;                      // Dry/wet mix parameter
    DadGUI::cUIParameter m_Feedback;                       // Feedback parameter
    DadGUI::cUIParameter m_Mode;                           // Mode selection parameter

    DadGUI::cParameterNumNormalView    m_DeepView;         // Depth parameter view
    DadGUI::cParameterNumNormalView    m_SpeedView;        // Speed parameter view
    DadGUI::cParameterNumNormalView    m_DryWetMixView;    // Dry/wet mix parameter view
    DadGUI::cParameterNumLeftRightView m_FeedbackView;     // Feedback parameter view
    DadGUI::cParameterDiscretView      m_ModeView;         // Mode parameter view

    DadGUI::cPanelOfParameterView      m_PanelPhaser;      // Main phaser parameters panel
    DadGUI::cPanelOfParameterView      m_PanelOptions;     // Options panel

    // =============================================================================
    // DSP COMPONENTS SECTION
    // =============================================================================
    DadDSP::cDCO m_LeftLFO;                                // Left channel LFO
    DadDSP::cDCO m_RightLFO;                               // Right channel LFO

    float m_LeftFeedback;                                  // Left channel feedback value
    float m_RightFeedback;                                 // Right channel feedback value
    float m_DeepValue;                                     // Current depth value
    uint8_t m_NewMode;                                     // Requested mode (pending change)
    uint8_t m_ActiveMode;                                  // Currently active mode
    float m_Fad;                                           // Fade factor for mode switching
    uint8_t m_SwitchMode;                                  // Mode switching state machine
    uint8_t m_CtMaJFilter;                                 // Filter update counter

    // Mode parameter definitions
    static constexpr std::array<ModeParam, NB_PH_MODE> m_ModeParams = {{
        // NbFilter, APFOrder, SpreadMid, SpreadDelta, FreqMin, FreqMax
        MakeModeParam(4, 1, 0.0f, 0.0f, 200.0f, 1000.0f),   // Mode 1
        MakeModeParam(6, 1, 0.0f, 0.0f, 800.0f, 3000.0f),   // Mode 2
        MakeModeParam(6, 1, 0.5f, 0.5f, 200.0f, 1500.0f),   // Mode 3
        MakeModeParam(4, 2, 0.0f, 0.0f, 200.0f, 1000.0f),   // Mode 4
        MakeModeParam(6, 2, 0.5f, 0.3f, 200.0f, 1500.0f),   // Mode 5
        MakeModeParam(6, 2, 0.0f, 0.0f, 300.0f, 5000.0f)    // Mode 6
    }};

    DadDSP::cAllPass     m_AllPass[NB_MAX_TOTAL_FILTERS];   // First-order all-pass filters
    DadDSP::sAPFState    m_APFState[NB_MAX_TOTAL_FILTERS];  // First-order filter states

    DadDSP::cAllPass2    m_AllPass2[NB_MAX_TOTAL_FILTERS];  // Second-order all-pass filters
    DadDSP::sAPF2State   m_APF2State[NB_MAX_TOTAL_FILTERS]; // Second-order filter states
};

} // namespace DadEffect

//***End of file**************************************************************
