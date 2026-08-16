// ======================================================================
// ======================================================================
// File: cPanelOfParametricEqualizer.cpp
// Description: 5-band parametric EQ panel implementation
//
// Copyright(c) 2026 DadDesign-Projects
//
// SPDX-License-Identifier: Apache-2.0
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// ======================================================================
// ======================================================================

//**********************************************************************************
// Includes
//**********************************************************************************
#include "cPanelOfParametricEqualizer.h"
#include "cDisplay.h"
#include "cThemesManager.h"
#include "MainGUI.h"
#include "cEncoder.h"
#include "HardwareDefines.h"
#include <cmath>
#include <cstdio>

//**********************************************************************************
// Global variables
//**********************************************************************************
extern DadGFX::cDisplay 	__Display;
extern DadGUI::cMainGUI 	__GUI;
extern DadDrivers::cEncoder	__Encoder1;
extern DadDrivers::cEncoder	__Encoder2;
extern DadDrivers::cEncoder	__Encoder3;

namespace DadGUI {
extern cThemesManager	__ThemesManager;

//**********************************************************************************
// Layer declarations
//**********************************************************************************
DECLARE_LAYER(EQStatLayer, SCREEN_WIDTH, PARAM_HEIGHT);   // Static layer for background
DECLARE_LAYER(EQDynLayer,  SCREEN_WIDTH, PARAM_HEIGHT);   // Dynamic layer for curve and markers

//**********************************************************************************
// Local constants and helpers
//**********************************************************************************
namespace {

	// Graph layout ------------------------------------------------------------
	constexpr uint16_t kStatusHeight  = 12;   // Height reserved for the status line
	constexpr uint16_t kMarkerMargin  = 10;   // Room above/below the curve for filter labels
	constexpr uint16_t kGraphMarginX  = 4;    // Left/right margin of the graph

	constexpr uint16_t kGraphX      = kGraphMarginX;
	constexpr uint16_t kGraphY      = kStatusHeight + kMarkerMargin;
	constexpr uint16_t kGraphWidth  = SCREEN_WIDTH - (2 * kGraphMarginX);
	constexpr uint16_t kGraphHeight = PARAM_HEIGHT - kStatusHeight - (2 * kMarkerMargin);

	// Response range ------------------------------------------------------------
	constexpr float kMinFreq = 25.0f;     // Minimum displayed frequency
	constexpr float kMaxFreq = 20000.0f;  // Maximum displayed frequency
	constexpr float kMinGain = -20.0f;    // Minimum displayed gain in dB
	constexpr float kMaxGain =  20.0f;    // Maximum displayed gain in dB

	// Filter type rotation list (Encoder 2)
	// AFP is intentionally excluded as it's not part of parametric EQ workflow
	constexpr DadDSP::FilterType kFilterTypeList[] = {
		DadDSP::FilterType::LPF,
		DadDSP::FilterType::LPF24,
		DadDSP::FilterType::HPF,
		DadDSP::FilterType::HPF24,
		DadDSP::FilterType::BPF,
		DadDSP::FilterType::NOTCH,
		DadDSP::FilterType::PEQ,
		DadDSP::FilterType::LSH,
		DadDSP::FilterType::HSH
	};
	constexpr uint8_t kNumFilterTypes = sizeof(kFilterTypeList) / sizeof(kFilterTypeList[0]);

	// --------------------------------------------------------------------------
	// Clamp a float value between lo and hi bounds
	inline float clampf(float v, float lo, float hi) {
		if (v < lo) return lo;
		if (v > hi) return hi;
		return v;
	}

	// --------------------------------------------------------------------------
	// Find the index of a filter type in the rotation list (0 if not found)
	int findTypeIndex(DadDSP::FilterType type) {
		for (uint8_t i = 0; i < kNumFilterTypes; ++i) {
			if (kFilterTypeList[i] == type) return i;
		}
		return 0;
	}

