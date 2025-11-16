//==================================================================================
//==================================================================================
// File: cPanelOfSystemView.cpp
// Description: System view panel implementation for parameter management
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cPanelOfSystemView.h"

namespace DadGUI {

//**********************************************************************************
// Class: cPanelOfSystemView
//
// Description: Panel for managing system parameters like color theme and MIDI channel
//
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes the three parameter views
// -----------------------------------------------------------------------------
void cPanelOfSystemView::Initialize(uint32_t SerializeID) {
    // Initialize color theme parameter and view
    m_ColorTheme.Init(SerializeID, 0, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, ColorCallback, (uint32_t) this);
    m_ColorThemeView.Init(&m_ColorTheme, "Theme", "Color Theme");
    m_ColorThemeView.AddDiscreteValue("Blue", "Blue");      // Add color theme options
    m_ColorThemeView.AddDiscreteValue("Amber", "Amber");
    m_ColorThemeView.AddDiscreteValue("Yellow", "Yellow");
    m_ColorThemeView.AddDiscreteValue("Purple", "Purple");

    // Initialize MIDI channel parameter and view
    m_MidiChannel.Init(SerializeID, 0, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, MIDICallback, (uint32_t) this);
    m_MidiChannelView.Init(&m_MidiChannel, "MIDI", "MIDI Channel");
    m_MidiChannelView.AddDiscreteValue("All", "All Channels");  // Add MIDI channel options
    m_MidiChannelView.AddDiscreteValue("CH. 1", "Channel 1");
    m_MidiChannelView.AddDiscreteValue("CH. 2", "Channel 2");
    m_MidiChannelView.AddDiscreteValue("CH. 3", "Channel 3");
    m_MidiChannelView.AddDiscreteValue("CH. 4", "Channel 4");
    m_MidiChannelView.AddDiscreteValue("CH. 5", "Channel 5");
    m_MidiChannelView.AddDiscreteValue("CH. 6", "Channel 6");
    m_MidiChannelView.AddDiscreteValue("CH. 7", "Channel 7");
    m_MidiChannelView.AddDiscreteValue("CH. 8", "Channel 8");
    m_MidiChannelView.AddDiscreteValue("CH. 9", "Channel 9");
    m_MidiChannelView.AddDiscreteValue("CH. 10", "Channel 10");
    m_MidiChannelView.AddDiscreteValue("CH. 11", "Channel 11");
    m_MidiChannelView.AddDiscreteValue("CH. 12", "Channel 12");
    m_MidiChannelView.AddDiscreteValue("CH. 13", "Channel 13");
    m_MidiChannelView.AddDiscreteValue("CH. 14", "Channel 14");
    m_MidiChannelView.AddDiscreteValue("CH. 15", "Channel 15");
    m_MidiChannelView.AddDiscreteValue("CH. 16", "Channel 16");

    // Initialize the parameter view with theme and MIDI controls
    cPanelOfParameterView::Init(&m_ColorThemeView, nullptr, &m_MidiChannelView);
}

// -----------------------------------------------------------------------------
// Adds serializable objects to the serialization family
// -----------------------------------------------------------------------------
void cPanelOfSystemView::addToSerializeFamily(uint32_t SerializeID) {
    __GUI.addSerializeObject(&m_ColorTheme, SerializeID);   // Add color theme to serialization
    __GUI.addSerializeObject(&m_MidiChannel, SerializeID);  // Add MIDI channel to serialization
}

// -----------------------------------------------------------------------------
// Updates the panel state
// -----------------------------------------------------------------------------
void cPanelOfSystemView::Update() {
    if (m_isActive) {
        m_ColorTheme.Process();         // Process color theme changes
        m_MidiChannel.Process();        // Process MIDI channel changes
        cPanelOfParameterView::Update(); // Update base class functionality
    }
}

// -----------------------------------------------------------------------------
// Callback for color theme parameter changes
// -----------------------------------------------------------------------------
void cPanelOfSystemView::ColorCallback(DadDSP::cParameter* pParameter, uint32_t Context) {
    uint8_t IndexPalette = (int8_t) pParameter->getValue(); // Get selected palette index

    // Validate index and set active palette
    if (IndexPalette < NB_PALETTE) {
        __pActivePalette = &__ColorPalette[IndexPalette]; // Update global palette reference
    }
}

// -----------------------------------------------------------------------------
// Callback for MIDI channel parameter changes
// -----------------------------------------------------------------------------
void cPanelOfSystemView::MIDICallback(DadDSP::cParameter* pParameter, uint32_t Context) {
    uint8_t Channel = (int8_t) pParameter->getValue(); // Get selected channel value

    // Convert UI channel selection to MIDI channel format
    if (Channel == 0) {
        Channel = 0xFF; // All channels (broadcast)
    } else {
        Channel--;      // Convert to zero-based MIDI channel
    }

    __Midi.ChangeChanel(Channel); // Apply MIDI channel change
}

} // namespace DadGUI

//***End of file**************************************************************
