//==================================================================================
//==================================================================================
// File: DadGUI.h
// Description: Core infrastructure for graphical user interface
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "Sections.h"
#include "iUIComponent.h"
#include "iRTObject.h"
#include "iGUIProcessObject.h"
#include "cMemoryManager.h"
#include "cObjectIterator.h"
#include "Serialize.h"
#include "GFX.h"
#include <cstdint>

namespace DadGUI {

//**********************************************************************************
// GUI Layout Overview
//
// Screen Layout:
//   _____________________________________________________________
//  |                           MenuArea                           |
//  |______________________________________________________________|
//  |   Parameter1Area  |    Parameter2Area   |   Parameter3Area   |
//  |                   |                     |                    |
//  |                   |                     |                    |
//  |                   |                     |                    |
//  |___________________|_____________________|____________________|
//  |                          InfoArea                            |
//  |                                                              |
//  |______________________________________________________________|
//
// Each zone corresponds to a dedicated component with its own visual and
// interactive logic managed by the DadGUI controller.
//**********************************************************************************

// =============================================================================
// Screen and Layout Constants
// =============================================================================

#define SCREEN_WIDTH        320     // Total screen width in pixels
#define SCREEN_HEIGHT       240     // Total screen height in pixels

// Menu layout parameters
#define MENU_HEIGHT         22      // Height of the menu area
#define MENU_EDGE           10      // Edge margin for menu items
#define NB_MENU_ITEM        4       // Number of menu items
#define MENU_ITEM_WIDTH     ((SCREEN_WIDTH - (2*MENU_EDGE)) / NB_MENU_ITEM)  // Width per menu item

// Parameter area layout parameters
#define MAIN_WIDTH          SCREEN_WIDTH                    // Main area width
#define PARAM_HEIGHT        128                             // Height of parameter area
#define NB_PARAM_ITEM       3                               // Number of parameter items
#define PARAM_WIDTH         (MAIN_WIDTH / NB_PARAM_ITEM)    // Width per parameter item
#define PARAM_NAME_HEIGHT   32                              // Height for parameter names
#define PARAM_FORM_HEIGHT   64                              // Height for parameter forms
#define PARAM_VAL_HEIGHT    32                              // Height for parameter values

// Info area layout parameters
#define INFO_HEIGHT         (SCREEN_HEIGHT - (MENU_HEIGHT + PARAM_HEIGHT))  // Calculated info area height

// =============================================================================
// Color Palette Definition
// =============================================================================

#define NB_PALETTE 4  // Number of available color palettes

// Structure defining a complete color palette for the GUI
struct sColorPalette {
    // Menu colors
    DadGFX::sColor MenuBack;        // Menu background color
    DadGFX::sColor MenuText;        // Menu text color
    DadGFX::sColor MenuActiveText;  // Active menu text color
    DadGFX::sColor MenuActive;      // Active menu item color
    DadGFX::sColor MenuArrow;       // Menu arrow color

    // Parameter area colors
    DadGFX::sColor ParameterBack;   // Parameter background color
    DadGFX::sColor ParameterName;   // Parameter name color
    DadGFX::sColor ParameterNum;    // Parameter number color
    DadGFX::sColor ParameterValue;  // Parameter value color
    DadGFX::sColor ParameterLines;  // Parameter lines color
    DadGFX::sColor ParameterCursor; // Parameter cursor color

    // Parameter info colors
    DadGFX::sColor ParamInfoBack;   // Parameter info background
    DadGFX::sColor ParamInfoName;   // Parameter info name color
    DadGFX::sColor ParamInfoValue;  // Parameter info value color

    // Main info colors
    DadGFX::sColor MainInfoBack;    // Main info background
    DadGFX::sColor MainInfoMem;     // Memory indicator color
    DadGFX::sColor MainInfoDirty;   // Dirty state indicator color
    DadGFX::sColor MainInfoState;   // System state color

    // Memory View colors
    DadGFX::sColor MemViewBack;     // Memory view background
    DadGFX::sColor MemViewText;     // Memory view text color
    DadGFX::sColor MemViewLine;     // Memory view line color
    DadGFX::sColor MemViewActive;   // Active memory item color

    // VuMeter View colors
    DadGFX::sColor VuMeterBack;     // VU meter background
    DadGFX::sColor VuMeterText;     // VU meter text color
    DadGFX::sColor VuMeterLine;     // VU meter line color
    DadGFX::sColor VuMeterCursor;   // VU meter cursor color
    DadGFX::sColor VuMeterPeak;     // VU meter peak indicator color
    DadGFX::sColor VuMeterClip;     // VU meter clip indicator color