	// --------------------------------------------------------------------------
	// Return a dimmed copy of a color (used for disabled filters)
	DadGFX::sColor dimColor(const DadGFX::sColor& c, float factor) {
		return DadGFX::sColor(
			static_cast<uint8_t>(c.getR() * factor),
			static_cast<uint8_t>(c.getG() * factor),
			static_cast<uint8_t>(c.getB() * factor),
			c.getA());
	}

	// --------------------------------------------------------------------------
	// Map frequency (25Hz..20kHz, log scale) to X offset in [0, kGraphWidth-1]
	uint16_t freqToX(float freq) {
		freq = clampf(freq, kMinFreq, kMaxFreq);
		float t = log10f(freq / kMinFreq) / log10f(kMaxFreq / kMinFreq);
		return static_cast<uint16_t>(t * (kGraphWidth - 1));
	}

	// --------------------------------------------------------------------------
	// Map X offset in [0, kGraphWidth-1] to frequency (log scale)
	float xToFreq(uint16_t x) {
		float t = static_cast<float>(x) / static_cast<float>(kGraphWidth - 1);
		return kMinFreq * powf(10.0f, t * log10f(kMaxFreq / kMinFreq));
	}

	// --------------------------------------------------------------------------
	// Map gain in dB (-20..+20) to Y coordinate within the graph
	uint16_t gainToY(float gainDb) {
		gainDb = clampf(gainDb, kMinGain, kMaxGain);
		float t = (gainDb - kMinGain) / (kMaxGain - kMinGain); // 0 = bottom, 1 = top
		return kGraphY + (kGraphHeight - 1) - static_cast<uint16_t>(t * (kGraphHeight - 1));
	}

} // anonymous namespace

//**********************************************************************************
// Public methods
//**********************************************************************************

// -----------------------------------------------------------------------------
// Function: Init
// Description: Initialize the panel with default filter settings
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Init(uint32_t SerializeID) {
	// Create display layers
	m_pDynLayer  = ADD_LAYER(__Display, EQDynLayer, 0, MENU_HEIGHT, 0);
	m_pStatLayer = ADD_LAYER(__Display, EQStatLayer, 0, MENU_HEIGHT, 0);
	m_isActive   = false;
	m_SerializeID = SerializeID;

	m_bCalcCoeffs = false;

	// Subscribe to RT_ProcessOut event for audio processing
    DadGUI::__GUI_EventManager.Subscribe_RT_ProcessOut(this);

    // Subscribe to serialize events for persistence
    DadGUI::__GUI_EventManager.Subscribe_AllSerializeEvents(this, SerializeID);

	// Default frequencies spread across the spectrum
	constexpr float kDefaultFreq[kNumFilters]  = { 100.0f, 300.0f, 1000.0f, 3000.0f, 8000.0f };

	// Initialize filters with default parameters
	for (uint8_t i = 0; i < kNumFilters; ++i) {
		DadDSP::FilterType Type;
		if(i == 0){
			Type = DadDSP::FilterType::HPF;        // First filter: high-pass
		}else if (i == (kNumFilters -1)){
			Type = DadDSP::FilterType::LPF;        // Last filter: low-pass
		}else{
			Type = DadDSP::FilterType::PEQ;        // Middle filters: parametric EQ
		}
		m_Filter[i].Initialize(SAMPLING_RATE, kDefaultFreq[i], 0.0f, 1.0f, Type);
		m_FilterEnabled[i] = false;                // All filters disabled by default
		m_FilterFaderEnabled[i] = false;
	}
	// Initialise Fader
	m_Fader.Init(300.0f, 300.0f, SAMPLING_RATE, nullptr, FadeOutEndCallback, (void*) this, nullptr);
	m_Fader.TriggerFadeIn();

	// Initialize UI state
	m_SelectedFilter    = 0;
	m_PreviewTypeIndex  = findTypeIndex(m_Filter[m_SelectedFilter].getType());
	m_SelectedParameter = eEQParameter::Frequency;

	// Reset encoder switch states
	m_PrevSwitch1 = 0;
	m_PrevSwitch2 = 0;
	m_PrevSwitch3 = 0;

	m_Dirty = false;
}

// -----------------------------------------------------------------------------
// Function: Activate
// Description: Called when the component becomes active and visible
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Activate() {
	// Set layer Z-order to make them visible
	m_pStatLayer->changeZOrder(40);
	m_pDynLayer->changeZOrder(41);

	m_isActive = true;

	// Draw the initial display
	drawStatPartOffLayer();
	drawDynPartOffLayer();
}

// -----------------------------------------------------------------------------
// Function: Deactivate
// Description: Called when the component is deactivated or hidden
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Deactivate() {
	// Hide layers by setting Z-order to 0
	m_pStatLayer->changeZOrder(0);
	m_pDynLayer->changeZOrder(0);

	m_isActive = false;
}

// -----------------------------------------------------------------------------
// Function: Update
// Description: Called every frame while active. Reads encoders and refreshes display.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Update() {
	if (!m_isActive) return;

