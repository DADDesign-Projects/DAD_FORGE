// ======================================================================
// ======================================================================
// File: cThemesManager.cpp
// Description: Implementation of the cThemesManager class.
//
// Copyright(c) 2025-2026 DadDesign-Projects
//
// SPDX-License-Identifier: Apache-2.0
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// ======================================================================
// ======================================================================
#include "cThemesManager.h"
#include "GUI_Event.h"
#include "@Options.h"

namespace DadGUI {
// *****************************************************************************
// Global variables declarations
// *****************************************************************************
extern GUI_EventManager __GUI_EventManager;

#if __has_include("UserPalette.h")
#include "UserPalette.h"
#else

//**********************************************************************************
// Color Palette Definitions
//**********************************************************************************

// Array of color palettes available in the system
sColorPalette __ColorPalette[NB_PALETTE] = {
#include "MixBlue.hpp"
,
#include "BlueGreen.hpp"
,
#include "Ambre2.hpp"
,
#include "Blue.hpp"
,
#include "Ambre.hpp"
,
#include "Yellow.hpp"
,
#include "Purple.hpp"
,
#include "PaleBlue.hpp"
};

#endif

//**********************************************************************************
// Class: cThemesManager
//
// Description: Manages the active color palette and notifies listeners of changes.
//
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialize
// Description: Initializes the theme manager with default settings.
// -----------------------------------------------------------------------------
void cThemesManager::Initialize() {
    // Set default palette to the first available one
    m_pActivePalette = &__ColorPalette[0];
    m_PaletteBuilder.InitPalette(m_pActivePalette);
    m_ThemeChangeListener.Clear();
    __GUI_EventManager.Subscribe_Update(this);
}

// -----------------------------------------------------------------------------
// on_GUI_Update
// Description: Updates the theme based on palette changes and triggers redraws.
// -----------------------------------------------------------------------------
void cThemesManager::on_GUI_Update() {

#ifdef PALETTE_BUILDER
    // Handle dynamic palette changes if palette builder is enabled
    m_PaletteBuilder.ParseBuffer();
    if (m_PaletteBuilder.IsChangedPalette()) {
        setActivePalette(m_PaletteBuilder.getPalette());
    }
#endif

}

// -----------------------------------------------------------------------------
// RegisterThemeChangeListener
// Description: Registers a callback to be notified when the theme changes.
// -----------------------------------------------------------------------------
void cThemesManager::RegisterThemeChangeListener(DadUtilities::IteratorCallback_t callback, uint32_t listenerContext, uint8_t priority) {
    m_ThemeChangeListener.RegisterListener(callback, listenerContext, priority);
}

// -----------------------------------------------------------------------------
// UnregisterThemeChangeListener
// Description: Unregisters a previously registered theme change listener.
// -----------------------------------------------------------------------------
void cThemesManager::UnregisterThemeChangeListener(DadUtilities::IteratorCallback_t callback) {
    m_ThemeChangeListener.UnregisterListener(callback);
}

} // namespace DadGUI

//***End of file**************************************************************
