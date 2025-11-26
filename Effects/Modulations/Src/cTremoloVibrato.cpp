//==================================================================================
//==================================================================================
// File: TremoloVibrato.cpp
// Description: Tremolo and Vibrato audio effect implementation
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cTremoloVibrato.h"

// Vibrato constants
constexpr float DELAY_MAX_TIME = 0.02f;  // Maximum modulation delay time in seconds

// Tremolo/Vibrato constants
constexpr float FREQ_MIN = 0.5f;  // Minimum LFO frequency in Hz
constexpr float FREQ_MAX = 9.0f;  // Maximum LFO frequency in Hz

//**********************************************************************************
// Utility function: ceil_to_uint
// Description: Rounds a float up to the next unsigned integer
//**********************************************************************************
constexpr uint32_t ceil_to_uint(float value) {
    return static_cast<uint32_t>(value + 0.999f);
}

// Compute delay buffer size based on sampling rate and max delay time
constexpr uint32_t DELAY_BUFFER_SIZE = ceil_to_uint(SAMPLING_RATE * DELAY_MAX_TIME);

// Allocate modulation delay buffers in external SDRAM (+100 samples for safe interpolation)
SDRAM_SECTION float __ModulationBufferLeftA[DELAY_BUFFER_SIZE + 100];
SDRAM_SECTION float __ModulationBufferRightA[DELAY_BUFFER_SIZE + 100];