	bool redraw = false;

	// Process all three encoders
	ProcessEncoder1(redraw);
	ProcessEncoder2(redraw);
	ProcessEncoder3(redraw);

	// Redraw dynamic layer if any change occurred
	if (redraw) {
		drawDynPartOffLayer();
	}
}

// -----------------------------------------------------------------------------
// Function: Redraw
// Description: Forces a complete redraw of static and dynamic parts
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Redraw() {
	if (m_isActive) {
		drawStatPartOffLayer();
		drawDynPartOffLayer();
	}
}

// -----------------------------------------------------------------------------
// Function: on_GUI_RT_ProcessOut
// Description: Audio processing - runs signal through all active filters in series
//              Uses generic Process(sample, eChannel) path so that 24dB filters
//              (LPF24/HPF24) are correctly cascaded internally by cBiQuad.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::on_GUI_RT_ProcessOut(AudioBuffer *pInOut){
	float left  = pInOut->Left;
	float right = pInOut->Right;

	m_Fader.Process();

	// Process through all filters in series
	for (uint8_t i = 0; i < kNumFilters; ++i) {
		m_Fader.Process();
		if (m_FilterEnabled[i]) {
			left  = m_Filter[i].Process(left,  DadDSP::eChannel::Left);
			right = m_Filter[i].Process(right, DadDSP::eChannel::Right);
		}
	}
	float	FaderGain = m_Fader.GetGainAudio();

	pInOut->Left  = left * FaderGain;
	pInOut->Right = right * FaderGain;
}

//**********************************************************************************
// Encoder handling
//**********************************************************************************

// -----------------------------------------------------------------------------
// Function: ProcessEncoder1
// Description: Rotation selects the filter being edited (circular).
//              Press toggles enable/disable of that filter's DSP processing.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::ProcessEncoder1(bool &redraw) {
	int8_t inc = __Encoder1.getIncrement();
	if (inc != 0) {
		// Circular selection of filter
		int32_t next = static_cast<int32_t>(m_SelectedFilter) + inc;
		next = ((next % kNumFilters) + kNumFilters) % kNumFilters;
		m_SelectedFilter = static_cast<uint8_t>(next);

		// Resync the type preview on the newly selected filter's actual type
		m_PreviewTypeIndex = findTypeIndex(m_Filter[m_SelectedFilter].getType());
		redraw = true;
	}

	// Handle press to toggle filter enable
	uint8_t sw = __Encoder1.getSwitchState();
	if (sw == 1 && m_PrevSwitch1 == 0) {
		if(true == m_FilterEnabled[m_SelectedFilter]){
			m_FilterFaderEnabled[m_SelectedFilter]= false;
		}else{
			m_FilterFaderEnabled[m_SelectedFilter] = true;
		}
		m_bCalcCoeffs = false;
		m_Fader.TriggerFadeOut();
		m_Dirty = true;
		redraw = true;
	}
	m_PrevSwitch1 = sw;
}

