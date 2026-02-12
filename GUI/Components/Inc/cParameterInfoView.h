//==================================================================================
//==================================================================================
// File: cParameterInfoView.h
// Description: Parameter information temporary overlay tooltip windows implementation.
//              Display duration: PARAM_INFO_TIME_MS seconds from last parameter interaction.
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "GUI_Include.h"
#include "GUI_Event.h"
#include <string>

namespace DadGUI {

constexpr uint32_t	PARAMETER_INFO_VIEW_DISPLAY_TIME_MS = 2000; // Time to show parameter info (ms)

//**********************************************************************************
// Class: cParameterInfoView
// Description: Handles temporary display of parameter information (name and value)
//**********************************************************************************
class cParameterInfoView : public iGUI_EventListener{
public:
    cParameterInfoView() = default;
    virtual ~cParameterInfoView() = default;

    // ---------------------------------------------------------------------------------
    // Function: Init
    // Initialize the class
    void Init();

    // ---------------------------------------------------------------------------------
    // Function: ShowParamView
    // Description: Display the parameter information on a given layer
    // ---------------------------------------------------------------------------------
    void ShowParamView(const std::string Name, const std::string Value);

    // ---------------------------------------------------------------------------------
    // Function: HideParamView
    // Description: Hide the parameter information view (reset z-order or clear)
    // ---------------------------------------------------------------------------------
    void HideParamView();

    // ---------------------------------------------------------------------------------
    // Function: on_GUI_Update
    // Update GUI Object time GUI_UPDATE_MS in __ms__
    void on_GUI_Update() override;

    // ---------------------------------------------------------------------------------
    // Parameter change callback
    static void ParameterChange(void* pParameter, uint32_t Context);

protected:
    // Info layer for temporary display
    DadGFX::cLayer*     m_pParamInfoLayer = nullptr;

    // Internal counter in ms for info view timing
    int32_t            m_InfoViewTimeCounter = 0;
};

} // namespace DadGUI
//***End of file**************************************************************
