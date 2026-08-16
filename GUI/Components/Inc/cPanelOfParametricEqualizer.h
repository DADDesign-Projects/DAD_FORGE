// ======================================================================
// ======================================================================
// File: cPanelOfParametricEqualizer.h
// Description: 5-band parametric EQ panel. Displays the combined frequency
//              response of 5 cBiQuad filters and lets the user edit each of
//              them (type, frequency, gain, width) via the 3 encoders.
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

#pragma once

//**********************************************************************************
// Includes
//**********************************************************************************
#include "main.h"
#include "iUIComponent.h"
#include "BiquadFilter.h"
#include "GUI_Event.h"
#include "Serialize.h"
#include "cFadeInOut.h"
#include <cstdint>

namespace DadGFX {
	class cLayer;
}

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfParametricEqualizer
// Description: Panel managing 5 parametric-EQ biquad filters (cBiQuad) and
//              displaying their combined frequency response.
//**********************************************************************************
class cPanelOfParametricEqualizer : public iUIComponent,
                                    public iGUI_EventListener,
									public DadPersistentStorage::cSerializedObject {
public:
    // -----------------------------------------------------------------------------
    // Constructor / Destructor
    // -----------------------------------------------------------------------------
	virtual ~cPanelOfParametricEqualizer() = default;

	// Number of managed filters
	static constexpr uint8_t kNumFilters = 5;

	// Parameter currently selectable/editable with Encoder 3
	enum class eEQParameter : uint8_t {
		Frequency = 0,      // Filter cutoff frequency
		Gain,               // Filter gain in dB
		Width               // Filter bandwidth / Q factor
	};

	// -----------------------------------------------------------------------------
	// Initialization
	// -----------------------------------------------------------------------------
	void Init(uint32_t SerializeID);

	// -----------------------------------------------------------------------------
	// UI Component lifecycle
	// -----------------------------------------------------------------------------
	void Activate() override;       // Called when component becomes active/visible
	void Deactivate() override;     // Called when component is deactivated/hidden
	void Update() override;         // Called every frame while active
	void Redraw() override;         // Forces a full redraw

	// -----------------------------------------------------------------------------
	// Audio processing
	// -----------------------------------------------------------------------------
	void on_GUI_RT_ProcessOut(AudioBuffer *pInOut) override;

	// -----------------------------------------------------------------------------
	// Persistence
	// -----------------------------------------------------------------------------
	void Save(DadPersistentStorage::cSerialize* pSerializer) override;
	void Restore(DadPersistentStorage::cSerialize* pSerializer) override;
	bool isDirty() override;

protected:
	// -----------------------------------------------------------------------------
	// Drawing methods
	// -----------------------------------------------------------------------------
	void drawStatPartOffLayer();    // Draw static elements (background, reference line)
	void drawDynPartOffLayer();     // Draw dynamic elements (curve, markers, status)
	void drawStatusLine();          // Draw the status line with filter info
	void drawResponseCurve();       // Draw the combined frequency response
	void drawFilterMarkers();       // Draw filter markers on the curve

	// -----------------------------------------------------------------------------
	// Encoder handling
	// -----------------------------------------------------------------------------
	void ProcessEncoder1(bool &redraw);   // Rotation: select filter. Press: toggle DSP on/off
	void ProcessEncoder2(bool &redraw);   // Rotation: preview filter type. Press: apply type
	void ProcessEncoder3(bool &redraw);   // Rotation: edit parameter. Press: cycle parameter

	// -----------------------------------------------------------------------------
	// Helper methods
	// -----------------------------------------------------------------------------
	void applyParameterIncrement(int8_t steps);   // Apply encoder step to selected parameter
	static const char* getFilterTypeName(DadDSP::FilterType type);   // Get short type name

	// -----------------------------------------------------------------------------
	// Callback methods
	// -----------------------------------------------------------------------------
	static void FadeInEndCallback(void* Context, void* userData);
	static void FadeOutEndCallback(void* Context, void* userData);

	// =============================================================================
	// Member variables
	// =============================================================================

	// Display layers
	DadGFX::cLayer*   m_pDynLayer;          // Dynamic layer: curve, markers, status line
	DadGFX::cLayer*   m_pStatLayer;         // Static layer: background, axis
	bool              m_isActive;           // True while the panel is active/visible

	// Persistence
	uint32_t          m_SerializeID;        // Serialization ID for NVM storage

	// Per-filter DSP engine and enable state
	DadDSP::cBiQuad   m_Filter[kNumFilters];      		// 5 biquad filters
	bool              m_FilterEnabled[kNumFilters];   	// Filter enable flags
	bool              m_FilterFaderEnabled[kNumFilters];// Filter enable flags
	DadDSP::cFadeInOut m_Fader;							// Fader for filter change
    bool			   m_bCalcCoeffs;

	// UI navigation / edition state (applies to currently selected filter)
	uint8_t           m_SelectedFilter;      // 0..kNumFilters-1
	uint8_t           m_PreviewTypeIndex;    // Index in type rotation list (previewed but not applied)
	eEQParameter      m_SelectedParameter;   // Parameter edited in real time by Encoder 3

	// Encoder switch edge detection
	uint8_t           m_PrevSwitch1;         // Previous state of Encoder 1 switch
	uint8_t           m_PrevSwitch2;         // Previous state of Encoder 2 switch
	uint8_t           m_PrevSwitch3;         // Previous state of Encoder 3 switch

	// Dirty flag for persistence
	bool 			  m_Dirty;               // True if parameters changed since last Save()
};

} // namespace DadGUI

//***End of file**************************************************************
