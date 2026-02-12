//==================================================================================
//==================================================================================
// File: Reverb.h
// Description: Declaration of stereo Reverberation with Hadamard matrix
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cEffectBase.h"
#include "cDCO.h"
#include "BiquadFilter.h"
#include "cDelayLine.h"
#include "cFastLFO.h"
#include "cPitchShifter.h"

namespace DadEffect {

//**********************************************************************************
// cReverb - Enhanced Configuration
//

// Pre-Delay
constexpr float				TIME_MAX_PRE_DELAYS = 0.100f; // 100ms Pre-delay max
constexpr uint16_t			PRE_DELAYS_BUFFER_SIZE = static_cast<uint32_t>(TIME_MAX_PRE_DELAYS * SAMPLING_RATE);

// Early Delay
constexpr uint16_t			NUM_EARLY_PER_CHANNEL = 6;      // 6 per channel
constexpr float				TIME_MAX_EARLY_DELAYS = 0.060f; // 60ms Early delay max
constexpr uint16_t			EARLY_DELAYS_BUFFER_SIZE = static_cast<uint32_t>(TIME_MAX_EARLY_DELAYS * SAMPLING_RATE);

// Diffusion network (allpass filters)
constexpr uint16_t			NUM_ALLPASS = 5;
constexpr float				TIME_MAX_ALLPASS = 0.020f; // 20ms All pass delay max
constexpr uint16_t			ALLPASS_BUFFER_SIZE = static_cast<uint32_t>(TIME_MAX_ALLPASS * SAMPLING_RATE);

// Damping Biquad
constexpr float    			DAMPING_CUTOFF_HIGHT = 6000.0f;
constexpr float 			DAMPING_CUTOFF_LOW   = 500.0f;
constexpr float				DAMPING_COEF		 = DAMPING_CUTOFF_HIGHT / DAMPING_CUTOFF_LOW;
constexpr float 			DAMPING_CUTOFF_INIT  = 4000.0f;
constexpr float             DAMPING_Q            = 2.0f;

// FDN Feedback Delay Network
constexpr uint16_t			FDM_MOD_MAX_SAMPLES = 80;
constexpr uint16_t			FDM_NUM_DELAYS  = 16;
constexpr float				FDM_MAX_DELAY_S = 0.5f;
constexpr float 			FDM_MIN_LEN_MULTIPLIER = 0.6f;
constexpr float 			FDM_MAX_LEN_MULTIPLIER = 3.0f;
constexpr uint32_t 			FDM_BUFFER_SIZE = static_cast<uint32_t>(FDM_MOD_MAX_SAMPLES + (FDM_MAX_LEN_MULTIPLIER * FDM_MAX_DELAY_S * SAMPLING_RATE));
constexpr uint32_t 			FDM_BUFFER_SIZE_NO_MOD = static_cast<uint32_t>(FDM_MAX_LEN_MULTIPLIER * FDM_MAX_DELAY_S * SAMPLING_RATE);


constexpr uint32_t			REVERB_ID = BUILD_ID('R', 'E', 'V', 'B');

//**********************************************************************************
// class cReverb
//**********************************************************************************
class cReverb: public cEffectBase{
public:
    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor - initializes nothing by itself
    // -----------------------------------------------------------------------------
    cReverb() = default;

    // -----------------------------------------------------------------------------
    // Initializes DSP components and user interface parameters
    // -----------------------------------------------------------------------------
    void onInitialize() override;

    // -----------------------------------------------------------------------------
    // Returns the unique effect identifier
    // -----------------------------------------------------------------------------
    uint32_t getEffectID() override{
    	return REVERB_ID;
    }

    // -----------------------------------------------------------------------------
    // Audio processing function - processes one input/output audio buffer
    // -----------------------------------------------------------------------------
    void onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) override;

protected:

