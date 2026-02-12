//==================================================================================
//==================================================================================
// File: Reverb.cpp
// Description: Implementation of stereo Reverberation effect with Hadamard matrix
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================
#include "EffectsConfig.h"
#ifdef REVERB_EFFECT
#include "Reverb.h"
#include "HardwareAndCo.h"
#include <cmath>
#include "cFastSinLUT.h"

namespace DadEffect {

//**********************************************************************************
// Delay buffers and configuration
//**********************************************************************************
#define ALIGN_32 __attribute__((aligned(32)))

// -----------------------------------------------------------------------------
// Pre-Delay
RAM_D1 ALIGN_32 static float __PreDelayBufferL[PRE_DELAYS_BUFFER_SIZE+128];
RAM_D1 ALIGN_32 static float __PreDelayBufferR[PRE_DELAYS_BUFFER_SIZE+128];

// -----------------------------------------------------------------------------
// Early Delay
RAM_D1 ALIGN_32 static float __EarlyBufferL[NUM_EARLY_PER_CHANNEL][EARLY_DELAYS_BUFFER_SIZE+128];
RAM_D1 ALIGN_32 static float __EarlyBufferR[NUM_EARLY_PER_CHANNEL][EARLY_DELAYS_BUFFER_SIZE+128];

// Early reflections configuration /!\ max 60ms = 2880
static const uint32_t __EarlyDelaysL[NUM_EARLY_PER_CHANNEL] = {
    199, 353, 547, 769, 1019, 1361
};
static const uint32_t __EarlyDelaysR[NUM_EARLY_PER_CHANNEL] = {
    211, 367, 563, 787, 1049, 1399
};

static const float __EarlyGains[NUM_EARLY_PER_CHANNEL] = {
    0.85f, 0.75f, 0.65f, 0.5f, 0.35f, 0.2f
};

// -----------------------------------------------------------------------------
// Diffusion network
ALIGN_32 static float __AllpassBuffer[NUM_ALLPASS][ALLPASS_BUFFER_SIZE+128];

// Allpass delay lengths  /!\ max 20ms = 960
static const uint32_t __AllpassLengths[NUM_ALLPASS] = {
		149, 263, 389, 509, 641
};

static const float 	__AllpassCoeff[NUM_ALLPASS] = {
		0.64f, 0.66f, 0.68f, 0.65f, 0.62f
};

// -----------------------------------------------------------------------------
// FDN Feedback Delay Network
SDRAM_SECTION ALIGN_32 static float __FDM_DelayBuffer[FDM_NUM_DELAYS][FDM_BUFFER_SIZE+256];

// FDM delay lengths /!\ max 0.5s = 24000
static const float __BaseDelayLengths[FDM_NUM_DELAYS] = {
	1493.7f, 1787.3f, 2087.9f, 2383.1f,
	2683.7f, 2999.3f, 3217.7f, 3539.9f,
	3863.3f, 4177.1f, 4441.7f, 4787.9f,
	5087.3f, 5399.1f, 5701.7f, 6007.3f
};

// Stereo Panoramisation fixe
static const float pan_left[16] = {
	0.85f, 0.25f, 0.70f, 0.40f,
	0.60f, 0.15f, 0.90f, 0.50f,
	0.80f, 0.30f, 0.65f, 0.45f,
	0.75f, 0.20f, 0.95f, 0.35f
};

static const float pan_right[16] = {
	0.30f, 0.80f, 0.45f, 0.65f,
	0.25f, 0.85f, 0.55f, 0.95f,
	0.40f, 0.75f, 0.50f, 0.70f,
	0.35f, 0.90f, 0.20f, 0.80f
};

// Pre calculed constants
constexpr float INV_SQRT2 = 0.7071067811865475f;
constexpr float INV_SQRT16 = 0.25f;
constexpr float MyPI = 3.14159265358979323846f;
constexpr float ONE_OVER_SAMPLING_RATE = 1.0f / SAMPLING_RATE;

// Conservation d'énergie (standard)
constexpr float INPUT_GAIN = 1.0f / std::sqrt(static_cast<float>(FDM_NUM_DELAYS));

//**********************************************************************************
// Fast Math Helpers (Inlined)
//**********************************************************************************

// -----------------------------------------------------------------------------
// Optimized Hadamard Matrix (N=16) - Inlined
// -----------------------------------------------------------------------------
// Function: Fast Walsh-Hadamard Transform for 16 elements
// Algorithm: FFT-like implementation in 4 stages (log2(16) = 4)
// Complexity: O(N log N) instead of O(N²) for naive version
// Memory: Uses one temporary array of 16 floats
// Inlining: Eliminates function call overhead for performance
// -----------------------------------------------------------------------------

inline void FastHadamardMatrix16(float* data) {
    // Temporary array for intermediate calculations
    // Avoids data overwrite issues during butterfly operations
    float t1[16];

    // -------------------------------------------------------------------------
    // STAGE 1: Stride = 8 (split into two 8-element blocks)
    // -------------------------------------------------------------------------
    // Combine elements 8 positions apart (coarsest frequency separation)
    // Butterfly operation: (a+b) and (a-b)
    // This corresponds to applying H₂ across halves of the array
    // -------------------------------------------------------------------------
    for(int i=0; i<8; ++i) {
        float a = data[i];        // Element from first half
        float b = data[i+8];      // Corresponding element from second half
        t1[i]   = a + b;          // Sum → stored in first half of temporary
        t1[i+8] = a - b;          // Difference → stored in second half
    }

    // -------------------------------------------------------------------------
    // STAGE 2: Stride = 4 (split into four 4-element blocks)
    // -------------------------------------------------------------------------
    // Process the two halves generated in stage 1 separately
    // First half (indices 0-7): stride 4 combination
    // Second half (indices 8-15): stride 4 combination
    // This corresponds to applying H₂ within each 8-element block
    // -------------------------------------------------------------------------
    for(int i=0; i<4; ++i) {     // For each quarter (0..3)
        // Process first half (0-7)
        float a = t1[i];         // Element from first quarter
        float b = t1[i+4];       // Corresponding element from second quarter
        data[i] = a + b;         // Store result in data[0-3]
        data[i+4] = a - b;       // Store result in data[4-7]

        // Process second half (8-15)
        a = t1[i+8];             // Element from third quarter
        b = t1[i+12];            // Corresponding element from fourth quarter
        data[i+8] = a + b;       // Store result in data[8-11]
        data[i+12] = a - b;      // Store result in data[12-15]
    }

    // -------------------------------------------------------------------------
    // STAGE 3: Stride = 2 (split into eight 2-element blocks)
    // -------------------------------------------------------------------------
    // Process in blocks of 4 elements with internal stride 2
    // Combine pairs: (0,2), (1,3), (4,6), (5,7), etc.
    // This corresponds to applying H₂ within each 4-element block
    // -------------------------------------------------------------------------
    for(int i=0; i<16; i+=4) {   // Process blocks of 4: indices 0, 4, 8, 12
        // First pair of block (elements 0 and 2 of sub-block)
        float a = data[i];       // First element
        float b = data[i+2];     // Element at distance 2
        t1[i] = a + b;           // Store result at position i
        t1[i+2] = a - b;         // Store result at position i+2

        // Second pair of block (elements 1 and 3 of sub-block)
        a = data[i+1];           // Second element
        b = data[i+3];           // Element at distance 2
        t1[i+1] = a + b;         // Store result at position i+1
        t1[i+3] = a - b;         // Store result at position i+3
    }

	// -------------------------------------------------------------------------
	// STAGE 4: Stride = 1 (final neighbor combination)
	// -------------------------------------------------------------------------
	// Final stage: combine adjacent elements
	// Pairs: (0,1), (2,3), (4,5), ..., (14,15)
	// Note: The normalization factor 0.25 (1/√16) is NOT applied here
	// because it's handled separately in the m_Gains[i] calculation
	// within the updateDelayLengths() method
	// This corresponds to applying H₂ to each adjacent pair
	// -------------------------------------------------------------------------
	for(int i=0; i<16; i+=2) {   // Process adjacent pairs
	   float a = t1[i];         // First element of pair
	   float b = t1[i+1];       // Second element of pair
	   data[i]   = (a + b);     // Result at even position
	   data[i+1] = (a - b);     // Result at odd position
	}

    // -------------------------------------------------------------------------
    // Final result: data contains the Walsh-Hadamard transform
    // The 16 input elements are replaced by 16 transform coefficients
    // -------------------------------------------------------------------------
}

//**********************************************************************************
// cReverb
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes DSP components and user interface parameters
// -----------------------------------------------------------------------------
void cReverb::onInitialize() {
    // =============================================================================
    // Initialize DSP Components

    // -----------------------------------------------------------------------------
	// Shimmer initialization
	m_PitchShifterUp.Initialize(SAMPLING_RATE);
	m_PitchShifterUp.SetBrightness(0.75f);
	m_PitchShifterUp.SetQuality(true);

	// High-pass filter ~600 Hz 24dB/oct
	m_ShimmerHPF.Initialize(SAMPLING_RATE, 600.f, 0.0f, 1.8f, DadDSP::FilterType::HPF24);
	m_ShimmerHPF.setCutoffFreq(600.0f);
	m_ShimmerHPF.CalculateParameters();

	m_ShimmerDeep = 0.0f;

    // -----------------------------------------------------------------------------
	// Pre-delay initialization
    m_PreDelayLineL.Initialize(__PreDelayBufferL, PRE_DELAYS_BUFFER_SIZE);
    m_PreDelayLineL.Clear();
    m_PreDelayLineR.Initialize(__PreDelayBufferR, PRE_DELAYS_BUFFER_SIZE);
    m_PreDelayLineR.Clear();
    m_PreDelayLength = 0;

    // -----------------------------------------------------------------------------
    // Initialize early reflections
	m_EarlyFinalGain = 0;
    for(int i = 0; i < NUM_EARLY_PER_CHANNEL; i++) {
        m_EarlyReflectionsL[i].Initialize(__EarlyBufferL[i], EARLY_DELAYS_BUFFER_SIZE);
        m_EarlyReflectionsL[i].Clear();
        m_EarlyReflectionsR[i].Initialize(__EarlyBufferR[i], EARLY_DELAYS_BUFFER_SIZE);
        m_EarlyReflectionsR[i].Clear();
        m_EarlyFinalGain += __EarlyGains[i];
    }
    m_EarlyFinalGain = 1.0f / m_EarlyFinalGain;

    // -----------------------------------------------------------------------------
    // Initialize allpass diffusion network (mono)
    for(int i = 0; i < NUM_ALLPASS; i++) {
        m_AllpassLine[i].Initialize(__AllpassBuffer[i], ALLPASS_BUFFER_SIZE);
        m_AllpassLine[i].Clear();
    }

    // -----------------------------------------------------------------------------
    // Initialize main FDN delay lines (mono)
    for(int i = 0; i < FDM_NUM_DELAYS; i++) {
        m_DelayLine[i].Initialize(__FDM_DelayBuffer[i], FDM_BUFFER_SIZE);
        m_DelayLine[i].Clear();

        // Initialize modulation with different phases and rates
        // f = 0.3 to 1.05 Hz
        m_FDM_ModLFO[i].Initialise(SAMPLING_RATE, 0.3f + (float)i * 0.05f, (float)i/(float)FDM_NUM_DELAYS);
        m_DampingFilterStates[i].Reset();
    }

    m_DampingFilter.Initialize(SAMPLING_RATE, DAMPING_CUTOFF_INIT, 0.0f, DAMPING_Q, DadDSP::FilterType::LPF24);
    m_DampingLFO.Initialise(SAMPLING_RATE, 0.55f, 0.0f);
    m_DampingLFO2.Initialise(SAMPLING_RATE, 0.25f, 0.0f);
    m_DampingCutoff = DAMPING_CUTOFF_INIT;
    m_MemLFO_Value = 0.0f;
    m_DampingLFO_Depth = 0.50f;

    m_SizeMultiplier = 1.0f;
    m_rt60 = 2.0f;
    updateDelayLengths();

    // -----------------------------------------------------------------------------
    // Initialize tone filters - STEREO
    m_BassFilterL.Initialize(SAMPLING_RATE, 400.0f, 0.0f, 1.0f, DadDSP::FilterType::LSH);
    m_BassFilterR.Initialize(SAMPLING_RATE, 400.0f, 0.0f, 1.0f, DadDSP::FilterType::LSH);
    m_TrebleFilterL.Initialize(SAMPLING_RATE, 4000.0f, 0.0f, 1.0f, DadDSP::FilterType::HSH);
    m_TrebleFilterR.Initialize(SAMPLING_RATE, 4000.0f, 0.0f, 1.0f, DadDSP::FilterType::HSH);

    // -----------------------------------------------------------------------------
    // Initialize state variables
    m_ModDepth = 5.0f;

	// =============================================================================
    // Initialize UI Parameters

    // Time parameter: controls reverb decay time (0.1s to 10s)
    //             Initial value, Min, Max ,   Rapid inc, Slow incr, CallBack,  Callback data,  SlopeTime,   MIDI CC
    m_Time.Init(REVERB_ID, 4.5f, 0.1f, 10.0f,  0.5f,      0.1f,      TimeChange,(uint32_t)this, 0.5f,        20);

    // Pre-delay parameter (in milliseconds)
    //                         Initial value, Min,  Max,    Rapid inc, Slow incr, CallBack,       Callback data,   SlopeTime, MIDI CC  RealTime
    m_PreDelay.Init(REVERB_ID, 0.0f,          0.0f, 100.0f, 10.0f,     1.0f,      PreDelayChange, (uint32_t)this, 0.3f,      21,       true);

    // Mix parameter: dry/wet balance (0% to 100%)
    //                    Initial value, Min,  Max ,   Rapid inc, Slow incr, CallBack,  Callback data,  SlopeTime, MIDI CC
    m_Mix.Init(REVERB_ID, 40.0f,         0.0f, 100.0f, 5.0f,      1.0f,      MixChange, (uint32_t)this, 1.0f,      22);

    // Modulation Depth parameter (chorus-like effect)
    //                              Initial value, Min,  Max , Rapid inc, Slow incr, CallBack,       Callback data,  SlopeTime, MIDI CC
    m_ModDepthParam.Init(REVERB_ID, 25.0f,         0.0f, 100,  2.0f,      0.5f,      ModDepthChange, (uint32_t)this, 0.5f,      23);

    // Shimmer parameter (pitch-shifted reverb feedback)
    //                        Initial value, Min,  Max,   Rapid inc, Slow incr, CallBack,      Callback data,  SlopeTime, MIDI CC
    m_Shimmer.Init(REVERB_ID, 5.0f,          0.0f, 100.f, 5.f,       1.f,       ShimmerChange, (uint32_t)this, 0.4f,      24);

    // Bass parameter (gain in dB for low shelf)
    //                     Initial value, Min,    Max ,  Rapid inc, Slow incr, CallBack,   Callback data,  SlopeTime, MIDI CC
    m_Bass.Init(REVERB_ID, 0.0f,          -12.0f, 12.0f, 1.0f,      0.5f,      BassChange, (uint32_t)this, 0.0f,      25);

    // Treble parameter (gain in dB for high shelf)
    //                       Initial value, Min,    Max ,  Rapid inc, Slow incr, CallBack,     Callback data,  SlopeTime, MIDI CC
    m_Treble.Init(REVERB_ID, 0.0f,          -12.0f, 12.0f, 1.0f,      0.5f,      TrebleChange, (uint32_t)this, 0.0f,      26);

    // Damping parameter (controls high frequency decay)
    //                        Initial value, Min,  Max ,   Rapid inc, Slow incr, CallBack,      Callback data,  SlopeTime, MIDI CC
    m_Damping.Init(REVERB_ID, 50.0f,         0.0f, 100.0f, 5.0f,      1.0f,      DampingChange, (uint32_t)this, 0.3f,      27);

    // Damping mod parameter (controls modulation to damping frequency)
    //                        Initial value, Min,  Max ,   Rapid inc, Slow incr, CallBack,      Callback data,        SlopeTime, MIDI CC
    m_DampingMod.Init(REVERB_ID, 50.0f,         0.0f, 100.0f, 5.0f,      1.0f,      DampingModChange, (uint32_t)this, 0.3f,      28);

    // Size parameter (room size multiplier)
    //             Initial value, Min, Max ,   Rapid inc, Slow incr, CallBack,  Callback data,   SlopeTime, MIDI CC, RT process
    m_Size.Init(REVERB_ID, 65.0f, 0,   100.0f, 5.0f,      1.0f,      SizeChange, (uint32_t)this, 0.0f,      29);


    // Parameter view initialization
    m_TimeView.Init(&m_Time, "Time", "Time", "S", "Second");
    m_PreDelayView.Init(&m_PreDelay, "PreDly", "Pre-Delay", "ms", "millisec");
    m_MixView.Init(&m_Mix, "Mix", "Mix", "%", "%");

    m_ModDepthView.Init(&m_ModDepthParam, "Mod", "Modulation Deep", "%", "%");
    m_ShimmerView.Init(&m_Shimmer, "Shimmer", "Shimmer Deep", "%", "%");

    m_BassView.Init(&m_Bass, "Bass", "Bass", "dB", "Decibel");
    m_TrebleView.Init(&m_Treble, "Treble", "Treble", "dB", "Decibel");

    m_DampingView.Init(&m_Damping, "Damp", "Damping", "%", "%");
    m_DampingModView.Init(&m_DampingMod, "DampMod", "Damping modulation", "%", "%");
    m_SizeView.Init(&m_Size, "Size", "Size", "%", "%");

    // Panel Initialization
    m_ParameterMainPanel.Init(&m_TimeView, &m_PreDelayView, &m_MixView);
    m_ParameterEffectPanel.Init(&m_ModDepthView, nullptr, &m_ShimmerView);
    m_ParameterTonePanel.Init(&m_BassView, nullptr, &m_TrebleView);
    m_ParameterAdvancedPanel.Init(&m_DampingView, &m_DampingModView, &m_SizeView);

    // Effect menu part initialization
    m_Menu.addMenuItem(&m_ParameterMainPanel, "Reverb");
    m_Menu.addMenuItem(&m_ParameterEffectPanel, "Effects");
    m_Menu.addMenuItem(&m_ParameterTonePanel, "Tone");
    m_Menu.addMenuItem(&m_ParameterAdvancedPanel, "Advanced");
}

// -----------------------------------------------------------------------------
// Update delay lengths based on size parameter
// -----------------------------------------------------------------------------
void cReverb::updateDelayLengths() {
    for(int i = 0; i < FDM_NUM_DELAYS; i++) {

    	m_CurrentDelayLengths[i] =
        		static_cast<uint32_t>(__BaseDelayLengths[i] * m_SizeMultiplier);
        // Clamp to buffer size
        if(m_CurrentDelayLengths[i] >= FDM_BUFFER_SIZE_NO_MOD) {
            m_CurrentDelayLengths[i] = FDM_BUFFER_SIZE_NO_MOD - 1;
        }

		float delaySec = static_cast<float>(m_CurrentDelayLengths[i]) * ONE_OVER_SAMPLING_RATE;
		m_Gains[i] = std::pow(10.0f, -3.0f * delaySec / (m_rt60 * 0.85f)) * 0.25f;
    }
}

// -----------------------------------------------------------------------------
// Audio processing function - processes one input/output audio buffer
// STEREO: Early reflections stereo + Late reverb mono
// -----------------------------------------------------------------------------
void cReverb::onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) {
    float inL = pIn->Left;
    float inR = pIn->Right;

    // ─────────────────────────────────────────────────────────────────────────────
    // 1. Pre-delay (stereo)
    m_PreDelayLineL.Push(inL);
    m_PreDelayLineR.Push(inR);
    float preDelayedL = m_PreDelayLineL.Pull(m_PreDelayLength);
    float preDelayedR = m_PreDelayLineR.Pull(m_PreDelayLength);

    // ─────────────────────────────────────────────────────────────────────────────
    // 2. Early reflections (stereo - separate for each channel)
    float EarlyL = 0.0f;
    float EarlyR = 0.0f;
    for(int i = 0; i < NUM_EARLY_PER_CHANNEL; i++) {
        m_EarlyReflectionsL[i].Push(preDelayedL);
        EarlyL += m_EarlyReflectionsL[i].Pull(__EarlyDelaysL[i]) * __EarlyGains[i];
        m_EarlyReflectionsR[i].Push(preDelayedR);
        EarlyR += m_EarlyReflectionsR[i].Pull(__EarlyDelaysR[i]) * __EarlyGains[i];
    }
    EarlyL *= m_EarlyFinalGain;
    EarlyR *= m_EarlyFinalGain;

    // ─────────────────────────────────────────────────────────────────────────────
    // 3. Diffusion through allpass cascade
    float diffused = (preDelayedL + preDelayedR) * INV_SQRT2;
    for(int i = 0; i < NUM_ALLPASS; i++) {
        float delayed = m_AllpassLine[i].Pull(__AllpassLengths[i]);
        m_AllpassLine[i].Push( diffused + __AllpassCoeff[i] * delayed);
        diffused = -diffused + delayed;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // 4. Read delay outputs with modulation
    float delayOuts[FDM_NUM_DELAYS];

    for (int i = 0; i < FDM_NUM_DELAYS; i++) {
        // Calculate modulated delay length
        float modDelayLength = m_CurrentDelayLengths[i] + (m_ModDepth * m_FDM_ModLFO[i].processFast());
        delayOuts[i] = m_DelayLine[i].Pull(modDelayLength);
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // 5. Apply moduled lowpass damping in feedback loop

    // Damping modulation
    float LFO_Value = (m_DampingLFO2.processFast() + m_DampingLFO.processFast()) * 0.5;

    if(LFO_Value != m_MemLFO_Value){
    	m_MemLFO_Value = LFO_Value;
    	float ratio = powf(2.0f, LFO_Value * m_DampingLFO_Depth);
      	float targetCutoffMod = m_DampingCutoff * ratio;

    	// Clamp
    	if(targetCutoffMod < DAMPING_CUTOFF_LOW){ targetCutoffMod = DAMPING_CUTOFF_LOW;}
    	else if (targetCutoffMod > DAMPING_CUTOFF_HIGHT){ targetCutoffMod = DAMPING_CUTOFF_HIGHT;}

    	m_DampingFilter.setCutoffFreq(targetCutoffMod);
    	m_DampingFilter.CalculateParameters();
    }

    for (uint16_t i = 0; i < FDM_NUM_DELAYS; i++) {
    	delayOuts[i] = m_DampingFilter.Process(delayOuts[i], m_DampingFilterStates[i]);
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // 6. Apply per-delay gains for decay (based on RT60)
    for (int i = 0; i < FDM_NUM_DELAYS; i++) {
        delayOuts[i] *= m_Gains[i];
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // 7. Compute feedback using Hadamard mix
    FastHadamardMatrix16(delayOuts);

    // ─────────────────────────────────────────────────────────────────────────────
    // 8 Compute Shimmer

    // Somme normalisée des sorties des delays
    float lateSum = 0.0f;
    for (int i = 0; i < FDM_NUM_DELAYS; i++) {
        lateSum += delayOuts[i];
    }
    lateSum *= INV_SQRT16;   // normalisation √(1/16)

    // Pitch shift +1 octave
    float shimmerShifted = m_PitchShifterUp.Process(lateSum);

    // Filtre passe-haut
    shimmerShifted = m_ShimmerHPF.Process(shimmerShifted);

    // ──────────────────────────────────────────────────────────────
    // 9. Compute and mix feedback

    // Add diffused input (Stage 3) to feedback (normalized injection)
    for (int i = 0; i < FDM_NUM_DELAYS; i++) {
    	float feedbackSample = delayOuts[i] + (diffused * INPUT_GAIN) + (shimmerShifted * INPUT_GAIN * m_ShimmerDeep);
    	m_DelayLine[i].Push(feedbackSample);
    }

    // ──────────────────────────────────────────────────────────────
    // 9. Compute stereo reverb output

    float lateL = 0.0f;
    float lateR = 0.0f;

    for (int i = 0; i < FDM_NUM_DELAYS; i++) {
        lateL += delayOuts[i] * pan_left[i];
        lateR += delayOuts[i] * pan_right[i];
    }

    constexpr float FINAL_GAIN = 0.080f;
    lateL *= FINAL_GAIN;
    lateR *= FINAL_GAIN;

    // ──────────────────────────────────────────────────────────────
    // 10. Mix early + late reverb stereo
    float reverbL = EarlyL * 0.3f + lateL * 0.7f;
    float reverbR = EarlyR * 0.3f + lateR * 0.7f;

    // ──────────────────────────────────────────────────────────────
    // 11. Apply tone filters (stereo)
    float reverbProcessedL = m_TrebleFilterL.Process(
        m_BassFilterL.Process(reverbL, DadDSP::eChannel::Left),
        DadDSP::eChannel::Left
    );

    float reverbProcessedR = m_TrebleFilterR.Process(
        m_BassFilterR.Process(reverbR, DadDSP::eChannel::Right),
        DadDSP::eChannel::Right
    );

    // ──────────────────────────────────────────────────────────────
    // 12. Apply wet gain and output
    float WetGain = __DryWet.getGainWet();
    pOut->Left = reverbProcessedL * WetGain;
    pOut->Right = reverbProcessedR * WetGain;
}


// ---------------------------------------------------------------------------------
// Callback: TimeChange - Updates per-delay gains based on RT60
// ---------------------------------------------------------------------------------
void cReverb::TimeChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
    cReverb* pthis = (cReverb*)CallbackUserData;
    pthis->m_rt60 = pParameter->getValue();
    pthis->updateDelayLengths();
}

// ---------------------------------------------------------------------------------
// Callback: MixChange - Updates dry/wet mix parameters
// ---------------------------------------------------------------------------------
void cReverb::MixChange(DadDSP::cParameter* pParameter, uint32_t CallbackUserData) {
	const float exponent = 1.5f;
    __DryWet.setNormalizedMix(powf(pParameter->getNormalizedValue(), exponent));
	//__DryWet.setMix(pParameter->getValue());
}

// ---------------------------------------------------------------------------------
// Callback: BassChange - Updates bass filter gains (stereo)
// ---------------------------------------------------------------------------------
void cReverb::BassChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    float gain = pParameter->getValue();

    pthis->m_BassFilterL.setGainDb(gain);
    pthis->m_BassFilterL.CalculateParameters();

    pthis->m_BassFilterR.setGainDb(gain);
    pthis->m_BassFilterR.CalculateParameters();
}

// ---------------------------------------------------------------------------------
// Callback: TrebleChange - Updates treble filter gains (stereo)
// ---------------------------------------------------------------------------------
void cReverb::TrebleChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    float gain = pParameter->getValue();

    pthis->m_TrebleFilterL.setGainDb(gain);
    pthis->m_TrebleFilterL.CalculateParameters();

    pthis->m_TrebleFilterR.setGainDb(gain);
    pthis->m_TrebleFilterR.CalculateParameters();
}

// ---------------------------------------------------------------------------------
// Callback: DampingChange - Updates damping coefficient
// ---------------------------------------------------------------------------------
void cReverb::DampingChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;

   // Interpolation exponentielle de la fréquence de coupure
   float cutoffFreq = DAMPING_CUTOFF_LOW * std::pow(DAMPING_COEF, pParameter->getValue() * 0.01);
   pthis->m_DampingCutoff = cutoffFreq;
   pthis->m_DampingFilter.setCutoffFreq(cutoffFreq);
   pthis->m_DampingFilter.CalculateParameters();
}

