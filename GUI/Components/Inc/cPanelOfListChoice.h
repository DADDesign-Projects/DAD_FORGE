//==================================================================================
//==================================================================================
// File: cPanelOfListChoice.h
// Description: Displays a scrollable list of items and lets the
//              user select one via the encoders
//
// Copyright (c) 2026 DadDesign-Projects.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "iUIComponent.h"
#include "cUIParameter.h"
#include "cDisplay.h"
#include <string>
#include <vector>

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfListChoice
// Description: Displays a scrollable list of items (e.g. IR files) and lets the
//              user select one via the encoders
//**********************************************************************************
class cPanelOfListChoice : public iUIComponent {
public:
	virtual ~cPanelOfListChoice() = default;

	// ---------------------------------------------------------------------------------
	// Function: Init
	// Description: Initializes the display layers and the selection parameter
	// ---------------------------------------------------------------------------------
	void Init(uint32_t SerializeID, DadDSP::CallbackType Callback = nullptr, uint32_t CallbackUserData = 0);

	// ---------------------------------------------------------------------------------
	// Function: Activate
	// Description: Called when the component becomes active and visible
	// ---------------------------------------------------------------------------------
	void Activate() override;

	// ---------------------------------------------------------------------------------
	// Function: Deactivate
	// Description: Called when the component is deactivated or hidden
	// ---------------------------------------------------------------------------------
	void Deactivate() override;

	// ---------------------------------------------------------------------------------
	// Function: Update
	// Description: Called every frame while the component is active to refresh the display
	// ---------------------------------------------------------------------------------
	void Update() override;

	// ---------------------------------------------------------------------------------
	// Function: Redraw
	// Description: Forces a full redraw of the static and dynamic elements
	// ---------------------------------------------------------------------------------
	void Redraw() override;

	// ---------------------------------------------------------------------------------
	// Function: addItem2List
	// Description: Adds a raw item to the list and updates the parameter range
	// ---------------------------------------------------------------------------------
	void addItem2List(const std::string &Item);

	// ---------------------------------------------------------------------------------
	// Function: getSelectedItemIndex
	// Description: Returns the Index of the currently selected item (no index prefix)
	// ---------------------------------------------------------------------------------
	uint16_t getSelectedItemIndex() const{
		return m_SelectedItem;
	}

	// ---------------------------------------------------------------------------------
	// Function: getSelectedItemText
	// Description: Returns the raw text of the currently selected item (no index prefix)
	// ---------------------------------------------------------------------------------
	bool getSelectedItemText(std::string& outItem) const;

	// ---------------------------------------------------------------------------------
	// Function: GetItemText
	// Description: Returns the formatted text for an item (indexed, extension stripped)
	// ---------------------------------------------------------------------------------
	std::string GetItemText(uint16_t index) const;

protected:
	// ---------------------------------------------------------------------------------
	// Function: drawStatPartOffLayer
	// Description: Draws static parts (frame, help text, cursor background)
	// ---------------------------------------------------------------------------------
	void drawStatPartOffLayer();

	// ---------------------------------------------------------------------------------
	// Function: drawDynPartOffLayer
	// Description: Draws dynamic parts (scrolling list centered on the active item)
	// ---------------------------------------------------------------------------------
	void drawDynPartOffLayer();

	// -----------------------------------------------------------------------------
	// Parameter change callback
	// -----------------------------------------------------------------------------
	static void ParameterChange(DadDSP::cParameter* pParameter, uint32_t Context);

	// Member variables
	DadGFX::cLayer*    		m_pDynLayer;	// Pointer to the dynamic display layer
	DadGFX::cLayer*    		m_pStatLayer;   // Pointer to the static display layer
	bool               		m_isActive;		// Indicates whether the UI component is active

	std::vector<std::string> m_List;			// Raw item list (filenames)
	uint16_t				 m_ActiveItem;		// Index currently highlighted by the encoders
	uint16_t				 m_SelectedItem;	// Index confirmed by the parameter (switch press)

	cUIParameter			 m_Parameter;		// Selection parameter, range synced to m_List size

	DadDSP::CallbackType 	m_Callback;
	uint32_t 				m_CallbackUserData;
	bool					m_Redraw;
};

} // namespace DadGUI
//***End of file**************************************************************
