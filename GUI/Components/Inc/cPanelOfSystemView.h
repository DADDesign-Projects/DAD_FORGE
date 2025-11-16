//==================================================================================
//==================================================================================
// File: cPanelOfSystemView.h
// Description: Header for system view panel managing color themes and MIDI channels
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include <stdint.h>

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfSystemView
//
// Description: Panel for managing system parameters including color themes and MIDI channels
//
//**********************************************************************************
class cPanelOfSystemView : public cPanelOfParameterView {
public:
    virtual ~cPanelOfSystemView() {};

    // Initializes the parameter views for color theme and MIDI channel
    void Initialize(uint32_t SerializeID);

    // Updates the panel state and processes parameter changes
    void Update() override;

    // Adds serializable objects to the serialization system
    void addToSerializeFamily(uint32_t SerializeID);

    // Callback for color theme parameter changes
    static void ColorCallback(DadDSP::cParameter* pParameter, uint32_t Context);

    // Callback for MIDI channel parameter changes
    static void MIDICallback(DadDSP::cParameter* pParameter, uint32_t Context);

protected:
    DadGUI::cUIParameter           m_ColorTheme;        // Color theme parameter
    DadGUI::cUIParameter           m_MidiChannel;       // MIDI channel parameter

    DadGUI::cParameterDiscretView  m_ColorThemeView;    // Color theme view component
    DadGUI::cParameterDiscretView  m_MidiChannelView;   // MIDI channel view component
};

} // namespace DadGUI

//***End of file**************************************************************
