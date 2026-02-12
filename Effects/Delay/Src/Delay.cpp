//==================================================================================
//==================================================================================
// File: Delay.cpp
// Description: Implementation of stereo delay effect with modulation and tone control
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#include "EffectsConfig.h"
#ifdef DELAY_EFFECT
#include "Delay.h"

constexpr float DELAY_MAX_TIME = 1.5f;  // Maximum delay time in seconds
constexpr float DELAY_MIN_TIME = 0.1f;  // Minimum delay time in seconds

// Utility to round up to the next uint
constexpr uint32_t ceil_to_uint(float value) {
    return static_cast<uint32_t>(value + 0.999f);
}


constexpr float SMOOTH_COEFF = 0.00008f;

// Calculate buffer size based on sampling rate and max delay time
constexpr uint32_t DELAY_BUFFER_SIZE = ceil_to_uint(SAMPLING_RATE * DELAY_MAX_TIME);

// Allocate delay buffers in SDRAM (extra 100 samples for interpolation safety)
SDRAM_SECTION float __DelayBufferLeft[DELAY_BUFFER_SIZE + 100];   // Left channel delay buffer 1
SDRAM_SECTION float __DelayBufferRight[DELAY_BUFFER_SIZE + 100];  // Right channel delay buffer 1
SDRAM_SECTION float __Delay2BufferLeft[DELAY_BUFFER_SIZE + 100];  // Left channel delay buffer 2
SDRAM_SECTION float __Delay2BufferRight[DELAY_BUFFER_SIZE + 100]; // Right channel delay buffer 2