// -----------------------------------------------------------------------------
// Function: ProcessEncoder2
// Description: Rotation browses the filter type list (circular) as a preview.
//              Press commits the previewed type to the selected filter.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::ProcessEncoder2(bool &redraw) {
	int8_t inc = __Encoder2.getIncrement();
	if (inc != 0) {
		// Circular browsing of filter types
		int idx = static_cast<int>(m_PreviewTypeIndex);
		idx = ((idx + inc) % kNumFilterTypes + kNumFilterTypes) % kNumFilterTypes;
		m_PreviewTypeIndex = static_cast<uint8_t>(idx);
		redraw = true;
	}

	// Handle press to apply previewed type
	uint8_t sw = __Encoder2.getSwitchState();
	if (sw == 1 && m_PrevSwitch2 == 0) {
		DadDSP::cBiQuad &filter = m_Filter[m_SelectedFilter];
		filter.setType(kFilterTypeList[m_PreviewTypeIndex]);
		filter.CalculateParameters();
		m_Dirty = true;
		redraw = true;
	}
	m_PrevSwitch2 = sw;
}

// -----------------------------------------------------------------------------
// Function: ProcessEncoder3
// Description: Rotation edits the currently selected parameter in real time.
//              Press cycles which parameter is edited (circular).
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::ProcessEncoder3(bool &redraw) {
	int8_t inc = __Encoder3.getIncrement();
	if (inc != 0) {
		applyParameterIncrement(inc);
		m_Dirty = true;
		redraw = true;
	}

	// Handle press to cycle through parameters
	uint8_t sw = __Encoder3.getSwitchState();
	if (sw == 1 && m_PrevSwitch3 == 0) {
		int idx = static_cast<int>(m_SelectedParameter);
		idx = (idx + 1) % 3;
		m_SelectedParameter = static_cast<eEQParameter>(idx);
		redraw = true;
	}
	m_PrevSwitch3 = sw;
}

// -----------------------------------------------------------------------------
// Function: applyParameterIncrement
// Description: Applies one encoder step to the selected parameter of the filter.
//              Frequency and Width use multiplicative (log-feeling) step,
//              Gain uses a fixed linear dB step.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::applyParameterIncrement(int8_t steps) {
	DadDSP::cBiQuad &filter = m_Filter[m_SelectedFilter];

	switch (m_SelectedParameter) {
	case eEQParameter::Frequency: {
		// Logarithmic frequency adjustment (~24 steps per octave)
		constexpr float kStepsPerOctave = 24.0f;
		float ratio = powf(2.0f, static_cast<float>(steps) / kStepsPerOctave);
		float freq = clampf(filter.getCutoffFreq() * ratio, kMinFreq, kMaxFreq);
		filter.setCutoffFreq(freq);
		break;
	}
	case eEQParameter::Gain: {
		// Linear gain adjustment (0.5 dB per step)
		constexpr float kDbStep = 0.5f;
		float gain = clampf(filter.getGainDb() + steps * kDbStep, kMinGain, kMaxGain);
		filter.setGainDb(gain);
		break;
	}
	case eEQParameter::Width: {
		// Logarithmic bandwidth adjustment (~6% per step)
		constexpr float kRatioPerStep = 1.06f;
		float width = clampf(filter.getBandwidth() * powf(kRatioPerStep, static_cast<float>(steps)), 0.10f, 5.00f);
		filter.setBandwidth(width);
		break;
	}
	}

	// Recalculate biquad coefficients after parameter change
	filter.CalculateParameters();
}

