// ======================================================================
// ======================================================================
// File: cThemesManager.h
// Description: Manages GUI themes and color palettes.
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
#pragma once
#include "cPaletteBuilder.h"
#include "cCallBackIterator.h"
#include "GUI_Event.h"

namespace DadGUI {

// *****************************************************************************
// Global variables declarations
// *****************************************************************************
extern sColorPalette __ColorPalette[]; // Array of available palettes

//**********************************************************************************
// Class: cThemesManager
//
// Description: Manages the active color palette and notifies listeners of changes.
//
//**********************************************************************************

class cThemesManager : public iGUI_EventListener {
public:
    // -----------------------------------------------------------------------------
    // cThemesManager
    // Description: Default constructor.
    // -----------------------------------------------------------------------------
    cThemesManager() = default;

    // -----------------------------------------------------------------------------
    // ~cThemesManager
    // Description: Virtual destructor.
    // -----------------------------------------------------------------------------
    virtual ~cThemesManager() = default;

    // -----------------------------------------------------------------------------
    // Initialize
    // Description: Initializes the theme manager with default settings.
    // -----------------------------------------------------------------------------
    void Initialize();

    // -----------------------------------------------------------------------------
    // on_GUI_Update
    // Description: Updates the theme based on palette changes and triggers redraws.
    // -----------------------------------------------------------------------------
    void on_GUI_Update() override;

    // -----------------------------------------------------------------------------
    // getActivePalette
    // Description: Returns the currently active palette.
    // -----------------------------------------------------------------------------
    inline sColorPalette* getActivePalette() {
        return m_pActivePalette;
    }

    // -----------------------------------------------------------------------------
    // operator->
    // Description: Allows direct access to active palette members via the -> operator (e.g., instance->Color)
    // -----------------------------------------------------------------------------
    inline sColorPalette* operator->() {
        return m_pActivePalette;
    }

    // -----------------------------------------------------------------------------
    // setActivePalette (Pointer)
    // Description: Sets the active palette using a pointer.
    // -----------------------------------------------------------------------------
    inline void setActivePalette(sColorPalette* pColorPalette) {
        if (pColorPalette) {
            m_pActivePalette = pColorPalette;
        } else {
            m_pActivePalette = &__ColorPalette[0];
        }
        m_ThemeChangeListener.NotifyListeners(m_pActivePalette);
    }

    // -----------------------------------------------------------------------------
    // setActivePalette (Index)
    // Description: Sets the active palette using an index.
    // -----------------------------------------------------------------------------
    inline void setActivePalette(uint8_t PaletteIndex) {
        if (PaletteIndex < NB_PALETTE) {
            setActivePalette(&__ColorPalette[PaletteIndex]);
        } else {
            setActivePalette(&__ColorPalette[NB_PALETTE - 1]);
        }
    }

    // -----------------------------------------------------------------------------
    // RegisterThemeChangeListener
    // Description: Registers a callback to be notified when the theme changes.
    // -----------------------------------------------------------------------------
    void RegisterThemeChangeListener(DadUtilities::IteratorCallback_t callback, uint32_t listenerContext, uint8_t priority = 127);

    // -----------------------------------------------------------------------------
    // UnregisterThemeChangeListener
    // Description: Unregisters a previously registered theme change listener.
    // -----------------------------------------------------------------------------
    void UnregisterThemeChangeListener(DadUtilities::IteratorCallback_t callback);

protected:
    // =============================================================================
    // Protected Members
    // =============================================================================
    sColorPalette* 					m_pActivePalette;     // Currently active palette
    cPaletteBuilder 				m_PaletteBuilder;     // Palette builder for dynamic changes
    DadUtilities::cCallBackIterator	m_ThemeChangeListener; // Listener for theme change notifications
};

} // namespace DadGUI

//***End of file**************************************************************
