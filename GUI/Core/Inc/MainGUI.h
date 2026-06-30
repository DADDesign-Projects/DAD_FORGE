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
#include "GUI_Defines.h"
#include "cThemesManager.h"

namespace DadGUI {

// =============================================================================
// Global variables
// =============================================================================

// -----------------------------------------------------------------------------
// Themes manager
extern cThemesManager __ThemesManager;

// -----------------------------------------------------------------------------
// Slot Memory Manager
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

    // -----------------------------------------------------------------------------
    // ThemeChange_CallBack
    //
    // Description: callback for theme change notification
    // -----------------------------------------------------------------------------
    static void ThemeChange_CallBack(void* parameter, uint32_t contextValue);


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