// -----------------------------------------------------------------------------
// Function: getFilterTypeName
// Description: Returns a short display name for a filter type
// -----------------------------------------------------------------------------
const char* cPanelOfParametricEqualizer::getFilterTypeName(DadDSP::FilterType type) {
	switch (type) {
	case DadDSP::FilterType::LPF:   return "LPF";
	case DadDSP::FilterType::LPF24: return "LPF24";
	case DadDSP::FilterType::HPF:   return "HPF";
	case DadDSP::FilterType::HPF24: return "HPF24";
	case DadDSP::FilterType::BPF:   return "BPF";
	case DadDSP::FilterType::NOTCH: return "NOTCH";
	case DadDSP::FilterType::PEQ:   return "PEQ";
	case DadDSP::FilterType::LSH:   return "LSH";
	case DadDSP::FilterType::HSH:   return "HSH";
	case DadDSP::FilterType::AFP:   return "AFP";
	}
	return "?";
}

//**********************************************************************************
// Persistence
//**********************************************************************************

// -----------------------------------------------------------------------------
// Function: Save
// Description: Serializes all 5 filters (enabled, type, frequency, gain, width)
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Save(DadPersistentStorage::cSerialize* pSerializer) {
	for (uint8_t i = 0; i < kNumFilters; ++i) {
		DadDSP::cBiQuad &filter = m_Filter[i];

		// Save enable state
		uint8_t enabled = m_FilterEnabled[i] ? 1 : 0;
		pSerializer->Push(enabled);

		// Save filter type
		uint8_t type = static_cast<uint8_t>(filter.getType());
		pSerializer->Push(type);

		// Save filter parameters
		float freq = filter.getCutoffFreq();
		pSerializer->Push(freq);

		float gain = filter.getGainDb();
		pSerializer->Push(gain);

		float width = filter.getBandwidth();
		pSerializer->Push(width);
	}

	m_Dirty = false;
}

// -----------------------------------------------------------------------------
// Function: Restore
// Description: Deserializes all 5 filters and re-applies them to the biquads
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::Restore(DadPersistentStorage::cSerialize* pSerializer) {

	for (uint8_t i = 0; i < kNumFilters; ++i) {
		DadDSP::cBiQuad &filter = m_Filter[i];

		// Restore enable state
		uint8_t enabled = 0;
		pSerializer->Pull(enabled);
		m_FilterFaderEnabled[i] = (enabled != 0);

		// Restore filter type
		uint8_t type = 0;
		pSerializer->Pull(type);
		filter.setType(static_cast<DadDSP::FilterType>(type));

		// Restore filter parameters
		float freq = 0.0f;
		pSerializer->Pull(freq);
		filter.setCutoffFreq(freq);

		float gain = 0.0f;
		pSerializer->Pull(gain);
		filter.setGainDb(gain);

		float width = 0.0f;
		pSerializer->Pull(width);
		filter.setBandwidth(width);

		// Recalculate biquad coefficients after fade out
		m_bCalcCoeffs = true;
	}

	// Resync the type-preview cursor on the restored selected filter
	m_PreviewTypeIndex = findTypeIndex(m_Filter[m_SelectedFilter].getType());

	m_Dirty = false;

	// Refresh the display so restored values are visible immediately
	if (m_isActive) {
		drawDynPartOffLayer();
	}

	m_Fader.TriggerFadeOut();
}

// -----------------------------------------------------------------------------
// Function: isDirty
// Description: Returns true if a filter parameter changed since the last Save()
// -----------------------------------------------------------------------------
bool cPanelOfParametricEqualizer::isDirty() {
	return m_Dirty;
}

//**********************************************************************************
// Drawing
//**********************************************************************************

// -----------------------------------------------------------------------------
// Function: drawStatPartOffLayer
// Description: Draws static elements: background and 0dB reference line
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::drawStatPartOffLayer() {
	// Draw background
	m_pStatLayer->drawFillRect(0, 0, SCREEN_WIDTH, PARAM_HEIGHT, __ThemesManager->ParameterBack);

	// Draw 0dB reference line
	uint16_t yZero = gainToY(0.0f);
	m_pStatLayer->drawLine(kGraphX, yZero, kGraphX + kGraphWidth - 1, yZero,
							dimColor(__ThemesManager->ParameterName, 0.3f));
}

