//==================================================================================
//==================================================================================
// File: cInfoView.cpp
// Description: Implementation of the cInfoView class for displaying system information
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cInfoView.h"

namespace DadGUI {

//**********************************************************************************
// Layer declaration for info view display
//**********************************************************************************
DECLARE_LAYER(InfoLayer, SCREEN_WIDTH, INFO_HEIGHT);

//**********************************************************************************
// cInfoView class implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialization section
// -----------------------------------------------------------------------------

//==================================================================================
// Initialize internal members and layer references
//==================================================================================
void cInfoView::Init() {
	m_pInfoLayer  = ADD_LAYER(__Display, InfoLayer, 0, MENU_HEIGHT + PARAM_HEIGHT, 0);  // Create display layer
	m_MemSlot     = MAX_SLOT + 1;                                                       // Initialize memory slot
	m_MemDirty    = false;                                                              // Initialize dirty flag
	m_MemState    = eOnOff::ByPass;                                                     // Initialize memory state
	m_isActive	  = false;                                                              // Initialize active state
}

// -----------------------------------------------------------------------------
// Public methods section
// -----------------------------------------------------------------------------

//==================================================================================
// Called when this component becomes active (brought to foreground)
//==================================================================================
void cInfoView::Activate() {
	m_isActive = true;                                             // Set active flag
	m_pInfoLayer->changeZOrder(10);                                // Bring to foreground
	m_MemState = eOnOff::ByPass;                                   // Set default state
	ShowView(false, __MemoryManager.getActiveSlot() + 1, "BYPASS"); // Display initial view
}

//==================================================================================
// Called when this component is deactivated (background or hidden)
//==================================================================================
void cInfoView::Deactivate() {
	m_isActive	  = false;                    // Clear active flag
	m_pInfoLayer->changeZOrder(0);            // Send to background
}

//==================================================================================
// Update display state every frame/tick while the component is active
//==================================================================================
void cInfoView::Update() {
	if(m_isActive == false) return;  // Skip update if not active

	bool Dirty = __GUI_EventManager.sendEventToActive_SerializeIsDirty();  // Check if parameters have been modified

	// Check if any monitored state (slot, dirty, On/Off) has changed
	if ((m_MemState != __MemOnOff) ||
		(m_MemSlot != __MemoryManager.getActiveSlot()) ||
		(m_MemDirty != Dirty)) {

		std::string State;  // String representation of current state

		// Map current memory state to string representation
		switch (__MemOnOff) {
			case eOnOff::ByPass:
				State = "BYPASS";
				m_MemState = eOnOff::ByPass;
				break;
			case eOnOff::Off:
				State = "OFF";
				m_MemState = eOnOff::Off;
				break;
			case eOnOff::On:
				State = "ON";
				m_MemState = eOnOff::On;
				break;
		}

		// Update memory display with new state
		m_MemDirty = Dirty;                                // Update dirty flag
		m_MemSlot  = __MemoryManager.getActiveSlot();      // Update current slot
		ShowView(Dirty, m_MemSlot + 1, State);            // Refresh display
	}
}

//==================================================================================
// Force to redraw UI component
//==================================================================================
void cInfoView::Redraw(){
	if(m_isActive){
		std::string State;  // String representation of current state

		// Determine current state string
		switch (__MemOnOff) {
			case eOnOff::ByPass:
				State = "BYPASS";
				break;
			case eOnOff::Off:
				State = "OFF";
				break;
			case eOnOff::On:
				State = "ON";
				break;
		}
		ShowView(m_MemDirty, m_MemSlot + 1, State);  // Refresh display with current state
	}
}

// -----------------------------------------------------------------------------
// Private methods section
// -----------------------------------------------------------------------------

// Layout constants for text placement
#define MEM_OFFSET    64      // Vertical offset for memory label
#define NUM_OFFSET    10     // Vertical offset for memory number
#define CENTER_MEM    65     // Horizontal center for memory display
#define CENTER_STATE  220    // Horizontal center for state display

//==================================================================================
// Draw the full info view content: memory slot, state, and dirty flag
//==================================================================================
void cInfoView::ShowView(bool isDirty, uint8_t MemSlot, const std::string State) {
	const uint16_t yCenterView = m_pInfoLayer->getHeight() / 2;  // Calculate vertical center

	m_pInfoLayer->changeZOrder(40);                                     // Ensure proper z-order
	m_pInfoLayer->eraseLayer(__pActivePalette->MainInfoBack);           // Clear background

	//------------------------
	// Memory label
	//------------------------
	m_pInfoLayer->setFont(FONTXSB);                                      // Set font for label
	uint16_t MemWidth = m_pInfoLayer->getTextWidth("Memory");           // Calculate text width
	m_pInfoLayer->setCursor(CENTER_MEM - (MemWidth / 2) + 2, MEM_OFFSET);   // Position cursor
	m_pInfoLayer->setTextFrontColor(__pActivePalette->MainInfoMem);     // Set text color
	m_pInfoLayer->drawText("Memory");                                   // Draw memory label

	//------------------------
	// Memory number
	//------------------------
	std::string NumMem = std::to_string(MemSlot);						// Convert slot to string
	m_pInfoLayer->setFont(FONTXXXLB);
	uint16_t MemHeight = m_pInfoLayer->getTextHeight();                 // Get text height
	uint16_t NumWidth = m_pInfoLayer->getTextWidth(NumMem.c_str());     // Calculate number width
	m_pInfoLayer->setCursor(CENTER_MEM - (NumWidth / 2), yCenterView - (MemHeight/2) - NUM_OFFSET);  // Position cursor
	m_pInfoLayer->setTextFrontColor(__pActivePalette->MainInfoMem);     // Set text color
	m_pInfoLayer->drawText(NumMem.c_str());                             // Draw memory number

	//------------------------
	// "Dirty" indicator
	//------------------------
	if (isDirty) {
		std::string Dirty = "Dirty";                                    // Dirty indicator text
		m_pInfoLayer->setFont(FONTXXSB);                                // Set small font
		m_pInfoLayer->setCursorOffset(6, 4);                            // Position offset
		m_pInfoLayer->setTextFrontColor(__pActivePalette->MainInfoDirty); // Set dirty color
		m_pInfoLayer->drawText(Dirty.c_str());                          // Draw dirty indicator
	}

	//------------------------
	// State label ("ON", "OFF", "BYPASS")
	//------------------------
	m_pInfoLayer->setFont(FONTXXXLB);                                   // Set largest font
	uint16_t StateWidth = m_pInfoLayer->getTextWidth(State.c_str());    // Calculate state width

	// Auto-adjust font size if the state string is too wide
	if (StateWidth > 150) {
		m_pInfoLayer->setFont(FONTXXLB);                                // Reduce font size
		StateWidth = m_pInfoLayer->getTextWidth(State.c_str());         // Recalculate width
	}
	if (StateWidth > 150) {
		m_pInfoLayer->setFont(FONTXLB);                                 // Further reduce font size
		StateWidth = m_pInfoLayer->getTextWidth(State.c_str());         // Recalculate width
	}

	uint16_t StateHeight = m_pInfoLayer->getTextHeight();               // Get state text height
	m_pInfoLayer->setCursor(CENTER_STATE - (StateWidth / 2), yCenterView - (StateHeight / 2));  // Center position
	m_pInfoLayer->setTextFrontColor(__pActivePalette->MainInfoState);   // Set state color
	m_pInfoLayer->drawText(State.c_str());                              // Draw state text
}

} // namespace DadGUI

//***End of file**************************************************************