    // Splash screen colors
    DadGFX::sColor SplatchBack;     // Splash screen background
    DadGFX::sColor SplatchText;     // Splash screen text color
};

// Global color palette declarations
extern const sColorPalette __ColorPalette[NB_PALETTE];  // Array of available palettes
extern const sColorPalette* __pActivePalette;           // Pointer to currently active palette

// =============================================================================
// Font Shortcuts
// =============================================================================

// Macros for quick access to different font sizes and styles
#define FONTXXS    __GUI.GetFontXXS()    // Extra extra small font
#define FONTXXSB   __GUI.GetFontXXSB()   // Extra extra small bold font
#define FONTXS     __GUI.GetFontXS()     // Extra small font
#define FONTXSB    __GUI.GetFontXSB()    // Extra small bold font
#define FONTS      __GUI.GetFontS()      // Small font
#define FONTSB     __GUI.GetFontSB()     // Small bold font
#define FONTM      __GUI.GetFontM()      // Medium font
#define FONTMB     __GUI.GetFontMB()     // Medium bold font
#define FONTL      __GUI.GetFontL()      // Large font
#define FONTLB     __GUI.GetFontLB()     // Large bold font
#define FONTXL     __GUI.GetFontXL()     // Extra large font
#define FONTXLB    __GUI.GetFontXLB()    // Extra large bold font
#define FONTXXL    __GUI.GetFontXXL()    // Extra extra large font
#define FONTXXLB   __GUI.GetFontXXLB()   // Extra extra large bold font
#define FONTXXXL   __GUI.GetFontXXXL()   // Extra extra extra large font
#define FONTXXXLB  __GUI.GetFontXXXLB()  // Extra extra extra large bold font

//-------------------------------------------------------------------------
// Slot Memory Manager
//-------------------------------------------------------------------------
extern cMemoryManager __MemoryManager;  // Global memory manager instance

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

class cMainGUI {
public:
    // =============================================================================
    // System Initialization
    // =============================================================================

    //-------------------------------------------------------------------------
    // Initialize
    //
    // Description: Prepares the GUI system by initializing fonts, palettes,
    // and layout data.
    //-------------------------------------------------------------------------
    void Initialize();

    //-------------------------------------------------------------------------
    // Start
    //
    // Description: Initializes memory management and sets up system callbacks.
    //-------------------------------------------------------------------------
    void Start();

    // =============================================================================
    // Update Management
    // =============================================================================

    //-------------------------------------------------------------------------
    // setUpdateID
    //
    // Description: Sets the active update family ID for component grouping.
    //-------------------------------------------------------------------------
    void setUpdateID(uint32_t updateID);

    //-------------------------------------------------------------------------
    // addUpdateComponent
    //
    // Description: Registers a component in a specific update family.
    //-------------------------------------------------------------------------
    void addUpdateComponent(iUIComponent* pUpdateComponent, uint32_t ID);

    //-------------------------------------------------------------------------
    // Update (inline)
    //
    // Description: Updates all components using the current active update family.
    //-------------------------------------------------------------------------
    inline void Update() {
        Update(m_UpdateID);
    }

    //-------------------------------------------------------------------------
    // Update
    //
    // Description: Updates all UI components for the specified update family.
    //-------------------------------------------------------------------------
    void Update(uint32_t updateID);

    //-------------------------------------------------------------------------
    // setComponentNeedUpdate
    //
    // Description: Marks a specific component for immediate update.
    //-------------------------------------------------------------------------
    void setComponentNeedUpdate(iUIComponent* pComponent) {
        m_pComponentNeedUpdate = pComponent;
    }

    // =============================================================================
    // Real-Time Process Management
    // =============================================================================

    //-------------------------------------------------------------------------
    // setRtProcessID
    //
    // Description: Sets the active real-time process family ID.
    //-------------------------------------------------------------------------
    void setRtProcessID(uint32_t RtrocessID);

    //-------------------------------------------------------------------------
    // addRtProcessObject
    //
    // Description: Registers a real-time processing object.
    //-------------------------------------------------------------------------
    void addRtProcessObject(iRtObject* pRtObject, uint32_t ID);

    //-------------------------------------------------------------------------
    // RTProcess
    //
    // Description: Executes real-time processing for active family objects.
    //-------------------------------------------------------------------------
    ITCM void RTProcess();

    // =============================================================================
    // GUI Process Management
    // =============================================================================