// -----------------------------------------------------------------------------
// Function: drawDynPartOffLayer
// Description: Draws dynamic elements: status line, response curve, filter markers
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::drawDynPartOffLayer() {
	m_pDynLayer->eraseLayer();

	drawStatusLine();
	drawResponseCurve();
	drawFilterMarkers();
}

// -----------------------------------------------------------------------------
// Function: formatFloat1
// Description: Formats a float with 1 decimal digit into a string, without
//              relying on printf float support (not linked on H743/newlib-nano
//              builds unless -u _printf_float is forced).
// -----------------------------------------------------------------------------
static void formatFloat1(char* out, size_t outSize, float value) {
	bool neg = value < 0.0f;
	if (neg) value = -value;

	// Extract integer and fractional parts
	uint32_t intPart = static_cast<uint32_t>(value);
	uint32_t frac = static_cast<uint32_t>((value - static_cast<float>(intPart)) * 10.0f + 0.5f);
	if (frac >= 10) {
		frac -= 10;
		intPart += 1;
	}

	// Format as string without float printf support
	snprintf(out, outSize, "%s%lu.%lu",
			 neg ? "-" : "",
			 static_cast<unsigned long>(intPart),
			 static_cast<unsigned long>(frac));
}

// -----------------------------------------------------------------------------
// Function: drawStatusLine
// Description: Shows the selected filter, its applied type (plus preview if
//              different), and the parameter currently edited by Encoder 3.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::drawStatusLine() {
	DadDSP::cBiQuad &filter = m_Filter[m_SelectedFilter];
	char buf[64];
	char valueStr[16];

	// Determine current parameter and its value
	const char* paramName = "Freq";
	float paramValue = filter.getCutoffFreq();
	const char* unit = "Hz";
	if (m_SelectedParameter == eEQParameter::Gain) {
		paramName = "Gain";
		paramValue = filter.getGainDb();
		unit = "dB";
	} else if (m_SelectedParameter == eEQParameter::Width) {
		paramName = "Width";
		paramValue = filter.getBandwidth();
		unit = "";
	}

	formatFloat1(valueStr, sizeof(valueStr), paramValue);

	// Check if Encoder 2 is previewing a different type
	DadDSP::FilterType previewType = kFilterTypeList[m_PreviewTypeIndex];
	bool previewPending = (previewType != filter.getType());

	// Format type string (show preview if pending)
	char typeStr[16];
	if (previewPending) {
		snprintf(typeStr, sizeof(typeStr), "%s->%s",
				 getFilterTypeName(filter.getType()),
				 getFilterTypeName(previewType));
	} else {
		snprintf(typeStr, sizeof(typeStr), "%s", getFilterTypeName(filter.getType()));
	}

	m_pDynLayer->setFont(FONTXXS);
	m_pDynLayer->setTextFrontColor(__ThemesManager->ParameterName);

	// Encoder 1: Filter selection and enable status
	snprintf(buf, sizeof(buf), "F%d %s",
			 m_SelectedFilter + 1,
			 m_FilterFaderEnabled[m_SelectedFilter] ? "ON" : " OFF");

	uint16_t TextWidth = m_pDynLayer->getTextWidth(buf);
	m_pDynLayer->setCursor((53 - (TextWidth/2)), 2);
	m_pDynLayer->drawText(buf);

	// Encoder 2: Filter type (with preview if pending)
	snprintf(buf, sizeof(buf), "%s", typeStr);

	TextWidth = m_pDynLayer->getTextWidth(buf);
	m_pDynLayer->setCursor((160 - (TextWidth/2)), 2);
	m_pDynLayer->drawText(buf);

	// Encoder 3: Parameter name and value
	snprintf(buf, sizeof(buf), "%s %s%s",
			 paramName,
			 valueStr,
			 unit);

	TextWidth = m_pDynLayer->getTextWidth(buf);
	m_pDynLayer->setCursor((267 - (TextWidth/2)), 2);
	m_pDynLayer->drawText(buf);
}

