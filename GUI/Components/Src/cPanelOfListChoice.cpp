//==================================================================================
//==================================================================================
// File: cPanelOfListChoice.cpp
// Description:
//
// Copyright (c) 2026 DadDesign-Projects.
//==================================================================================
//==================================================================================

#include "cPanelOfListChoice.h"
#include "cDisplay.h"
#include "cMemoryManager.h"
#include "cThemesManager.h"
#include "MainGUI.h"
#include "cEncoder.h"

// *****************************************************************************
// Global variables declarations
// *****************************************************************************
extern DadGFX::cDisplay __Display;
extern DadGUI::cMainGUI __GUI;
extern DadDrivers::cEncoder	__Encoder1;
extern DadDrivers::cEncoder	__Encoder2;
extern DadDrivers::cEncoder	__Encoder3;

namespace DadGUI {
extern cThemesManager	__ThemesManager;

//**********************************************************************************
// Layer declaration
//**********************************************************************************
DECLARE_LAYER(ListChoiceStatLayer, SCREEN_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(ListChoiceDynLayer, SCREEN_WIDTH, PARAM_HEIGHT);

//**********************************************************************************
// Public methods
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initializes the class
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::Init(uint32_t SerializeID,
							  DadDSP::CallbackType Callback,
							  uint32_t CallbackUserData) {
	m_pDynLayer   = ADD_LAYER(__Display, ListChoiceDynLayer, 0, MENU_HEIGHT, 0);
	m_pStatLayer  = ADD_LAYER(__Display, ListChoiceStatLayer, 0, MENU_HEIGHT, 0);
	m_isActive     = false;
	m_ActiveItem   = 0;
	m_SelectedItem = 0;
	m_Callback = Callback;
	m_CallbackUserData = CallbackUserData;
	m_Parameter.Init(SerializeID,
			0, 0, 0,
			1, 0,
			ParameterChange,
			(uint32_t) this,
			0);
}

// ---------------------------------------------------------------------------------
// Function: Activate
// Description: Called when the component becomes active and visible
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::Activate() {
	// Adjust Z-order to bring the layers forward
	m_pStatLayer->changeZOrder(40);
	m_pDynLayer->changeZOrder(41);

	// Mark UI as active
	m_isActive = true;

	// Draw static and dynamic parts
	drawStatPartOffLayer();
	drawDynPartOffLayer();
}

// ---------------------------------------------------------------------------------
// Function: Deactivate
// Description: Called when the component is deactivated or hidden
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::Deactivate() {

	// Move layers to background
	m_pStatLayer->changeZOrder(0);
	m_pDynLayer->changeZOrder(0);

	// Mark UI as inactive
	m_isActive = false;

}

// ---------------------------------------------------------------------------------
// Function: Update
// Description: Called every frame while the component is active to refresh the display
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::Update() {
	bool redraw = m_Redraw;
	uint8_t SwitchState = __Encoder1.getSwitchState() + __Encoder2.getSwitchState() + __Encoder3.getSwitchState();
	int8_t Increment = __Encoder1.getIncrement() + __Encoder2.getIncrement() + __Encoder3.getIncrement();

	// Nothing to do if the list is empty or there is no rotation
	if (Increment != 0 && !m_List.empty()) {

		// 1. Get the size and the current index as signed values
		int32_t size = static_cast<int32_t>(m_List.size());
		int32_t nextIndex = static_cast<int32_t>(m_ActiveItem) + Increment;

		// 2. Handle bounds (wrap-around)
		if (nextIndex >= size) {
			nextIndex = 0; // Too high -> wrap to the beginning
		} else if (nextIndex < 0) {
			nextIndex = size - 1; // Negative -> wrap to the end
		}

		// 3. Store the result back into m_ActiveItem
		m_ActiveItem = static_cast<uint16_t>(nextIndex);
		redraw = true;
	}

	if (SwitchState > 0) {
		m_Parameter.setValue(m_ActiveItem);
	}

	if (redraw) {
		m_Redraw= false;
		drawDynPartOffLayer();
	}
}



// ---------------------------------------------------------------------------------
// Function: Redraw
// Description: Forces a complete redraw of static and dynamic parts
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::Redraw() {
	if (m_isActive) {
		drawStatPartOffLayer();
		drawDynPartOffLayer();
	}
}

// ---------------------------------------------------------------------------------
// Function: addItem2List
// Description: Adds a raw item to the list and updates the parameter range
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::addItem2List(const std::string& item) {
	m_List.push_back(item);
	m_Parameter.setMaxValue(m_List.size());
}

// ---------------------------------------------------------------------------------
// Function: GetItemText
// Description: Returns the formatted text with index prefix (01-Name), extension
//              stripped, and non-printable characters replaced with '_'
// ---------------------------------------------------------------------------------
std::string cPanelOfListChoice::GetItemText(uint16_t index) const {
	// 1. Bounds check
	if (index >= m_List.size()) return "";

	std::string text = m_List[index];

	// 2. Strip the suffix (extension): .wav, .doc, .xx, etc.
	size_t lastDot = text.find_last_of('.');
	if (lastDot != std::string::npos) {
		text.erase(lastDot);
	}

	// 3. Sanitize: replace any character outside [32-126] with '_'
	for (char& c : text) {
		unsigned char uc = static_cast<unsigned char>(c);
		if (uc < 32 || uc > 126) {
			c = '_';
		}
	}

	// 4. Add the indexed prefix (01-, 02-, ...)
	// (index + 1) gives a 1-based display index (1st, 2nd...)
	int displayIndex = index + 1;
	std::string prefix;
	if (displayIndex < 10) {
		prefix = "0" + std::to_string(displayIndex); // "01", "02" ... "09"
	} else {
		prefix = std::to_string(displayIndex);        // "10", "11" ...
	}

	return prefix + "-" + text;
}

// ---------------------------------------------------------------------------------
// Function: getSelectedItemText
// Description: Returns the raw text of the selected item (no index prefix)
// ---------------------------------------------------------------------------------
bool cPanelOfListChoice::getSelectedItemText(std::string& outItem) const {
	if (m_List.empty() || m_SelectedItem >= m_List.size()) {
		return false;
	}

	// Return the raw text stored in the vector
	outItem = m_List[m_SelectedItem];
	return true;
}

//**********************************************************************************
// Private methods
//**********************************************************************************
constexpr uint16_t ListOffset = 14;

// ---------------------------------------------------------------------------------
// Function: drawStatPartOffLayer
// Description: Draws static elements such as frames, labels, and background
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::drawStatPartOffLayer() {

	m_pStatLayer->drawFillRect(0, 0, SCREEN_WIDTH, PARAM_HEIGHT, __ThemesManager->ParameterBack);
	m_pStatLayer->setTextFrontColor(__ThemesManager->ParameterCursor);
	m_pStatLayer->setFont(FONTXXS);
	m_pStatLayer->setCursor(10, 5);
	m_pStatLayer->drawText("Use Enc. 1-3 to select IR. Press to confirm.");

	m_pDynLayer->setFont(FONTXS);
	uint16_t TextHeight = m_pDynLayer->getTextHeight();
	uint16_t TextBase   = ((PARAM_HEIGHT - TextHeight) / 2) + ListOffset;

	m_pStatLayer->drawFillRect(0, TextBase, SCREEN_WIDTH, TextHeight + 1, __ThemesManager->ParameterCursor);

}

// ---------------------------------------------------------------------------------
// Function: drawDynPartOffLayer
// Description: Draws dynamic elements - scrolling list centered on m_ActiveItem
// ---------------------------------------------------------------------------------
void cPanelOfListChoice::drawDynPartOffLayer() {
	constexpr uint16_t ItemOffset = 1;
	constexpr uint16_t XOffset = 20;

	m_pDynLayer->eraseLayer();
	m_pDynLayer->setFont(FONTXS);
	m_pDynLayer->setTextFrontColor(__ThemesManager->ParameterName);

	uint16_t TextHeight = m_pDynLayer->getTextHeight();
	uint16_t TextBase   = ((PARAM_HEIGHT - TextHeight) / 2) + ListOffset;
	uint16_t Step       = TextHeight + ItemOffset;
	uint16_t TextStart  = TextBase - (2 * Step);   // Position of the 1st slot (2 slots above center)

	const size_t listSize = m_List.size();

	for (int offset = -2; offset <= 2; ++offset) {
		m_pDynLayer->setCursor(XOffset, TextStart);

		if (listSize == 0) {
			m_pDynLayer->drawText("");
		}
		else if (listSize == 1) {
			// Single element -> only shown in the center slot
			if (offset == 0) {
				m_pDynLayer->drawText(GetItemText(0).c_str());
			} else {
				m_pDynLayer->drawText("");
			}
		}
		else {
			// 2 elements or more -> circular wrapping, active item always centered
			int idx = static_cast<int>(m_ActiveItem) + offset;
			idx = ((idx % static_cast<int>(listSize)) + static_cast<int>(listSize)) % static_cast<int>(listSize);
			if (idx == m_Parameter.getTargetValue()) {
				m_pDynLayer->setFont(FONTXSB);
				m_pDynLayer->drawText(GetItemText(idx).c_str());
				m_pDynLayer->setCursor(XOffset - 10, TextStart);
				m_pDynLayer->drawText("*");
				m_pDynLayer->setFont(FONTXS);
			} else {
				m_pDynLayer->drawText(GetItemText(idx).c_str());
			}
		}

		TextStart += Step;
	}
}

// -----------------------------------------------------------------------------
// Parameter change callback
// -----------------------------------------------------------------------------
void cPanelOfListChoice::ParameterChange(DadDSP::cParameter* pParameter, uint32_t Context){
	cPanelOfListChoice* pThis = (cPanelOfListChoice*) Context;

	uint16_t IndexItem = (uint16_t) pParameter->getTargetValue();

	if (IndexItem >= pThis->m_List.size()) {
		IndexItem = 0;
	}
	pThis->m_SelectedItem = IndexItem;
	pThis->m_ActiveItem   = IndexItem;
	pThis->m_Callback(pParameter, pThis->m_CallbackUserData);
	pThis->m_Redraw = true;
}

} // namespace DadGUI
//***End of file**************************************************************