// ---------------------------------------------------------------------------------
// Callback: DampingChange - Updates damping coefficient
// ---------------------------------------------------------------------------------
void cReverb::DampingModChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    pthis->m_DampingLFO_Depth = pParameter->getValue() * 0.01;
}

// ---------------------------------------------------------------------------------
// Callback: PreDelayChange - Updates pre-delay time
// ---------------------------------------------------------------------------------
void cReverb::PreDelayChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    float preDelay = pParameter->getValue() * 0.001f;

    // Convert milliseconds to samples
    pthis->m_PreDelayLength = static_cast<uint32_t>(preDelay * SAMPLING_RATE);

    // Clamp to buffer size
    if(pthis->m_PreDelayLength >= PRE_DELAYS_BUFFER_SIZE) {
        pthis->m_PreDelayLength = PRE_DELAYS_BUFFER_SIZE - 1;
    }
}

// ---------------------------------------------------------------------------------
// Callback: SizeChange - Updates room size multiplier
// ---------------------------------------------------------------------------------
void cReverb::SizeChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    pthis->m_SizeMultiplier = FDM_MIN_LEN_MULTIPLIER + (FDM_MAX_LEN_MULTIPLIER * pParameter->getValue() * 0.01);
    pthis->updateDelayLengths();
}

// ---------------------------------------------------------------------------------
// Callback: ModDepthChange - Updates modulation depth
// ---------------------------------------------------------------------------------
void cReverb::ModDepthChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    pthis->m_ModDepth = FDM_MOD_MAX_SAMPLES * pParameter->getValue() * 0.01f;
}

// ---------------------------------------------------------------------------------
// Callback: ShimmerChange - Updates Shimmer depth
// ---------------------------------------------------------------------------------
void cReverb::ShimmerChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cReverb *pthis = (cReverb *)CallbackUserData;
    pthis->m_ShimmerDeep = pParameter->getValue() * 0.6;
}


} // namespace DadEffect
#endif
//***End of file**************************************************************