    // -----------------------------------------------------------------------------
    // Static callbacks (UI parameter change handlers)
    // -----------------------------------------------------------------------------
    static void TimeChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void BassChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void TrebleChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void DampingChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void DampingModChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void PreDelayChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void SizeChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void WidthChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void ModDepthChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);
    static void ShimmerChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // DSP Helper Functions
    // -----------------------------------------------------------------------------
    void updateDelayLengths();

    // =============================================================================
    // Protected Member Variables
    // =============================================================================

     // =============================================================================
    // User Interface Components
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Parameter declarations
    DadGUI::cUIParameter                 m_Time;
    DadGUI::cUIParameter                 m_PreDelay;
    DadGUI::cUIParameter                 m_Mix;

    DadGUI::cUIParameter                 m_ModDepthParam;
    DadGUI::cUIParameter 				 m_Shimmer;

    DadGUI::cUIParameter                 m_Bass;
    DadGUI::cUIParameter                 m_Treble;

    DadGUI::cUIParameter                 m_Damping;
    DadGUI::cUIParameter                 m_DampingMod;
    DadGUI::cUIParameter                 m_Size;

    // -----------------------------------------------------------------------------
    // Parameter view declarations
    DadGUI::cParameterNumNormalView		m_TimeView;
    DadGUI::cParameterNumNormalView     m_PreDelayView;
    DadGUI::cParameterNumNormalView     m_MixView;

    DadGUI::cParameterNumNormalView     m_ModDepthView;
    DadGUI::cParameterNumNormalView		m_ShimmerView;

    DadGUI::cParameterNumLeftRightView  m_BassView;
    DadGUI::cParameterNumLeftRightView  m_TrebleView;

    DadGUI::cParameterNumNormalView     m_DampingView;
    DadGUI::cParameterNumNormalView     m_DampingModView;
    DadGUI::cParameterNumNormalView     m_SizeView;

    // -----------------------------------------------------------------------------
    // Panel declarations
    DadGUI::cPanelOfParameterView       m_ParameterMainPanel;
    DadGUI::cPanelOfParameterView       m_ParameterEffectPanel;
    DadGUI::cPanelOfParameterView       m_ParameterTonePanel;
    DadGUI::cPanelOfParameterView       m_ParameterAdvancedPanel;

    // =============================================================================
    // DSP Components
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Pre-delay
    DadDSP::cDelayLine      m_PreDelayLineL;
    DadDSP::cDelayLine      m_PreDelayLineR;
    uint32_t 				m_PreDelayLength;

    // -----------------------------------------------------------------------------
    // Early reflections
    DadDSP::cDelayLine		m_EarlyReflectionsL[NUM_EARLY_PER_CHANNEL];
    DadDSP::cDelayLine      m_EarlyReflectionsR[NUM_EARLY_PER_CHANNEL];
    float 					m_EarlyFinalGain;

    // -----------------------------------------------------------------------------
    // Diffusion network (allpass filters)
    DadDSP::cDelayLine      m_AllpassLine[NUM_ALLPASS];
    float                   m_AllpassCoeff[NUM_ALLPASS];

    // -----------------------------------------------------------------------------
    // Main FDN delay network (mono late reverb)
    DadDSP::cDelayLine      m_DelayLine[FDM_NUM_DELAYS];
    uint32_t 				m_CurrentDelayLengths[FDM_NUM_DELAYS];
    float 					m_SizeMultiplier;

    // -----------------------------------------------------------------------------
    // FD Modulation
	float 					m_ModDepth;
    DadDSP::cFastLFO<2024>  m_FDM_ModLFO[FDM_NUM_DELAYS];

    // -----------------------------------------------------------------------------
    // Damping
    DadDSP::cBiQuad		    m_DampingFilter;
    DadDSP::sFilterState	m_DampingFilterStates[FDM_NUM_DELAYS];
    float 					m_DampingCutoff;

    DadDSP::cFastLFO<2024>   m_DampingLFO;
    DadDSP::cFastLFO<2024>   m_DampingLFO2;
    float					m_MemLFO_Value;
    float 					m_DampingLFO_Depth;

    // Tone shaping filters - STEREO
    DadDSP::cBiQuad			m_BassFilterL;
    DadDSP::cBiQuad			m_BassFilterR;
    DadDSP::cBiQuad			m_TrebleFilterL;
    DadDSP::cBiQuad			m_TrebleFilterR;

    // Per-delay gains for decay control
     float m_Gains[FDM_NUM_DELAYS];
     float m_rt60;

	 // Shimmer
	 DadDSP::cPitchShifter	m_PitchShifterUp;
	 DadDSP::cBiQuad        m_ShimmerHPF;
	 float                  m_ShimmerDeep;
};

}  // namespace DadEffect

//***End of file**************************************************************