namespace DadEffect {

//**********************************************************************************
// Class: cTremoloVibrato
// Description: Implements tremolo and vibrato audio effects using LFO modulation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Method: onInitialize
// Description: Initializes effect parameters and configuration
// ---------------------------------------------------------------------------------
void cTremoloVibrato::onInitialize(){
	m_pShortName = "Tre/Vibr";          // Short name identifier
	m_pLongName	 = "Tremolo / Vibrato"; // Long descriptive name
	m_ID = TREMOLO_ID;                  // Unique effect identifier

    // =============================================================================
    // PARAMETER INITIALIZATION SECTION
    // =============================================================================

	// LFO Frequency parameter
	m_Freq.Init(TREMOLO_ID, 2.5f, FREQ_MIN, FREQ_MAX, 0.5f, 0.1f, SpeedChange, (uint32_t)this,
				5.0f * RT_RATE, 20);

	// Tremolo Depth parameter
	m_TremoloDeep.Init(TREMOLO_ID, 45.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0,
					   0.5f * RT_RATE, 21);

	// Vibrato Depth parameter (used for delay-based pitch modulation)
	m_VibratoDeep.Init(TREMOLO_ID, 0.0f, 0.0f, 100.0f, 5.0f, 1.0f, nullptr, 0,
					   0.5f * RT_RATE, 22);

	// DryWet Mix parameter
	m_DryWetMix.Init(TREMOLO_ID, 0, 0, 100, 5, 1, MixChange, (uint32_t) this, 0.5f * RT_RATE, 23);

	// LFO Shape parameter (0: Triangle, 1: Square)
	m_LFOShape.Init(TREMOLO_ID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, nullptr, 0,
					0.0f, 24);

	// LFO Duty Cycle Ratio parameter
	m_LFORatio.Init(TREMOLO_ID, 50.0f, 0.0f, 100.0f, 5.0f, 1.0f, RatioChange, (uint32_t)this,
					0.5f * RT_RATE, 25);

	// Stereo mode parameter
	m_StereoMode.Init(TREMOLO_ID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, nullptr, 0, 0.0f, 26);

    // =============================================================================
    // VIEW SETUP SECTION
    // =============================================================================

	m_FreqView.Init(&m_Freq, "Freq", "Frequency", "Hz", "Hz");                    // Frequency view
	m_TremoloDeepView.Init(&m_TremoloDeep, "Tremolo", "Tremolo depth", "%", "%"); // Tremolo depth view
	m_VibratoDeepView.Init(&m_VibratoDeep, "Vibrato", "Vibrato depth", "%", "%"); // Vibrato depth view
	m_DryWetMixView.Init(&m_DryWetMix, "Dry", "Dry", "%", "%");                   // Dry/Wet mix view
	m_LFORatioView.Init(&m_LFORatio, "Ratio", "Ratio", "%", "%");                 // LFO ratio view
	m_LFOShapeView.Init(&m_LFOShape, "Shape", "Shape");                           // LFO shape view
	m_LFOShapeView.AddDiscreteValue("Sinus", "");                                 // Sinus shape option
	m_LFOShapeView.AddDiscreteValue("Square", "Square");                          // Square shape option
	m_StereoModeView.Init(&m_StereoMode, "Stereo", "Stereo");                     // Stereo mode view
	m_StereoModeView.AddDiscreteValue("No", "No St. Effect");                     // No stereo effect
	m_StereoModeView.AddDiscreteValue("Trem", "Tremolo St.");                     // Tremolo stereo
	m_StereoModeView.AddDiscreteValue("Vibr", "Vibrato St.");                     // Vibrato stereo
	m_StereoModeView.AddDiscreteValue("Both", "Both St.");                        // Both stereo effects

    // =============================================================================
    // MENU GROUPING SECTION
    // =============================================================================

	m_ItemTremoloMenu.Init(&m_TremoloDeepView, &m_VibratoDeepView, &m_DryWetMixView); // Tremolo menu items
	m_ItemLFOMenu.Init(&m_LFOShapeView, &m_LFORatioView, &m_FreqView);                // LFO menu items
	m_ItemStereoMode.Init(&m_StereoModeView, nullptr, nullptr);                       // Stereo mode menu item

    // =============================================================================
    // MAIN MENU CONFIGURATION SECTION
    // =============================================================================

	m_Menu.addMenuItem(&m_ItemTremoloMenu, "Tre/Vib");   // Add tremolo menu to main section
	m_Menu.addMenuItem(&m_ItemLFOMenu, "LFO");        // Add LFO menu to LFO section
	m_Menu.addMenuItem(&m_ItemStereoMode, "Stereo");  // Add stereo mode to stereo section

    // =============================================================================
    // LFO AND DELAY BUFFER INITIALIZATION SECTION
    // =============================================================================

    // Initialize left and right channel LFOs
	m_LFOLeft.Initialize(SAMPLING_RATE, m_Freq, 1, 10, m_LFORatio.getNormalizedValue());
	m_LFORight.Initialize(SAMPLING_RATE, m_Freq, 1, 10, m_LFORatio.getNormalizedValue());
	m_LFORight.setPosition(0.5f);  // Offset right LFO phase for stereo effect

    // Initialize modulation delay lines
	m_ModulationLineRight.Initialize(__ModulationBufferRightA, DELAY_BUFFER_SIZE);
	m_ModulationLineRight.Clear();

	m_ModulationLineLeft.Initialize(__ModulationBufferLeftA, DELAY_BUFFER_SIZE);
	m_ModulationLineLeft.Clear();
}

// ---------------------------------------------------------------------------------
// Method: onActivate
// Description: Called when effect becomes active
// ---------------------------------------------------------------------------------
void cTremoloVibrato::onActivate(){
	__DryWet.setMix(m_DryWetMix.getValue());  // Set initial dry/wet mix
}

// ---------------------------------------------------------------------------------
// Method: onDesactivate
// Description: Called when effect becomes inactive
// ---------------------------------------------------------------------------------
void cTremoloVibrato::onDesactivate(){
	// Deactivation logic placeholder
}

// ---------------------------------------------------------------------------------
// Method: Process
// Description: Audio processing method - applies effect to input buffer
// ---------------------------------------------------------------------------------
void cTremoloVibrato::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff){
    // Update LFO phases
	m_LFOLeft.Step();
	m_LFORight.Step();

    // =============================================================================
    // TREMOLO MODULATION CALCULATION SECTION
    // =============================================================================

	float VolumeModulationLeft = 0.0f;     // Left channel volume modulation
	float VolumeModulationRight = 0.0f;    // Right channel volume modulation
	float TremoloDeep = (m_TremoloDeep / 100.0f);  // Normalized tremolo depth

    // Calculate volume modulation based on LFO shape and stereo mode
	switch(static_cast<uint32_t>(m_LFOShape.getValue())){
		case 0:  // Sine wave shape
			if((m_StereoMode == 0) || (m_StereoMode == 2)){
                // Mono tremolo: same modulation for both channels
				VolumeModulationLeft = 1 - (m_LFOLeft.getSineValue() * TremoloDeep);
				VolumeModulationRight = VolumeModulationLeft;
			}else if((m_StereoMode == 1) || (m_StereoMode == 3)){
                // Stereo tremolo: independent modulation per channel
				VolumeModulationLeft = 1 - (m_LFOLeft.getSineValue() * TremoloDeep);
				VolumeModulationRight = 1 - (m_LFORight.getSineValue() * TremoloDeep);
			}
			break;
		case 1:  // Square wave shape
			if((m_StereoMode == 0) || (m_StereoMode == 2)){
                // Mono tremolo: same modulation for both channels
				VolumeModulationLeft = 1 - (m_LFOLeft.getSquareModValue() * TremoloDeep);
				VolumeModulationRight = VolumeModulationLeft;
			}else if((m_StereoMode == 1) || (m_StereoMode == 3)){
                // Stereo tremolo: independent modulation per channel
				VolumeModulationLeft = 1 - (m_LFOLeft.getSquareModValue() * TremoloDeep);
				VolumeModulationRight = 1 - (m_LFORight.getSquareModValue() * TremoloDeep);
			}
			break;
	}

    // =============================================================================
    // VIBRATO DELAY CALCULATION SECTION
    // =============================================================================

    // Get LFO sine values for delay modulation
	float LFOSinLeft = m_LFOLeft.getSineValue();   // Left channel LFO value
	float LFOSinRight = m_LFORight.getSineValue(); // Right channel LFO value
	float DelayLeft = 0.0f;    // Left channel delay in samples
	float DelayRight = 0.0f;   // Right channel delay in samples

    // Calculate vibrato delay based on stereo mode
	if((m_StereoMode == 0) || (m_StereoMode == 1)){
        // Mono vibrato: same delay for both channels
		DelayLeft = DELAY_BUFFER_SIZE * LFOSinLeft * m_CoefComp * (m_VibratoDeep/100)  * 0.5f;
		DelayRight = DelayLeft;
	}else if((m_StereoMode == 2) || (m_StereoMode == 3)){
        // Stereo vibrato: independent delay per channel
		DelayLeft = DELAY_BUFFER_SIZE * LFOSinLeft * m_CoefComp * (m_VibratoDeep/100)  * 0.5f;
		DelayRight = DELAY_BUFFER_SIZE * LFOSinRight * m_CoefComp * (m_VibratoDeep/100)  * 0.5f;
	}

    // =============================================================================
    // DELAY LINE PROCESSING SECTION
    // =============================================================================

    // Push current samples to delay lines
	m_ModulationLineLeft.Push(pIn->Left);
	m_ModulationLineRight.Push(pIn->Right);

    // Process output based on effect on/off state
	if(OnOff == eOnOff::On){
        // Effect ON: apply modulated delay and tremolo
		pOut->Right = m_ModulationLineLeft.Pull(DelayLeft) * VolumeModulationLeft;
		pOut->Left = m_ModulationLineRight.Pull(DelayRight) * VolumeModulationRight;
	}else{
        // Effect OFF: output silence
		pOut->Right = 0.0f;
		pOut->Left = 0.0f;
	}
}