namespace DadEffect {

//**********************************************************************************
// cDelay
//
// Class responsible for managing delay effect parameters, processing audio,
// and handling user interface interaction.
// Implements a stereo delay effect with primary and secondary delay lines,
// feedback controls, LFO-based modulation, and tone shaping filters.
//**********************************************************************************

// -----------------------------------------------------------------------------
// Function: Initialize
// Description: Initializes parameters, UI, filters, buffers, and LFO
// -----------------------------------------------------------------------------
void cDelay::onInitialize() {
    // =============================================================================
    // Initialize DSP Components

    // -----------------------------------------------------------------------------
    // Initialize tone filters - STEREO
    m_BassFilter1.Initialize(SAMPLING_RATE, 400.0f, 0.0f, 1.0f, DadDSP::FilterType::LSH);
    m_BassFilter2.Initialize(SAMPLING_RATE, 400.0f, 0.0f, 1.0f, DadDSP::FilterType::LSH);
    m_TrebleFilter1.Initialize(SAMPLING_RATE, 1000.0f, 0.0f, 1.0f, DadDSP::FilterType::HSH);
    m_TrebleFilter2.Initialize(SAMPLING_RATE, 1000.0f, 0.0f, 1.0f, DadDSP::FilterType::HSH);

    // Initialize delay lines
    m_Delay1LineRight.Initialize(__DelayBufferRight, DELAY_BUFFER_SIZE);  // Right channel delay line 1
    m_Delay1LineRight.Clear();  // Clear buffer
    m_Delay1LineLeft.Initialize(__DelayBufferLeft, DELAY_BUFFER_SIZE);    // Left channel delay line 1
    m_Delay1LineLeft.Clear();   // Clear buffer

    m_Delay2LineRight.Initialize(__Delay2BufferRight, DELAY_BUFFER_SIZE); // Right channel delay line 2
    m_Delay2LineRight.Clear();  // Clear buffer
    m_Delay2LineLeft.Initialize(__Delay2BufferLeft, DELAY_BUFFER_SIZE);   // Left channel delay line 2
    m_Delay2LineLeft.Clear();   // Clear buffer

    m_SatDrive = 1.0f;
    m_PrevTime = 0.0f;
    m_PrevSubDelayR = 0.0f;
    m_PrevSubDelayL = 0.0f;

    // LFO and Filters Initialization
    m_LFO.Initialize(SAMPLING_RATE, 0.5, 1, 10, 0.5f);  // Reinitialize LFO

    // =============================================================================
    // Initialize UI Parameters
    // Delay 1 parameters
    m_Time.Init(DELAY_ID, 0.450f, DELAY_MIN_TIME, DELAY_MAX_TIME, 0.05f, 0.01f, nullptr, 0, 1.0f, 20);  		// Delay time parameter
    m_Repeat.Init(DELAY_ID, 30.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 1.0f, 21);  							// Feedback parameter
    m_Mix.Init(DELAY_ID, 10.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 1.0f, 22);  								// delay mix parameter

    // Delay 2 parameters
    m_SubDelay.Init(DELAY_ID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, nullptr, 0, 0.0f, 23);  							// Subdivision parameter
    m_RepeatDelay2.Init(DELAY_ID, 0.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 0.8f, 24);  						// Delay 2 feedback
    m_BlendD1D2.Init(DELAY_ID, 0.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 1.0f, 25);  							// Blend between delays

    // Tone control parameters
    m_Bass.Init(  DELAY_ID, 0.0f, 0.0f, -24.0f, 1.0f, 0.5f, BassChange,   (uint32_t)this, 0.0f, 26);  				// Bass control
    m_Treble.Init(DELAY_ID, 0.0f, 0.0f, -24.0f, 1.0f, 0.5f, TrebleChange, (uint32_t)this, 0.0f, 27);  			// Treble control
    m_Saturation.Init(DELAY_ID, 0.0f, 0.0f, 100.0f, 5.0f, 1.5f, SatChange, (uint32_t)this, 0.5f, 28);  			// Treble control

    // Modulation parameters
    m_ModulationDeep.Init(DELAY_ID, 10.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0, 1.0f, 29);  					// Modulation depth
    m_ModulationSpeed.Init(DELAY_ID, 1.5f, 0.25f, 8.0f, 0.5f, 0.05f, SpeedChange, (uint32_t)this, 0.5f, 30);  	// Modulation speed

    // Parameter Views Setup
    m_TimeView.Init(&m_Time, "Time", "Time", "s", "second");  		// Time parameter view
    m_RepeatView.Init(&m_Repeat, "Rep.", "Repeat", "%", "%");  		// Repeat parameter view
    m_MixView.Init(&m_Mix, "Wet Mix", "Wet Mix", "%", "%");  				// Mix parameter view

    // Discrete values for musical subdivisions
    m_SubDelayView.Init(&m_SubDelay, "Sub", "Sub Delay");  // Subdivision parameter view
    m_SubDelayView.AddDiscreteValue("1/8", "1/8");  // Eighth note
    m_SubDelayView.AddDiscreteValue("1/6", "1/6");  // Sixth note
    m_SubDelayView.AddDiscreteValue("1/4", "1/4");  // Quarter note
    m_SubDelayView.AddDiscreteValue("1/3", "1/3");  // Third note
    m_SubDelayView.AddDiscreteValue("3/8", "3/8");  // Three eighths
    m_SubDelayView.AddDiscreteValue("5/8", "5/8");  // Five eighths
    m_SubDelayView.AddDiscreteValue("2/3", "2/3");  // Two thirds
    m_SubDelayView.AddDiscreteValue("3/4", "3/4");  // Three quarters
    m_SubDelayView.AddDiscreteValue("5/6", "5/6");  // Five sixths
    m_SubDelayView.AddDiscreteValue("7/8", "7/8");  // Seven eighths

    m_RepeatDelay2View.Init(&m_RepeatDelay2, "Rep.", "Repeat", "%", "%");  			// Delay 2 repeat view
    m_BlendD1D2View.Init(&m_BlendD1D2, "Blend", "Blend D1/D2", "%", "%");  			// Blend view

    m_BassView.Init(&m_Bass, "Bass", "Bass", "dB", "dB");  							// Bass view
    m_TrebleView.Init(&m_Treble, "Treble", "Treble", "dB", "dB");  					// Treble view
    m_SaturationView.Init(&m_Saturation, "Sat", "Saturation", "%", "%");  			// Treble view

    m_ModulationDeepView.Init(&m_ModulationDeep, "Deep", "Mod. Deep", "%", "%");  	// Modulation depth view
    m_ModulationSpeedView.Init(&m_ModulationSpeed, "Speed", "Mod. Speed", "Hz", "Hz"); // Modulation speed view

    // Organize parameters into menu groups
    m_PanelDelay1.Init(&m_TimeView, &m_RepeatView, &m_MixView);  					// Delay 1 menu
    m_PanelDelay2.Init(&m_SubDelayView, &m_RepeatDelay2View, &m_BlendD1D2View);  	// Delay 2 menu
    m_PanelTone.Init(&m_BassView, &m_TrebleView, &m_SaturationView);  				// Tone menu
    m_PanelLFO.Init(&m_ModulationDeepView, nullptr, &m_ModulationSpeedView);  		// LFO menu

    // Build Main Menu
    m_Menu.Init();  // Initialize main menu
    m_Menu.addMenuItem(&m_PanelDelay1, "Delay1");  	// Add delay 1 menu item
    m_Menu.addMenuItem(&m_PanelDelay2, "Delay2");  	// Add delay 2 menu item
    m_Menu.addMenuItem(&m_PanelTone, "Tone");  		// Add tone menu item
    m_Menu.addMenuItem(&m_PanelLFO, "LFO");  		// Add LFO menu item

    // Initialize Tap tempo
    m_pTapTempoParameter = &m_Time;
    m_TempoType = DadGUI::eTempoType::period;
}

// -----------------------------------------------------------------------------
// Function: Process
// Description: Main audio processing function
// -----------------------------------------------------------------------------
void cDelay::onProcess(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence) {

	// Update LFO and dry/wet processing
    m_LFO.Step();  // Advance LFO

    // Compute modulated delay time using LFO
    float LFO1 = m_LFO.getTriangleValue();  // Get LFO value for left channel
    float LFO2 = m_LFO.getTriangleValuePhased(0.25f);  // Get phase-shifted LFO for right channel

    m_PrevTime += SMOOTH_COEFF * (m_Time - m_PrevTime);

    float Delay = m_PrevTime * SAMPLING_RATE;  // Convert delay time to samples

    float DelayL = Delay - (LFO1 * m_ModulationDeep * 0.8);  // Modulated left delay
    float DelayR = Delay - (LFO2 * m_ModulationDeep * 0.78);  // Modulated right delay

    // Compute musical subdivision for delay 2
    float SubDelayL;  // Left channel subdivision delay
    float SubDelayR;  // Right channel subdivision delay

    // Calculate subdivision based on selected value
    switch ((uint32_t)m_SubDelay.getValue()) {
        case 0:  // 1/8
            SubDelayL = DelayL / 8.0f;
            SubDelayR = DelayR / 8.0f;
            break;
        case 1:  // 1/6
            SubDelayL = DelayL / 6.0f;
            SubDelayR = DelayR / 6.0f;
            break;
        case 2:  // 1/4
            SubDelayL = DelayL / 4.0f;
            SubDelayR = DelayR / 4.0f;
            break;
        case 3:  // 1/3
            SubDelayL = DelayL / 3.0f;
            SubDelayR = DelayR / 3.0f;
            break;
        case 4:  // 3/8
            SubDelayL = DelayL * 3.0f / 8.0f;
            SubDelayR = DelayR * 3.0f / 8.0f;
            break;
        case 5:  // 5/8
            SubDelayL = DelayL * 5.0f / 8.0f;
            SubDelayR = DelayR * 5.0f / 8.0f;
            break;
        case 6:  // 2/3
            SubDelayL = DelayL * 2.0f / 3.0f;
            SubDelayR = DelayR * 2.0f / 3.0f;
            break;
        case 7:  // 3/4
            SubDelayL = DelayL * 3.0f / 4.0f;
            SubDelayR = DelayR * 3.0f / 4.0f;
            break;
        case 8:  // 5/6
            SubDelayL = DelayL * 5.0f / 6.0f;
            SubDelayR = DelayR * 5.0f / 6.0f;
            break;
        case 9:  // 7/8
            SubDelayL = DelayL * 7.0f / 8.0f;
            SubDelayR = DelayR * 7.0f / 8.0f;
            break;
        default:
            SubDelayL = DelayL;  // Default to main delay
            SubDelayR = DelayR;  // Default to main delay
    }

    // --- Delay Processing 1 ---
    // Read from delay line 1
    float OutRight = m_Delay1LineRight.Pull(DelayR);  // Get delayed right signal
    float OutLeft = m_Delay1LineLeft.Pull(DelayL);    // Get delayed left signal

    // Apply Saturation
    OutRight = std::tanh(OutRight * m_SatDrive) / m_SatDrive;
    OutLeft = std::tanh(OutLeft * m_SatDrive) / m_SatDrive;

    // Apply tone shaping filters to delay 1
    OutRight = m_BassFilter1.Process(OutRight, DadDSP::eChannel::Right);  // Apply bass filter
    OutLeft = m_BassFilter1.Process(OutLeft, DadDSP::eChannel::Left);     // Apply bass filter

    OutRight = m_TrebleFilter1.Process(OutRight, DadDSP::eChannel::Right);  // Apply treble filter
    OutLeft = m_TrebleFilter1.Process(OutLeft, DadDSP::eChannel::Left);     // Apply treble filter

    // Feedback path with optional input injection
    if (OnOff == eOnOff::On) {
        // When effect is on, mix input with feedback
        m_Delay1LineRight.Push((pIn->Right + OutRight) * m_Repeat / 100);  // Push to delay line
        m_Delay1LineLeft.Push((pIn->Left + OutLeft) * m_Repeat / 100);     // Push to delay line
    } else {
        // When effect is off, use feedback only
        m_Delay1LineRight.Push(OutRight * m_Repeat / 100);  // Push to delay line
        m_Delay1LineLeft.Push(OutLeft * m_Repeat / 100);    // Push to delay line
    }

    // --- Delay Processing 2 ---
    float Out2Right;  // Delay 2 right output
    float Out2Left;   // Delay 2 left output

    m_PrevSubDelayR += SMOOTH_COEFF * (SubDelayR - m_PrevSubDelayR);
    m_PrevSubDelayL += SMOOTH_COEFF * (SubDelayL - m_PrevSubDelayL);

    if (m_RepeatDelay2 == 0) {
        // Use delay line 1 as source for delay 2
        Out2Right = m_Delay1LineRight.Pull(m_PrevSubDelayR);  // Read from delay 1
        Out2Left = m_Delay1LineLeft.Pull(m_PrevSubDelayL);    // Read from delay 1
    } else {
        // Use dedicated delay line 2
        Out2Right = m_Delay2LineRight.Pull(m_PrevSubDelayR);  // Read from delay 2
        Out2Left = m_Delay2LineLeft.Pull(m_PrevSubDelayL);    // Read from delay 2
    }

    // Apply Saturation
    Out2Right = std::tanh(Out2Right * m_SatDrive) / m_SatDrive;
    Out2Left = std::tanh(Out2Left * m_SatDrive) / m_SatDrive;

    // Apply tone shaping to delay 2
    Out2Right = m_BassFilter2.Process(Out2Right, DadDSP::eChannel::Right);  // Apply bass filter
    Out2Left = m_BassFilter2.Process(Out2Left, DadDSP::eChannel::Left);     // Apply bass filter

    Out2Right = m_TrebleFilter2.Process(Out2Right, DadDSP::eChannel::Right);  // Apply treble filter
    Out2Left = m_TrebleFilter2.Process(Out2Left, DadDSP::eChannel::Left);     // Apply treble filter

    // Feedback path for delay 2
    m_Delay2LineRight.Push((pIn->Right + Out2Right) * m_RepeatDelay2 * 0.01f);  // Push to delay line 2
    m_Delay2LineLeft.Push((pIn->Left + Out2Left) * m_RepeatDelay2 * 0.01f);     // Push to delay line 2

    // --- Delay1 and Delay2 Blending ---
    float mix = m_BlendD1D2 * 0.01f;  // Normalize blend parameter
    float gain1 = cosf(mix * M_PI_2);  // Crossfade gain for delay 1
    float gain2 = sinf(mix * M_PI_2);  // Crossfade gain for delay 2

    // Blend the two delay outputs
    OutRight = ((OutRight * gain1) + (Out2Right * gain2));  // Blend right channel
    OutLeft = ((OutLeft * gain1) + (Out2Left * gain2));     // Blend left channel

    // Apply wet gain and output
    float Mix = 3 * m_Mix.getValue() / 100;
    pOut->Left = OutLeft * Mix;    // Apply wet gain to left channel
    pOut->Right = OutRight * Mix;  // Apply wet gain to right channel
}

// -----------------------------------------------------------------------------
// Function: SpeedChange
// Description: Modulation speed callback (updates LFO frequency)
// -----------------------------------------------------------------------------
void cDelay::SpeedChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cDelay *pthis = (cDelay *)CallbackUserData;  // Get class instance
    pthis->m_LFO.setFreq(pParameter->getValue());  // Update LFO frequency
}