// -----------------------------------------------------------------------------
// Function: drawResponseCurve
// Description: Draws the combined response of all active filters.
//              Since 20*log10(|H1*H2*...|) = sum(20*log10(|Hi|)), the total
//              curve in dB is simply the sum of each active filter's GainDb().
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::drawResponseCurve() {
	uint16_t prevX = kGraphX;
	uint16_t prevY = 0;
	bool prevInRange = false;

	// Iterate through each pixel column of the graph
	for (uint16_t x = 0; x < kGraphWidth; ++x) {
		float freq = xToFreq(x);
		float gain = 0.0f;

		// Sum the gain of all active filters at this frequency
		for (uint8_t i = 0; i < kNumFilters; ++i) {
			if (m_FilterFaderEnabled[i]) {
				gain += m_Filter[i].GainDb(freq);
			}
		}

		// Check if gain is within display range
		bool inRange = (gain >= kMinGain) && (gain <= kMaxGain);
		uint16_t px = kGraphX + x;
		uint16_t py = gainToY(gain);

		// Draw segment only when both ends are within range
		if (inRange && prevInRange) {
			m_pDynLayer->drawLine(prevX, prevY, px, py, __ThemesManager->ParameterCursor);
		}

		prevX = px;
		prevY = py;
		prevInRange = inRange;
	}
}

// -----------------------------------------------------------------------------
// Function: drawFilterMarkers
// Description: Draws a point on the curve for each filter at its frequency.
//              Selected filter is highlighted and shows its type above the point.
//              Disabled filters are dimmed.
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::drawFilterMarkers() {
	for (uint8_t i = 0; i < kNumFilters; ++i) {
		float freq = m_Filter[i].getCutoffFreq();

		// Calculate total gain at this frequency (including this filter)
		float gain = 0.0f;
		for (uint8_t j = 0; j < kNumFilters; ++j) {
			if (m_FilterFaderEnabled[j]) {
				gain += m_Filter[j].GainDb(freq);
			}
		}

		// Calculate marker position
		uint16_t x = kGraphX + freqToX(freq);
		uint16_t y = gainToY(gain);

		// Determine marker color (dimmed if filter is disabled)
		DadGFX::sColor color = m_FilterFaderEnabled[i]
			? __ThemesManager->ParameterName
			: dimColor(__ThemesManager->ParameterName, 0.4f);

		if (i == m_SelectedFilter) {
			color = __ThemesManager->ParameterCursor;
		}

		// Draw marker point
		m_pDynLayer->drawFillCircle(x, y, 2, color);

		// Draw filter type label above the marker (only for selected filter)
		if (i == m_SelectedFilter) {
			if(!m_FilterFaderEnabled[i]){
				color = dimColor(__ThemesManager->ParameterName, 0.8f);
			}
			m_pDynLayer->setFont(FONTXXS);
			m_pDynLayer->setTextFrontColor(color);
			const char* typeName = getFilterTypeName(m_Filter[i].getType());
			uint16_t tw = m_pDynLayer->getTextWidth(typeName);
			uint16_t textHeight = m_pDynLayer->getTextHeight();
			m_pDynLayer->setCursor(x - (tw / 2), y - 4 - textHeight);
			m_pDynLayer->drawText(typeName);
		}
	}
}

// -----------------------------------------------------------------------------
// Callback methods
// -----------------------------------------------------------------------------
void cPanelOfParametricEqualizer::FadeInEndCallback(void* Context, void* userData){

}

void cPanelOfParametricEqualizer::FadeOutEndCallback(void* Context, void* userData){
	cPanelOfParametricEqualizer* pThis = (cPanelOfParametricEqualizer*) Context;
	for (uint8_t i = 0; i < kNumFilters; ++i){
		if(pThis->m_bCalcCoeffs){
			pThis->m_Filter[i].CalculateParameters();
		}
		pThis->m_FilterEnabled[i] = pThis->m_FilterFaderEnabled[i];
	}
	pThis->m_Fader.TriggerFadeIn();
}

} // namespace DadGUI

//***End of file**************************************************************
