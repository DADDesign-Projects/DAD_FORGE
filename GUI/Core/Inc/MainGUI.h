//==================================================================================
//==================================================================================
// File: DadGUI.h
// Description: Core infrastructure for graphical user interface management
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#include "Sections.h"
#include "iUIComponent.h"
#include "cMemoryManager.h"
#include "GUI_Event.h"
#include "cCallBackIterator.h"
#include "cParameter.h"
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

#define MENU_HEIGHT         22      // Height of the menu area
#define MENU_EDGE           10      // Edge margin for menu items
#define NB_MENU_ITEM        4       // Number of menu items
#define MENU_ITEM_WIDTH     ((SCREEN_WIDTH - (2*MENU_EDGE)) / NB_MENU_ITEM)  // Width per menu item

#define MAIN_WIDTH          SCREEN_WIDTH                    // Main area width
#define PARAM_HEIGHT        128                             // Height of parameter area
#define NB_PARAM_ITEM       3                               // Number of parameter items
#define PARAM_WIDTH         (MAIN_WIDTH / NB_PARAM_ITEM)    // Width per parameter item
#define PARAM_NAME_HEIGHT   32                              // Height for parameter names
#define PARAM_FORM_HEIGHT   64                              // Height for parameter forms
#define PARAM_VAL_HEIGHT    32                              // Height for parameter values

#define INFO_HEIGHT         (SCREEN_HEIGHT - (MENU_HEIGHT + PARAM_HEIGHT))  // Calculated info area height

// =============================================================================
// Color Palette Definition
// =============================================================================

#define NB_PALETTE 4  // Number of available color palette

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

// -----------------------------------------------------------------------------
// Slot Memory Manager
// -----------------------------------------------------------------------------
extern cMemoryManager __MemoryManager;  // Global memory manager instance

class cParameterView;

//**********************************************************************************
// Class: cMainGUI
//
// Description:
//   Main GUI controller class that manages components, event handling,
//   and system-wide GUI operations.
//**********************************************************************************

class cMainGUI : public iGUI_EventListener {
public:
    virtual ~cMainGUI() = default;

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initialize
    //
    // Description: Prepares the GUI system by initializing fonts, palettes,
    //   and layout data.
    // -----------------------------------------------------------------------------
    void Initialize();

    // -----------------------------------------------------------------------------
    // Start
    //
    // Description: Initializes memory management and sets up system callbacks.
    // -----------------------------------------------------------------------------
    void Start();

    // -----------------------------------------------------------------------------
    // on_GUI_Update
    //
    // Description: Handles GUI update events, updates components and manages
    //   palette changes.
    // -----------------------------------------------------------------------------
    void on_GUI_Update() override;

    // =============================================================================
    // Component Activation
    // =============================================================================

    // -----------------------------------------------------------------------------
    // activeMainComponent
    //
    // Description: Activates a new main component, deactivating previous one.
    // -----------------------------------------------------------------------------
    void activeMainComponent(iUIComponent* pMainComponent);

    // -----------------------------------------------------------------------------
    // activeBackComponent
    //
    // Description: Activates a new background component, deactivating previous one.
    // -----------------------------------------------------------------------------
    void activeBackComponent(iUIComponent* pBackComponent);

    // =============================================================================
    // Parameter Change notification
    // =============================================================================

