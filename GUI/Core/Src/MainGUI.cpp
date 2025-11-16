//==================================================================================
//==================================================================================
// File: DadGUI.cpp
// Description: Core infrastructure for graphical user interface
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "HardwareAndCo.h"
#include "cPaletteBuilder.h"

namespace DadGUI {

//**********************************************************************************
// Color Palette Definitions
//**********************************************************************************

// Array of color palettes available in the system
const sColorPalette __ColorPalette[NB_PALETTE] = {
#include "Blue.hpp"
,
#include "Ambre.hpp"
,
#include "Yellow.hpp"
,
#include "Purple.hpp"
};

const sColorPalette* __pActivePalette = &__ColorPalette[0];      // Currently active palette
const sColorPalette* __MempActivePalette = __pActivePalette;     // Previous active palette for change detection

//-------------------------------------------------------------------------
// Slot Memory Manager
//-------------------------------------------------------------------------
cMemoryManager __MemoryManager;  // Global memory manager instance

//**********************************************************************************
// Class: cMainGUI
//
// Description:
// Main class responsible for managing the PENDA graphical user interface.
// Handles activation and update cycles of UI components, persistent serialization
// of user parameters or GUI states, and centralized management of color palettes
// and fonts. The GUI is organized into functional "families" (menu, parameter, info),
// and each family can contain multiple registered components managed dynamically
// through iterators.
//**********************************************************************************

// =============================================================================
// System Initialization
// =============================================================================

//-------------------------------------------------------------------------
// Initialize
//
// Description: Prepares the GUI system by initializing fonts, palettes,
// and layout data. Allocates font objects from binary font files stored
// in flash memory.
//-------------------------------------------------------------------------
void cMainGUI::Initialize() {
    // Initialize all font sizes and styles from binary font files
    m_pFontXXS   = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_11p.bin"));
    m_pFontXXSB  = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_11pb.bin"));
    m_pFontXS    = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_12p.bin"));
    m_pFontXSB   = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_12pb.bin"));
    m_pFontS     = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_16p.bin"));
    m_pFontSB    = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_16pb.bin"));
    m_pFontM     = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_20p.bin"));
    m_pFontMB    = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_20pb.bin"));
    m_pFontL     = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_24p.bin"));
    m_pFontLB    = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_24pb.bin"));
    m_pFontXL    = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_30p.bin"));
    m_pFontXLB   = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_30pb.bin"));
    m_pFontXXL   = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_38p.bin"));
    m_pFontXXLB  = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_38pb.bin"));
    m_pFontXXXL  = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_48p.bin"));
    m_pFontXXXLB = new DadGFX::cFont((DadGFX::GFXBinFont*)__FlasherStorage.GetFilePtr("Font_48pb.bin"));

    // Initialize component pointers to null
    m_pMenuComponent = nullptr;
    m_pMainComponent = nullptr;
    m_pBackComponent = nullptr;

    // Initialize state variables
    m_UpdateID = 0;
    m_SerializeID = 0;
    m_RtProcessID = 0;
    m_RestoreInProcess = false;
    m_pGUIProcessObject = nullptr;
    m_pComponentNeedUpdate = nullptr;

#ifdef PALETTE_BUILDER
    // Initialize palette builder if enabled
    __cPaletteBuilder.InitPalette(&__ColorPalette[0]);
#endif
}

//-------------------------------------------------------------------------
// Start
//
// Description: Initializes the memory manager and sets up MIDI callbacks
// for preset management and system control commands.
//-------------------------------------------------------------------------
void cMainGUI::Start() {
    // Initialize memory management system
    __MemoryManager.Init();
    __MemoryManager.RestoreSlot(__MemoryManager.getActiveSlot());

    // Register MIDI callbacks for preset and system control
    __Midi.addControlChangeCallback(MIDI_CC_PRESET_UP, reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_PresetUp_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_PRESET_DOWN, reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_PresetDown_CallBack);
    __Midi.addProgramChangeCallback(reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_ProgramChange_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_ON, 0, &MIDI_On_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_OFF, 0, &MIDI_Off_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_BYPASS, 0, &MIDI_ByPass_CallBack);
}

// =============================================================================
// Update Management
// =============================================================================

//-------------------------------------------------------------------------
// setUpdateID
//
// Description: Sets the active update family ID and activates the corresponding
// component group for updates.
//-------------------------------------------------------------------------
void cMainGUI::setUpdateID(uint32_t updateID) {
    m_UpdateID = updateID;
    m_UpdateComponent.setActiveFamily(updateID);
}

//-------------------------------------------------------------------------
// addUpdateComponent
//
// Description: Registers a component in a specific update family for
// organized update cycles.
//-------------------------------------------------------------------------
void cMainGUI::addUpdateComponent(iUIComponent* pUpdateComponent, uint32_t ID) {
    if (pUpdateComponent) {
        m_UpdateComponent.addObject(ID, pUpdateComponent);
    }
}

//-------------------------------------------------------------------------
// Update
//
// Description: Updates all UI components including main, background, and
// menu components, as well as the active update family. Also handles
// real-time processing and palette change detection with automatic redraw.
//-------------------------------------------------------------------------
void cMainGUI::Update(uint32_t updateID) {
    // Update main UI components
    if (m_pMenuComponent) m_pMenuComponent->Update();
    if (m_pBackComponent) m_pBackComponent->Update();
    if (m_pMainComponent) m_pMainComponent->Update();

    // Update components in the active update family
    if (m_UpdateComponent.setActiveFamily(updateID)) {
        m_UpdateComponent.forEachInActiveFamily(
            [](iUIComponent* component) {
                if (component) component->Update();
            });
    }

    // Always update family 0 components (base level)
    m_UpdateComponent.forEachInFamily(0,
        [](iUIComponent* component) {
            if (component) component->Update();
        });

    // Process real-time objects in family 0
    m_RtProcessObject.forEachInFamily(0,
        [](iRtObject* RtObject) {
            if (RtObject) RtObject->RtProcess();
        });

#ifdef PALETTE_BUILDER
    // Handle dynamic palette changes if palette builder is enabled
    __cPaletteBuilder.ParseBuffer();
    if (__cPaletteBuilder.IsChangedPalette()) {
        __pActivePalette = __cPaletteBuilder.getPalette();
        __MempActivePalette = nullptr;
    }
#endif

    // Detect palette changes and trigger redraws
    if (__MempActivePalette != __pActivePalette) {
        __MempActivePalette = __pActivePalette;
        if (m_pMenuComponent) m_pMenuComponent->Redraw();
        if (m_pBackComponent) m_pBackComponent->Redraw();
        if (m_pMainComponent) m_pMainComponent->Redraw();
    }

    // Update any component marked for immediate update
    if (m_pComponentNeedUpdate != nullptr) {
        m_pComponentNeedUpdate->Update();
    }
}

// =============================================================================
// Real-Time Process Management
// =============================================================================

//-------------------------------------------------------------------------
// setRtProcessID
//
// Description: Sets the active real-time process family ID for organizing
// real-time processing objects.
//-------------------------------------------------------------------------
void cMainGUI::setRtProcessID(uint32_t RtProcessID) {
    m_RtProcessID = RtProcessID;
    m_RtProcessObject.setActiveFamily(RtProcessID);
}

//-------------------------------------------------------------------------
// addRtProcessObject
//
// Description: Registers a real-time processing object in a specific family
// for organized real-time processing.
//-------------------------------------------------------------------------
void cMainGUI::addRtProcessObject(iRtObject* pRtObject, uint32_t RtProcessID) {
    if (pRtObject) {
        m_RtProcessObject.addObject(RtProcessID, pRtObject);
    }
}

//-------------------------------------------------------------------------
// RTProcess
//
// Description: Executes real-time processing for all objects in the active
// real-time process family.
//-------------------------------------------------------------------------
void cMainGUI::RTProcess() {
    if (m_RtProcessID != 0) {
        m_RtProcessObject.forEachInActiveFamily(
            [](iRtObject* RtObject) {
                if (RtObject) RtObject->RtProcess();
            });
    }
}

// =============================================================================
// GUI Process Management
// =============================================================================

//-------------------------------------------------------------------------
// GUIProcess
//
// Description: Processes GUI-related audio operations if a GUI process
// object is registered.
//-------------------------------------------------------------------------
void cMainGUI::GUIProcess(AudioBuffer* pIn) {
    if (m_pGUIProcessObject) {
        m_pGUIProcessObject->GUIProcess(pIn);
    }
}

// =============================================================================
// Serialization Management
// =============================================================================

//-------------------------------------------------------------------------
// setSerializeID
//
// Description: Sets the active serialization family ID for organizing
// persistent data objects.
//-------------------------------------------------------------------------
void cMainGUI::setSerializeID(uint32_t serializeID) {
    m_SerializeID = serializeID;
    m_SerializedObject.setActiveFamily(serializeID);
}

//-------------------------------------------------------------------------
// addSerializeObject
//
// Description: Registers a serializable object in the system for
// persistent storage management.
//-------------------------------------------------------------------------
void cMainGUI::addSerializeObject(DadPersistentStorage::cSerializedObject* pSerializedObject, uint32_t serializeID) {
    if (pSerializedObject) {
        m_SerializedObject.addObject(serializeID, pSerializedObject);
    }
}

//-------------------------------------------------------------------------
// Save
//
// Description: Saves the state of all serializable objects in the active
// family to persistent storage.
//-------------------------------------------------------------------------
void cMainGUI::Save(DadPersistentStorage::cSerialize* pSerializer, uint32_t serializeID) {
    if (pSerializer && m_SerializedObject.setActiveFamily(serializeID)) {
        m_SerializedObject.forEachInActiveFamily(
            [pSerializer](DadPersistentStorage::cSerializedObject* obj) {
                if (obj) obj->Save(pSerializer);
            });
    }
}

//-------------------------------------------------------------------------
// Restore
//
// Description: Restores the state of all serializable objects in the active
// family from persistent storage.
//-------------------------------------------------------------------------
void cMainGUI::Restore(DadPersistentStorage::cSerialize* pSerializer, uint32_t serializeID) {
    m_RestoreInProcess = true;
    if (pSerializer && m_SerializedObject.setActiveFamily(serializeID)) {
        m_SerializedObject.forEachInActiveFamily(
            [pSerializer](DadPersistentStorage::cSerializedObject* obj) {
                if (obj) obj->Restore(pSerializer);
            });
    }
}

//-------------------------------------------------------------------------
// isParametresDirty
//
// Description: Checks if any serializable object in the active family
// has unsaved changes (dirty state).
//-------------------------------------------------------------------------
bool cMainGUI::isParametresDirty() {
    bool Dirty = false;
    m_SerializedObject.forEachInActiveFamily(
        [&Dirty](DadPersistentStorage::cSerializedObject* obj) {
            if (obj) {
                if (true == obj->isDirty()) {
                    Dirty = true;
                }
            }
        });
    return Dirty;
}

// =============================================================================
// Component Activation
// =============================================================================

//-------------------------------------------------------------------------
// activeMainComponent
//
// Description: Activates a new main component, deactivating the previous
// one if exists. Only one main component can be active at a time.
//-------------------------------------------------------------------------
void cMainGUI::activeMainComponent(iUIComponent* pMainComponent) {
    if (m_pMainComponent) m_pMainComponent->Deactivate();
    if (pMainComponent) {
        m_pMainComponent = pMainComponent;
        m_pMainComponent->Activate();
    }
}

//-------------------------------------------------------------------------
// activeBackComponent
//
// Description: Activates a new background component, deactivating the
// previous one if exists. Only one background component can be active at a time.
//-------------------------------------------------------------------------
void cMainGUI::activeBackComponent(iUIComponent* pBackComponent) {
    if (m_pBackComponent) m_pBackComponent->Deactivate();
    if (pBackComponent) {
        m_pBackComponent = pBackComponent;
        m_pBackComponent->Activate();
    }
}

// =============================================================================
// MIDI Callback Functions
// =============================================================================

//-------------------------------------------------------------------------
// MIDI_On_CallBack
//
// Description: MIDI callback for system ON command
//-------------------------------------------------------------------------
void cMainGUI::MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData) {
    __OnOffCmd = On;
}

//-------------------------------------------------------------------------
// MIDI_Off_CallBack
//
// Description: MIDI callback for system OFF command
//-------------------------------------------------------------------------
void cMainGUI::MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData) {
    __OnOffCmd = Off;
}

//-------------------------------------------------------------------------
// MIDI_ByPass_CallBack
//
// Description: MIDI callback for system BYPASS command
//-------------------------------------------------------------------------
void cMainGUI::MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData) {
    __OnOffCmd = ByPass;
}

} // namespace DadGUI

//***End of file**************************************************************