    //-------------------------------------------------------------------------
    // setGUIProcess
    //
    // Description: Sets the GUI process object for audio-related operations.
    //-------------------------------------------------------------------------
    void setGUIProcess(iGUIProcessObject* pGUIProcessObject) {
        m_pGUIProcessObject = pGUIProcessObject;
    }

    //-------------------------------------------------------------------------
    // GUIProcessIn
    //
    // Description: Process audio buffer through GUI object before audio process
    //-------------------------------------------------------------------------
    ITCM void GUIProcessIn(AudioBuffer* pIn);

    //-------------------------------------------------------------------------
    // GUIProcessOut
    //
    // Description: Process audio buffer through GUI object after audio process
    //-------------------------------------------------------------------------
    ITCM void GUIProcessOut(AudioBuffer* pOut);

    // =============================================================================
    // Serialization Management
    // =============================================================================

    //-------------------------------------------------------------------------
    // setSerializeID
    //
    // Description: Sets the active serialization family ID.
    //-------------------------------------------------------------------------
    void setSerializeID(uint32_t serializeID);

    //-------------------------------------------------------------------------
    // addSerializeObject
    //
    // Description: Registers a serializable object for persistent storage.
    //-------------------------------------------------------------------------
    void addSerializeObject(DadPersistentStorage::cSerializedObject* pSerializedObject, uint32_t serializeID);

    //-------------------------------------------------------------------------
    // Save (inline)
    //
    // Description: Saves all objects from the active serialization family.
    //-------------------------------------------------------------------------
    inline void Save(DadPersistentStorage::cSerialize* pSerializer) {
        Save(pSerializer, m_SerializeID);
    }

    //-------------------------------------------------------------------------
    // Save
    //
    // Description: Saves objects from specified serialization family.
    //-------------------------------------------------------------------------
    void Save(DadPersistentStorage::cSerialize* pSerializer, uint32_t serializeID);

    //-------------------------------------------------------------------------
    // Restore (inline)
    //
    // Description: Restores all objects for the active serialization family.
    //-------------------------------------------------------------------------
    void Restore(DadPersistentStorage::cSerialize* pSerializer) {
        Restore(pSerializer, m_SerializeID);
    }

    //-------------------------------------------------------------------------
    // Restore
    //
    // Description: Restores objects from specified serialization family.
    //-------------------------------------------------------------------------
    void Restore(DadPersistentStorage::cSerialize* pSerializer, uint32_t serializeID);

    //-------------------------------------------------------------------------
    // IsRestoreInProcess
    //
    // Description: Checks if a restore operation is currently in progress.
    //-------------------------------------------------------------------------
    bool IsRestoreInProcess() {
        return m_RestoreInProcess;
    }

    //-------------------------------------------------------------------------
    // resetRestoreInProcess
    //
    // Description: Resets the restore in process flag.
    //-------------------------------------------------------------------------
    void resetRestoreInProcess() {
        m_RestoreInProcess = false;
    }

    //-------------------------------------------------------------------------
    // isParametresDirty
    //
    // Description: Checks if any serializable object has unsaved changes.
    //-------------------------------------------------------------------------
    bool isParametresDirty();

    // =============================================================================
    // Component Activation
    // =============================================================================

    //-------------------------------------------------------------------------
    // activeMainComponent
    //
    // Description: Activates a new main component, deactivating previous one.
    //-------------------------------------------------------------------------
    void activeMainComponent(iUIComponent* pMainComponent);

    //-------------------------------------------------------------------------
    // activeBackComponent
    //
    // Description: Activates a new background component, deactivating previous one.
    //-------------------------------------------------------------------------
    void activeBackComponent(iUIComponent* pBackComponent);

    // =============================================================================
    // Font Accessors
    // =============================================================================