    // -----------------------------------------------------------------------------
    // RegisterParameterListener
    //   Adds a callback + context to be notified on parameter changes
    // -----------------------------------------------------------------------------
    void RegisterParameterListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext){
        m_ParameterCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -----------------------------------------------------------------------------
    // NotifyParamChange
    //   Broadcasts parameter change to all registered listeners
    //   pParameter   The modified parameter (passed as void* to callbacks)
    // -----------------------------------------------------------------------------
    void NotifyParamChange(cParameterView* pParameter){
        m_ParameterCallBackIterator.NotifyListeners((void*)pParameter);
    }

    // =============================================================================
    // Notification to restore slot operation
    // =============================================================================

    // -----------------------------------------------------------------------------
    // RegisterStartRestoreListener
    //   Adds a callback + context to be notified on start of a backup slot restoration operation
    // -----------------------------------------------------------------------------
    void RegisterStartRestoreListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext){
        m_StartRestoreCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -----------------------------------------------------------------------------
    // NotifyStartRestore
    //   Description: called by the memory manager at the start of a backup slot restoration operation
    // -----------------------------------------------------------------------------
    void NotifyStartRestore(uint32_t Slot){
        m_StartRestoreCallBackIterator.NotifyListeners(&Slot);
    }

    // -----------------------------------------------------------------------------
    // RegisterEndRestoreListener
    //   Adds a callback + context to be notified on end of a backup slot restoration operation
    // -----------------------------------------------------------------------------
    void RegisterEndRestoreListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext){
        m_EndRestoreCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -----------------------------------------------------------------------------
    // NotifyEndRestore
    //   Description: called by the memory manager at the end of a backup slot restoration operation
    // -----------------------------------------------------------------------------
    void NotifyEndRestore(uint32_t ID){
        m_EndRestoreCallBackIterator.NotifyListeners(&ID);
    }

    // =============================================================================
    // Font Accessors
    // =============================================================================

    // Provides type-safe access to all predefined font sizes and styles
    inline DadGFX::cFont* GetFontXXS()   const { return m_pFontXXS;   }   // Extra extra small font
    inline DadGFX::cFont* GetFontXXSB()  const { return m_pFontXXSB;  }   // Extra extra small bold font
    inline DadGFX::cFont* GetFontXS()    const { return m_pFontXS;    }   // Extra small font
    inline DadGFX::cFont* GetFontXSB()   const { return m_pFontXSB;   }   // Extra small bold font
    inline DadGFX::cFont* GetFontS()     const { return m_pFontS;     }   // Small font
    inline DadGFX::cFont* GetFontSB()    const { return m_pFontSB;    }   // Small bold font
    inline DadGFX::cFont* GetFontM()     const { return m_pFontM;     }   // Medium font
    inline DadGFX::cFont* GetFontMB()    const { return m_pFontMB;    }   // Medium bold font
    inline DadGFX::cFont* GetFontL()     const { return m_pFontL;     }   // Large font
    inline DadGFX::cFont* GetFontLB()    const { return m_pFontLB;    }   // Large bold font
    inline DadGFX::cFont* GetFontXL()    const { return m_pFontXL;    }   // Extra large font
    inline DadGFX::cFont* GetFontXLB()   const { return m_pFontXLB;   }   // Extra large bold font
    inline DadGFX::cFont* GetFontXXL()   const { return m_pFontXXL;   }   // Extra extra large font
    inline DadGFX::cFont* GetFontXXLB()  const { return m_pFontXXLB;  }   // Extra extra large bold font
    inline DadGFX::cFont* GetFontXXXL()  const { return m_pFontXXXL;  }   // Extra extra extra large font
    inline DadGFX::cFont* GetFontXXXLB() const { return m_pFontXXXLB; }   // Extra extra extra large bold font

protected:
    // =============================================================================
    // Protected Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // MIDI_On_CallBack
    //
    // Description: MIDI callback for system ON command.
    // -----------------------------------------------------------------------------
    static void MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -----------------------------------------------------------------------------
    // MIDI_Off_CallBack
    //
    // Description: MIDI callback for system OFF command.
    // -----------------------------------------------------------------------------
    static void MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -----------------------------------------------------------------------------
    // MIDI_ByPass_CallBack
    //
    // Description: MIDI callback for system BYPASS command.
    // -----------------------------------------------------------------------------
    static void MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData);

private:
    // =============================================================================
    // Private Member Variables
    // =============================================================================

    // -----------------------------------------------------------------------------
    // UI Components
    // -----------------------------------------------------------------------------
    iUIComponent* m_pMainComponent;  // Currently active main component
    iUIComponent* m_pBackComponent;  // Currently active background component

    // -----------------------------------------------------------------------------
    // Update Management
    // -----------------------------------------------------------------------------
    uint32_t m_updateID;            // Active update family ID
    uint32_t m_fastUpdateID;        // Active fast update family ID

    // -----------------------------------------------------------------------------
    // Parameter Change notification
    // -----------------------------------------------------------------------------
    DadUtilities::cCallBackIterator m_ParameterCallBackIterator;  // Iterator for parameter change listeners

    // -----------------------------------------------------------------------------
    // End restore memory slot change notification
    // -----------------------------------------------------------------------------
    DadUtilities::cCallBackIterator m_EndRestoreCallBackIterator;  // Iterator for end restore listeners

    // -----------------------------------------------------------------------------
    // Start restore memory slot change notification
    // -----------------------------------------------------------------------------
    DadUtilities::cCallBackIterator m_StartRestoreCallBackIterator;  // Iterator for start restore listeners

    // -----------------------------------------------------------------------------
    // Serialization Management
    // -----------------------------------------------------------------------------
    uint32_t m_SerializeID;                                         // Current serialization family ID

    // -----------------------------------------------------------------------------
    // Font Resources
    // -----------------------------------------------------------------------------
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
