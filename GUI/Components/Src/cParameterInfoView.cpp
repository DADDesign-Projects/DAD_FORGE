//==================================================================================
//==================================================================================
// File: ParameterInfoViews.cpp
// Description: Parameter information temporary overlay tooltip windows implementation.
//              Display duration: PARAM_INFO_TIME_MS seconds from last parameter interaction.
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================
#include "cParameterInfoView.h"
namespace DadGUI {

//**********************************************************************************
// cParameterInfoView implementation
//**********************************************************************************


DECLARE_LAYER(ParamInfoLayer, SCREEN_WIDTH, INFO_HEIGHT);
// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the info parameter view and allocate layer
// ---------------------------------------------------------------------------------
void cParameterInfoView::Init() {
    // Info layer for temporary overlay to parameter display
    m_pParamInfoLayer  = ADD_LAYER(__Display, ParamInfoLayer, 0, MENU_HEIGHT + PARAM_HEIGHT, 0);
    m_pParamInfoLayer->changeZOrder(0);
    m_pParamInfoLayer->eraseLayer();

    // Reset the display counter
    m_InfoViewTimeCounter = 0;

    // Register callback for parameter change notification
    __GUI.RegisterParameterListener(ParameterChange, (uint32_t) this);
    __GUI_EventManager.Subscribe_Update(this, 0);
}

// ---------------------------------------------------------------------------------
// Function: ShowParamView
// Description: Show a temporary banner with parameter name and value
// ---------------------------------------------------------------------------------
#define NAME_OFFSET 1
void cParameterInfoView::ShowParamView(const std::string Name, const std::string Value) {
    const uint16_t xCenterView = m_pParamInfoLayer->getWith() / 2;

    m_pParamInfoLayer->changeZOrder(41); // Bring to foreground
    m_pParamInfoLayer->eraseLayer(__pActivePalette->ParamInfoBack);

    // Parameter name (small font)
    m_pParamInfoLayer->setFont(FONTL);
    uint16_t NameWidth = m_pParamInfoLayer->getTextWidth(Name.c_str());
    m_pParamInfoLayer->setCursor(xCenterView - (NameWidth / 2), NAME_OFFSET);
    m_pParamInfoLayer->setTextFrontColor(__pActivePalette->ParamInfoName);
    m_pParamInfoLayer->drawText(Name.c_str());

    // Parameter value (large font)
    uint16_t NameHeight = m_pParamInfoLayer->getTextHeight() + NAME_OFFSET;
    m_pParamInfoLayer->setFont(FONTXL);
    uint16_t ValueWidth = m_pParamInfoLayer->getTextWidth(Value.c_str());
    m_pParamInfoLayer->setCursor(xCenterView - (ValueWidth / 2), NameHeight + 1);
    m_pParamInfoLayer->setTextFrontColor(__pActivePalette->ParamInfoValue);
    m_pParamInfoLayer->drawText(Value.c_str());

    m_InfoViewTimeCounter = PARAMETER_INFO_VIEW_DISPLAY_TIME_MS; // Set counter to display time (ms)
}

// ---------------------------------------------------------------------------------
// Function: HideParamView
// Description: Hide the temporary info banner (reset z-order)
// ---------------------------------------------------------------------------------
void cParameterInfoView::HideParamView() {
	m_pParamInfoLayer->changeZOrder(0); // Send to background
}

// ---------------------------------------------------------------------------------
// Function: on_GUI_Update
// Update GUI Object time GUI_UPDATE_MS in __ms__
// ---------------------------------------------------------------------------------
void cParameterInfoView::on_GUI_Update(){
	if(m_InfoViewTimeCounter > 0){
		m_InfoViewTimeCounter -= GUI_UPDATE_MS;
		if(m_InfoViewTimeCounter <= 0){
			m_InfoViewTimeCounter = 0;
			HideParamView();
		}
	}
}

// ---------------------------------------------------------------------------------
// Parameter change callback
// ---------------------------------------------------------------------------------
void cParameterInfoView::ParameterChange(void* pParameter, uint32_t Context){
	cParameterInfoView* pThis = (cParameterInfoView*)Context;
	cParameterView* pParameterView = (cParameterView*) pParameter;
	pThis->ShowParamView(pParameterView->getInfoName() , pParameterView->getInfoValue());
}

}// namespace DadGUI
//***End of file**************************************************************