// -----------------------------------------------------------------------------
// Function: BassChange
// Description: Bass control callback - sets high-pass filter frequency
// -----------------------------------------------------------------------------
void cDelay::BassChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cDelay *pthis = (cDelay *)CallbackUserData;  // Get class instance
    float gain = pParameter->getValue();

    // Calculate frequency from normalized parameter value using logarithmic scale
    pthis->m_BassFilter1.setGainDb(gain);
    pthis->m_BassFilter1.CalculateParameters();

    pthis->m_BassFilter2.setGainDb(gain);
    pthis->m_BassFilter2.CalculateParameters();
}

// -----------------------------------------------------------------------------
// Function: TrebleChange
// Description: Treble control callback - sets low-pass filter frequency
// -----------------------------------------------------------------------------
void cDelay::TrebleChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cDelay *pthis = (cDelay *)CallbackUserData;  // Get class instance
    float gain = pParameter->getValue();

    pthis->m_TrebleFilter1.setGainDb(gain);
    pthis->m_TrebleFilter1.CalculateParameters();

    pthis->m_TrebleFilter2.setGainDb(gain);
    pthis->m_TrebleFilter2.CalculateParameters();
}

// -----------------------------------------------------------------------------
// Function: SatChange
// Description: Saturation control callback
// -----------------------------------------------------------------------------
void cDelay::SatChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    cDelay *pthis = (cDelay *)CallbackUserData;  // Get class instance
    float gain = pParameter->getValue() * 0.01;
    pthis->m_SatDrive = 1.0f + gain * 10.0f;
}


}  // namespace DadEffect
#endif
//***End of file**************************************************************
