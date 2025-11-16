//==================================================================================
//==================================================================================
// File: cUIMemory.cpp
// Description: Implementation of memory management UI component
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUIMemory.h"
#include "HardwareAndCo.h"

namespace DadGUI {

//**********************************************************************************
// Layers declaration
//**********************************************************************************
DECLARE_LAYER(DynMemoryLayer, SCREEN_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(StatMemoryLayer, SCREEN_WIDTH, PARAM_HEIGHT);

// =============================================================================
// Initialize
// =============================================================================
void cUIMemory::Init(uint32_t SerializeID) {
	m_SerializeID  = SerializeID;                       // Serialization ID for state save/restore
	m_isActive     = false;                             // Initially inactive
	m_MemorySlot   = __MemoryManager.getActiveSlot();   // Get current active memory slot
	m_MemChoice    = eMemChoice::No;                    // Default user choice
	m_MemState     = eMemState::Save;                   // Default memory operation
	m_ActionExec   = 0;                                 // No action executed

	// Attach display layers
	m_pDynMemoryLayer  = ADD_LAYER(__Display, DynMemoryLayer,  0, MENU_HEIGHT, 0);
	m_pStatMemoryLayer = ADD_LAYER(__Display, StatMemoryLayer, 0, MENU_HEIGHT, 0);
}

// =============================================================================
// Activate
// =============================================================================
void cUIMemory::Activate() {
	m_isActive = true;                                  // Set UI as active
	m_pStatMemoryLayer->changeZOrder(41);               // Bring static layer to front
	m_pDynMemoryLayer->changeZOrder(40);                // Position dynamic layer behind
	m_MemorySlot = __MemoryManager.getActiveSlot();     // Update active memory slot
	drawStatLayer();                                    // Draw static UI elements
	drawDynLayer();                                     // Draw dynamic UI elements
}

// =============================================================================
// Deactivate
// =============================================================================
void cUIMemory::Deactivate() {
	m_isActive = false;                                 // Set UI as inactive
	m_MemChoice = eMemChoice::No;                       // Reset user choice
	m_pStatMemoryLayer->changeZOrder(0);                // Reset static layer Z-order
	m_pDynMemoryLayer->changeZOrder(0);                 // Reset dynamic layer Z-order
}

// =============================================================================
// Update
// Description: Handles user input, updates UI state, and triggers memory operations
// =============================================================================
void cUIMemory::Update() {
	if (!m_isActive) return;                            // Skip if UI inactive
	bool Redraw = false;

	// -----------------------------------------------------------------------------
	// Encoder 1: Change memory operation (Save / Delete / Load)
	// -----------------------------------------------------------------------------
	int8_t Increment = __Encoder1.getIncrement();
	if (Increment != 0) {
		// Rotate through memory operation states
		if (Increment > 0) {
			// Forward rotation
			switch (m_MemState) {
				case eMemState::Save:   m_MemState = eMemState::Delete; break;
				case eMemState::Delete: m_MemState = eMemState::Load;   break;
				case eMemState::Load:   m_MemState = eMemState::Save;   break;
			}
		} else {
			// Backward rotation
			switch (m_MemState) {
				case eMemState::Save:   m_MemState = eMemState::Load;   break;
				case eMemState::Delete: m_MemState = eMemState::Save;   break;
				case eMemState::Load:   m_MemState = eMemState::Delete; break;
			}
		}
		Redraw = true;
	}

	// -----------------------------------------------------------------------------
	// Encoder 2: Change memory slot
	// -----------------------------------------------------------------------------
	Increment = __Encoder2.getIncrement();
	if (Increment != 0) {
		m_MemorySlot += Increment;
		// Clamp memory slot between 0 and 9
		if (m_MemorySlot > 9) m_MemorySlot = 9;
		if (m_MemorySlot < 0) m_MemorySlot = 0;
		Redraw = true;
	}

	// -----------------------------------------------------------------------------
	// Encoder 3: Toggle user choice (Yes / No)
	// -----------------------------------------------------------------------------
	Increment = __Encoder3.getIncrement();
	if (Increment != 0) {
		m_MemChoice = (m_MemChoice == eMemChoice::No) ? eMemChoice::Yes : eMemChoice::No;
		Redraw = true;
	}

	// -----------------------------------------------------------------------------
	// Update "Yes/No" enable state based on memory availability
	// -----------------------------------------------------------------------------
	if (Redraw) {
	    bool shouldEnable = false;

	    // Determine if choice should be enabled based on current operation
	    switch (m_MemState) {
	        case eMemState::Save:   shouldEnable = true; break;
	        case eMemState::Delete: shouldEnable = __MemoryManager.isErasable(m_MemorySlot); break;
	        case eMemState::Load:   shouldEnable = __MemoryManager.isLoadable(m_MemorySlot); break;
	    }

	    // Update choice state
	    if (shouldEnable && m_MemChoice == eMemChoice::disabled)
	        m_MemChoice = eMemChoice::No;
	    else if (!shouldEnable)
	        m_MemChoice = eMemChoice::disabled;
	}

	// -----------------------------------------------------------------------------
	// Encoder 0 (button): Execute the selected action
	// -----------------------------------------------------------------------------
	uint8_t SwitchState = __Encoder0.getSwitchState();
	if (SwitchState == 1) {
		// Execute action only if not already executed and user confirmed with Yes
		if ((m_ActionExec == 0) && (m_MemChoice == eMemChoice::Yes)) {
			// Perform the selected memory operation
			switch (m_MemState) {
				case eMemState::Save:   __MemoryManager.SaveSlot(m_MemorySlot, m_SerializeID); break;
				case eMemState::Delete: __MemoryManager.ErraseSlot(m_MemorySlot); break;
				case eMemState::Load:   __MemoryManager.RestoreSlot(m_MemorySlot); break;
			}
			m_MemChoice = eMemChoice::No;  // Reset choice after execution
			Redraw = true;
		}
		m_ActionExec = 1;  // Mark action as executed
	} else {
		m_ActionExec = 0;  // Reset action execution flag
	}

	// Redraw dynamic layer if any changes occurred
	if (Redraw)
		drawDynLayer();
}

//**********************************************************************************
// Constants for layout positioning
//**********************************************************************************
constexpr uint16_t	MidYYexNo     = (PARAM_HEIGHT / 2);  // Vertical center for Yes/No buttons
constexpr uint16_t	MidXYesNo     = 265;                 // Horizontal center for Yes/No buttons
constexpr uint16_t	MidXFunction  = 65;                  // Horizontal center for function text
constexpr uint16_t	MidXSlot      = 175;                 // Horizontal center for slot number
constexpr uint16_t	Margin        = 6;                   // General margin
constexpr uint16_t 	TotalMargin   = Margin * 2;          // Total margin (both sides)
constexpr uint16_t  YesNoYOffset  = 4;                   // Vertical offset for Yes/No buttons
constexpr const char* YesText     = "Yes";               // Yes button text
constexpr const char* NoText      = "No";                // No button text

// =============================================================================
// Draw static layer
// Description: Draws permanent UI elements (buttons outlines, text labels)
// =============================================================================
void cUIMemory::drawStatLayer() {
	m_pStatMemoryLayer->eraseLayer();

	// Configure text properties
	m_pStatMemoryLayer->setFont(FONTL);
	m_pStatMemoryLayer->setTextFrontColor(__pActivePalette->MemViewText);

	// Calculate button dimensions
	uint16_t YesNoHeight   = m_pStatMemoryLayer->getTextHeight();
	uint16_t YesWidth      = m_pStatMemoryLayer->getTextWidth(YesText);
	uint16_t HalfYesWidth  = YesWidth / 2;
	uint16_t HalfNoWidth   = m_pStatMemoryLayer->getTextWidth(NoText) / 2;

	// Draw "Yes" button outline
	m_pStatMemoryLayer->drawRect(
		MidXYesNo - HalfYesWidth - Margin,
		MidYYexNo - YesNoYOffset - YesNoHeight - TotalMargin,
		YesWidth + TotalMargin,
		YesNoHeight + TotalMargin,
		2, __pActivePalette->MemViewLine);

	// Draw "Yes" text
	m_pStatMemoryLayer->setCursor(
		MidXYesNo - HalfYesWidth,
		MidYYexNo - YesNoYOffset - YesNoHeight - Margin);
	m_pStatMemoryLayer->drawText(YesText);

	// Draw "No" button outline
	m_pStatMemoryLayer->drawRect(
		MidXYesNo - HalfYesWidth - Margin,
		MidYYexNo + YesNoYOffset,
		YesWidth + TotalMargin,
		YesNoHeight + TotalMargin,
		2, __pActivePalette->MemViewLine);

	// Draw "No" text
	m_pStatMemoryLayer->setCursor(
		MidXYesNo - HalfNoWidth,
		MidYYexNo + YesNoYOffset + Margin);
	m_pStatMemoryLayer->drawText(NoText);
}

// =============================================================================
// Draw dynamic layer
// Description: Draws dynamic UI elements (current state, slot, highlights)
// =============================================================================
void cUIMemory::drawDynLayer() {
	// Clear layer with background color
	m_pDynMemoryLayer->eraseLayer(__pActivePalette->MemViewBack);
	m_pDynMemoryLayer->setTextFrontColor(__pActivePalette->MemViewText);

	// -----------------------------------------------------------------------------
	// Display function label (Save / Delete / Load)
	// -----------------------------------------------------------------------------
	char Buffer[10];
	switch (m_MemState) {
		case eMemState::Save:   snprintf(Buffer, sizeof(Buffer), "Save");   break;
		case eMemState::Delete: snprintf(Buffer, sizeof(Buffer), "Delete"); break;
		case eMemState::Load:   snprintf(Buffer, sizeof(Buffer), "Load");   break;
	}

	// Center and draw function text
	m_pDynMemoryLayer->setFont(FONTXL);
	uint16_t HalfTextHeight = m_pDynMemoryLayer->getTextHeight() / 2;
	uint16_t HalfTextWidth  = m_pDynMemoryLayer->getTextWidth(Buffer) / 2;
	m_pDynMemoryLayer->setCursor(MidXFunction - HalfTextWidth, MidYYexNo - 10 - HalfTextHeight);
	m_pDynMemoryLayer->drawText(Buffer);

	// -----------------------------------------------------------------------------
	// Display slot number
	// -----------------------------------------------------------------------------
	std::string Slot = std::to_string(m_MemorySlot + 1);
	m_pDynMemoryLayer->setFont(FONTXXLB);
	HalfTextHeight = m_pDynMemoryLayer->getTextHeight() / 2;
	HalfTextWidth  = m_pDynMemoryLayer->getTextWidth(Slot.c_str()) / 2;
	m_pDynMemoryLayer->setCursor(MidXSlot - HalfTextWidth, MidYYexNo - 10 - HalfTextHeight);
	m_pDynMemoryLayer->drawText(Slot.c_str());

	// -----------------------------------------------------------------------------
	// Highlight active choice (Yes / No)
	// -----------------------------------------------------------------------------
	m_pDynMemoryLayer->setFont(FONTLB);
	uint16_t YesNoHeight  = m_pDynMemoryLayer->getTextHeight();
	uint16_t YesWidth     = m_pDynMemoryLayer->getTextWidth(YesText);
	uint16_t HalfYesWidth = YesWidth / 2;

	// Highlight Yes button if selected
	if (m_MemChoice == eMemChoice::Yes) {
		m_pDynMemoryLayer->drawFillRect(
			MidXYesNo - HalfYesWidth - Margin,
			MidYYexNo - YesNoYOffset - YesNoHeight - TotalMargin,
			YesWidth + TotalMargin,
			YesNoHeight + TotalMargin,
			__pActivePalette->MemViewActive);
	}
	// Highlight No button if selected
	else if (m_MemChoice == eMemChoice::No) {
		m_pDynMemoryLayer->drawFillRect(
			MidXYesNo - HalfYesWidth - Margin,
			MidYYexNo + YesNoYOffset,
			YesWidth + TotalMargin,
			YesNoHeight + TotalMargin,
			__pActivePalette->MemViewActive);
	}

	// -----------------------------------------------------------------------------
	// Display action hints when "Yes" is selected
	// -----------------------------------------------------------------------------
	if (m_MemChoice == eMemChoice::Yes) {
		std::string String1 = "Press the Menu button";
		std::string String2;

		// Set appropriate hint text based on current operation
		switch (m_MemState) {
			case eMemState::Save:   String2 = "to save the memory";  break;
			case eMemState::Delete: String2 = "to clear the memory"; break;
			case eMemState::Load:   String2 = "to load the memory";  break;
		}

		// Draw hint text at bottom of screen
		m_pDynMemoryLayer->setFont(FONTXS);
		uint16_t FontHeight = m_pDynMemoryLayer->getTextHeight();
		m_pDynMemoryLayer->setCursor(5, PARAM_HEIGHT - (2 * FontHeight) - 7);
		m_pDynMemoryLayer->drawText(String1.c_str());
		m_pDynMemoryLayer->setCursor(5, PARAM_HEIGHT - FontHeight - 5);
		m_pDynMemoryLayer->drawText(String2.c_str());
	}
}

// =============================================================================
// Redraw
// Description: Force redraw of UI component
// =============================================================================
void cUIMemory::Redraw(){
	if(m_isActive){
		drawStatLayer();  // Redraw static elements
		drawDynLayer();   // Redraw dynamic elements
	}
}

} // namespace DadGUI

//***End of file**************************************************************