    // Provides type-safe access to all predefined font sizes and styles
    inline DadGFX::cFont* GetFontXXS()   const { return m_pFontXXS;   }
    inline DadGFX::cFont* GetFontXXSB()  const { return m_pFontXXSB;  }
    inline DadGFX::cFont* GetFontXS()    const { return m_pFontXS;    }
    inline DadGFX::cFont* GetFontXSB()   const { return m_pFontXSB;   }
    inline DadGFX::cFont* GetFontS()     const { return m_pFontS;     }
    inline DadGFX::cFont* GetFontSB()    const { return m_pFontSB;    }
    inline DadGFX::cFont* GetFontM()     const { return m_pFontM;     }
    inline DadGFX::cFont* GetFontMB()    const { return m_pFontMB;    }
    inline DadGFX::cFont* GetFontL()     const { return m_pFontL;     }
    inline DadGFX::cFont* GetFontLB()    const { return m_pFontLB;    }
    inline DadGFX::cFont* GetFontXL()    const { return m_pFontXL;    }
    inline DadGFX::cFont* GetFontXLB()   const { return m_pFontXLB;   }
    inline DadGFX::cFont* GetFontXXL()   const { return m_pFontXXL;   }
    inline DadGFX::cFont* GetFontXXLB()  const { return m_pFontXXLB;  }
    inline DadGFX::cFont* GetFontXXXL()  const { return m_pFontXXXL;  }
    inline DadGFX::cFont* GetFontXXXLB() const { return m_pFontXXXLB; }

protected:
    // =============================================================================
    // MIDI Callback Functions
    // =============================================================================

    //-------------------------------------------------------------------------
    // MIDI_On_CallBack
    //
    // Description: MIDI callback for system ON command.
    //-------------------------------------------------------------------------
    static void MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    //-------------------------------------------------------------------------
    // MIDI_Off_CallBack
    //
    // Description: MIDI callback for system OFF command.
    //-------------------------------------------------------------------------
    static void MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    //-------------------------------------------------------------------------
    // MIDI_ByPass_CallBack
    //
    // Description: MIDI callback for system BYPASS command.
    //-------------------------------------------------------------------------
    static void MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // =============================================================================
    // Member Variables
    // =============================================================================

    //-------------------------------------------------------------------------
    // UI Components
    //-------------------------------------------------------------------------
    iUIComponent* m_pMenuComponent;  // Currently active menu component
    iUIComponent* m_pMainComponent;  // Currently active main component
    iUIComponent* m_pBackComponent;  // Currently active background component

    //-------------------------------------------------------------------------
    // Update Management
    //-------------------------------------------------------------------------
    DadUtilities::cObjectIterator<iUIComponent*> m_UpdateComponent;  // Update component iterator
    uint32_t m_UpdateID;                                             // Current update family ID
    iUIComponent* m_pComponentNeedUpdate;                            // Component needing immediate update

    //-------------------------------------------------------------------------
    // Serialization Management
    //-------------------------------------------------------------------------
    DadUtilities::cObjectIterator<DadPersistentStorage::cSerializedObject*> m_SerializedObject;  // Serialized object iterator
    uint32_t m_SerializeID;                                         // Current serialization family ID
    bool m_RestoreInProcess;                                        // Restore operation flag

    //-------------------------------------------------------------------------
    // Real-Time Process Management
    //-------------------------------------------------------------------------
    DadUtilities::cObjectIterator<iRtObject*> m_RtProcessObject;    // Real-time object iterator
    uint32_t m_RtProcessID;                                         // Current RT process family ID

    //-------------------------------------------------------------------------
    // GUI Process Management
    //-------------------------------------------------------------------------
    iGUIProcessObject* m_pGUIProcessObject;                       // GUI process object pointer

    //-------------------------------------------------------------------------
    // Font Resources
    //-------------------------------------------------------------------------
    DadGFX::cFont* m_pFontXXS   = nullptr;   // Extra extra small font
    DadGFX::cFont* m_pFontXXSB  = nullptr;   // Extra extra small bold font
    DadGFX::cFont* m_pFontXS    = nullptr;   // Extra small font
    DadGFX::cFont* m_pFontXSB   = nullptr;   // Extra small bold font
    DadGFX::cFont* m_pFontS     = nullptr;   // Small font
    DadGFX::cFont* m_pFontSB    = nullptr;   // Small bold font
    DadGFX::cFont* m_pFontM     = nullptr;   // Medium font
    DadGFX::cFont* m_pFontMB    = nullptr;   // Medium bold font
    DadGFX::cFont* m_pFontL     = nullptr;   // Large font
    DadGFX::cFont* m_pFontLB    = nullptr;   // Large bold font
    DadGFX::cFont* m_pFontXL    = nullptr;   // Extra large font
    DadGFX::cFont* m_pFontXLB   = nullptr;   // Extra large bold font
    DadGFX::cFont* m_pFontXXL   = nullptr;   // Extra extra large font
    DadGFX::cFont* m_pFontXXLB  = nullptr;   // Extra extra large bold font
    DadGFX::cFont* m_pFontXXXL  = nullptr;   // Extra extra extra large font
    DadGFX::cFont* m_pFontXXXLB = nullptr;   // Extra extra extra large bold font
};

} // namespace DadGUI

//***End of file**************************************************************