// ---------------------------------------------------------------------------------
// Method: SpeedChange (Callback)
// Description: Updates LFO frequency and compensation factor when speed changes
// ---------------------------------------------------------------------------------
void cTremoloVibrato::SpeedChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData){
	cTremoloVibrato *pthis = reinterpret_cast<cTremoloVibrato *>(CallbackUserData);

    // Update LFO frequencies
	pthis->m_LFOLeft.setFreq(pParameter->getValue());
	pthis->m_LFORight.setFreq(pParameter->getValue());

    // Compute compensation factor to keep vibrato depth consistent across LFO frequencies
    // This avoids a shallower vibrato effect when the LFO speed increases
	pthis->m_CoefComp = (2 * FREQ_MIN) / (2 * (FREQ_MIN + (FREQ_MAX - FREQ_MIN) * pParameter->getNormalizedValue()));
}

// ---------------------------------------------------------------------------------
// Method: RatioChange (Callback)
// Description: Updates LFO duty cycle ratio
// ---------------------------------------------------------------------------------
void cTremoloVibrato::RatioChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData){
	cTremoloVibrato *pthis = reinterpret_cast<cTremoloVibrato *>(CallbackUserData);

    // Update LFO duty cycles
	pthis->m_LFOLeft.setNormalizedDutyCycle(pParameter->getNormalizedValue());
	pthis->m_LFORight.setNormalizedDutyCycle(pParameter->getNormalizedValue());
}

// ---------------------------------------------------------------------------------
// Method: MixChange (Callback)
// Description: Updates dry/wet mix parameter
// ---------------------------------------------------------------------------------
void cTremoloVibrato::MixChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData){
    // Update dry/wet mix (inverted since parameter represents dry level)
	__DryWet.setMix(100-pParameter->getValue());
}

} // namespace DadEffect

//***End of file**************************************************************
