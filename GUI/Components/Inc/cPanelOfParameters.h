//==================================================================================
//==================================================================================
// File: cPanelOfParameters.h
// Description: GUI Parameters panel
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "GUI_Include.h"
namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfParameterView
// Description: Manages a panel composed of three parameter views and their rendering layers
//**********************************************************************************
class cPanelOfParameterView : public iUIComponent {
public:
    virtual ~cPanelOfParameterView() {}

    // ---------------------------------------------------------------------------------
    // Function: Init
    // Description: Initialize the three parameter views and allocate their layers
    // ---------------------------------------------------------------------------------
    void Init(cParameterView* pParameter1, cParameterView* pParameter2, cParameterView* pParameter3);

    // Called when the component becomes active
    void Activate() override;

    // Called when the component becomes inactive
    void Deactivate() override;

    // Called periodically to refresh the panel
    void Update() override;

    // Force to redraw UI component
    void Redraw() override;

protected:
    // Parameter view pointers
    cParameterView*     m_pParameter1 = nullptr;
    cParameterView*     m_pParameter2 = nullptr;
    cParameterView*     m_pParameter3 = nullptr;

     // Layers for each parameter (static and dynamic)
    DadGFX::cLayer*     m_pParameter1LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter1LayerStat = nullptr;
    DadGFX::cLayer*     m_pParameter2LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter2LayerStat = nullptr;
    DadGFX::cLayer*     m_pParameter3LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter3LayerStat = nullptr;

    // Memorize active state
    bool                m_isActive = false;
};

} // namespace DadGUI
//***End of file**************************************************************
