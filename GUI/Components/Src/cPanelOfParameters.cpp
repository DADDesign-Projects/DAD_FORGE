//==================================================================================
//==================================================================================
// File: cPanelOfParameters.cpp
// Description: Parameters panel implementation for GUI
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================
#include "cPanelOfParameters.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfParameterView
// Description: Manages a panel composed of three parameter views and their rendering layers
//**********************************************************************************

// Layers declaration (SDRAM allocation)
DECLARE_LAYER(Parameter1LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter1LayerStat, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter2LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter2LayerStat, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter3LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter3LayerStat, PARAM_WIDTH, PARAM_HEIGHT);

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the three parameter views and allocate their layers
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Init(cParameterView* pParameter1, cParameterView* pParameter2, cParameterView* pParameter3) {
    // Initialize internal state
    m_isActive = false;

    // Parameter view pointers
    m_pParameter1 = pParameter1;
    m_pParameter2 = pParameter2;
    m_pParameter3 = pParameter3;

    // Allocate and initialize parameter layers
    m_pParameter1LayerDyn  = ADD_LAYER(__Display, Parameter1LayerDyn, 0, MENU_HEIGHT, 0);
    m_pParameter1LayerDyn->changeZOrder(0);
    m_pParameter1LayerDyn->eraseLayer();
    m_pParameter1LayerStat = ADD_LAYER(__Display, Parameter1LayerStat, 0, MENU_HEIGHT, 0);
    m_pParameter1LayerStat->changeZOrder(0);
    m_pParameter1LayerStat->eraseLayer();

    m_pParameter2LayerDyn  = ADD_LAYER(__Display, Parameter2LayerDyn, PARAM_WIDTH, MENU_HEIGHT, 0);
    m_pParameter2LayerDyn->changeZOrder(0);
    m_pParameter2LayerDyn->eraseLayer();
    m_pParameter2LayerStat = ADD_LAYER(__Display, Parameter2LayerStat, PARAM_WIDTH, MENU_HEIGHT, 0);
    m_pParameter2LayerStat->changeZOrder(0);
    m_pParameter2LayerStat->eraseLayer();

    m_pParameter3LayerDyn  = ADD_LAYER(__Display, Parameter3LayerDyn, PARAM_WIDTH * 2, MENU_HEIGHT, 0);
    m_pParameter3LayerDyn->changeZOrder(0);
    m_pParameter3LayerDyn->eraseLayer();
    m_pParameter3LayerStat = ADD_LAYER(__Display, Parameter3LayerStat, PARAM_WIDTH * 2, MENU_HEIGHT, 0);
    m_pParameter3LayerStat->changeZOrder(0);
    m_pParameter3LayerStat->eraseLayer();

}

// ---------------------------------------------------------------------------------
// Function: Activate
// Description: Called when the panel component becomes active
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Activate() {
    m_isActive = true;

    // Draw parameter 1 if exists
    if (m_pParameter1) {
        m_pParameter1->Draw(1, m_pParameter1LayerStat, m_pParameter1LayerDyn);
    } else {
        m_pParameter1LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter1LayerStat->changeZOrder(0);
    }

    // Draw parameter 2 if exists
    if (m_pParameter2) {
        m_pParameter2->Draw(2, m_pParameter2LayerStat, m_pParameter2LayerDyn);
    } else {
        m_pParameter2LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter2LayerStat->changeZOrder(0);
    }

    // Draw parameter 3 if exists
    if (m_pParameter3) {
        m_pParameter3->Draw(3, m_pParameter3LayerStat, m_pParameter3LayerDyn);
    } else {
        m_pParameter3LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter3LayerStat->changeZOrder(0);
    }
}

// ---------------------------------------------------------------------------------
// Function: Deactivate
// Description: Called when the component becomes inactive
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Deactivate() {
    m_isActive = false;
}

// ---------------------------------------------------------------------------------
// Function: Update
// Description: Periodic update; checks parameter updates and shows info banner
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Update() {
    if (!m_isActive) return;

    // Update parameter 1 and show info if changed
    if (m_pParameter1) {
        m_pParameter1->Update(1, m_pParameter1LayerDyn);
    }

    // Update parameter 2 and show info if changed
    if (m_pParameter2) {
        m_pParameter2->Update(2, m_pParameter2LayerDyn);
    }

    // Update parameter 3 and show info if changed
    if (m_pParameter3) {
       m_pParameter3->Update(3, m_pParameter3LayerDyn);
    }
}

// ---------------------------------------------------------------------------------
// Function: Redraw
// Description: Force redraw of all parameter views (static+dynamic)
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Redraw() {
    if (!m_isActive) return;

    // Redraw all parameters
    if (m_pParameter1) {
        m_pParameter1->Draw(1, m_pParameter1LayerStat, m_pParameter1LayerDyn);
    }
    if (m_pParameter2) {
        m_pParameter2->Draw(2, m_pParameter2LayerStat, m_pParameter2LayerDyn);
    }
    if (m_pParameter3) {
        m_pParameter3->Draw(3, m_pParameter3LayerStat, m_pParameter3LayerDyn);
    }
}


} // namespace DadGUI
//***End of file**************************************************************
