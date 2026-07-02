//==================================================================================
//==================================================================================
// File: MainGUI.h
// Description: Core infrastructure for graphical user interface management
//
// Copyright (c) 2025 DadDesign-Projects.
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
#include "cMonitor.h"

namespace DadGUI {

//----------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------

extern cThemesManager __ThemesManager;          // Themes manager instance
extern GUI_EventManager __GUI_EventManager;     // Event manager instance
extern cMemoryManager __MemoryManager;          // Global memory manager instance


class cParameterView;

//**********************************************************************************
// Class: cMainGUI
//
// Description:
//   Main GUI controller class that manages components, event handling,
//   and system-wide GUI operations.
//**********************************************************************************

class cMainGUI
{
public:
    virtual ~cMainGUI() = default;

    // -------------------------------------------------------------------------
    // Public Methods
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Initialize
    //
    // Description: Prepares the GUI system by initializing fonts, palettes,
    //   and layout data. Allocates font objects from binary font files stored
    //   in flash memory.
    // -------------------------------------------------------------------------
    void Initialize();

    // -------------------------------------------------------------------------
    // Start
    //
    // Description: Initializes memory management and sets up system callbacks.
    // -------------------------------------------------------------------------
    void Start();

    // -------------------------------------------------------------------------
    // activeMainComponent
    //
    // Description: Activates a new main component, deactivating previous one.
    //   Only one main component can be active at a time.
    // -------------------------------------------------------------------------
    void activeMainComponent(iUIComponent* pMainComponent);

    // -------------------------------------------------------------------------
    // activeBackComponent
    //
    // Description: Activates a new background component, deactivating previous one.
    //   Only one background component can be active at a time.
    // -------------------------------------------------------------------------
    void activeBackComponent(iUIComponent* pBackComponent);

    // -------------------------------------------------------------------------
    // RegisterParameterListener
    //
    // Description: Adds a callback + context to be notified on parameter changes.
    // -------------------------------------------------------------------------
    void RegisterParameterListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext)
    {
        m_ParameterCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -------------------------------------------------------------------------
    // NotifyParamChange
    //
    // Description: Broadcasts parameter change to all registered listeners.
    //   pParameter - The modified parameter (passed as void* to callbacks).
    // -------------------------------------------------------------------------
    void NotifyParamChange(cParameterView* pParameter)
    {
        m_ParameterCallBackIterator.NotifyListeners((void*)pParameter);
    }

    // -------------------------------------------------------------------------
    // RegisterStartRestoreListener
    //
    // Description: Adds a callback + context to be notified on start of a
    //   backup slot restoration operation.
    // -------------------------------------------------------------------------
    void RegisterStartRestoreListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext)
    {
        m_StartRestoreCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -------------------------------------------------------------------------
    // NotifyStartRestore
    //
    // Description: Called by the memory manager at the start of a backup slot
    //   restoration operation.
    // -------------------------------------------------------------------------
    void NotifyStartRestore(uint32_t Slot)
    {
        m_StartRestoreCallBackIterator.NotifyListeners(&Slot);
    }

    // -------------------------------------------------------------------------
    // RegisterEndRestoreListener
    //
    // Description: Adds a callback + context to be notified on end of a
    //   backup slot restoration operation.
    // -------------------------------------------------------------------------
    void RegisterEndRestoreListener(DadUtilities::IteratorCallback_t Callback, uint32_t ListenerContext)
    {
        m_EndRestoreCallBackIterator.RegisterListener(Callback, ListenerContext);
    }

    // -------------------------------------------------------------------------
    // NotifyEndRestore
    //
    // Description: Called by the memory manager at the end of a backup slot
    //   restoration operation.
    // -------------------------------------------------------------------------
    void NotifyEndRestore(uint32_t ID)
    {
        m_EndRestoreCallBackIterator.NotifyListeners(&ID);
    }

    // -------------------------------------------------------------------------
    // Font Accessors
    // -------------------------------------------------------------------------

    inline DadGFX::cFont* GetFontXXS()   const { return m_pFontXXS;   } // Extra extra small font
    inline DadGFX::cFont* GetFontXXSB()  const { return m_pFontXXSB;  } // Extra extra small bold font
    inline DadGFX::cFont* GetFontXS()    const { return m_pFontXS;    } // Extra small font
    inline DadGFX::cFont* GetFontXSB()   const { return m_pFontXSB;   } // Extra small bold font
    inline DadGFX::cFont* GetFontS()     const { return m_pFontS;     } // Small font
    inline DadGFX::cFont* GetFontSB()    const { return m_pFontSB;    } // Small bold font
    inline DadGFX::cFont* GetFontM()     const { return m_pFontM;     } // Medium font
    inline DadGFX::cFont* GetFontMB()    const { return m_pFontMB;    } // Medium bold font
    inline DadGFX::cFont* GetFontL()     const { return m_pFontL;     } // Large font
    inline DadGFX::cFont* GetFontLB()    const { return m_pFontLB;    } // Large bold font
    inline DadGFX::cFont* GetFontXL()    const { return m_pFontXL;    } // Extra large font
    inline DadGFX::cFont* GetFontXLB()   const { return m_pFontXLB;   } // Extra large bold font
    inline DadGFX::cFont* GetFontXXL()   const { return m_pFontXXL;   } // Extra extra large font
    inline DadGFX::cFont* GetFontXXLB()  const { return m_pFontXXLB;  } // Extra extra large bold font
    inline DadGFX::cFont* GetFontXXXL()  const { return m_pFontXXXL;  } // Extra extra extra large font
    inline DadGFX::cFont* GetFontXXXLB() const { return m_pFontXXXLB; } // Extra extra extra large bold font

    // -------------------------------------------------------------------------
    // MainLoop
    //
    // Description: Main GUI application loop. Handles GUI updates, MIDI
    //   processing, and system monitoring.
    // -------------------------------------------------------------------------
    void MainLoop();

    // -------------------------------------------------------------------------
    // notifyRTActivity
    //
    // Description: Increments the real-time activity counter used for LED
    //   blinking and system status indication.
    // -------------------------------------------------------------------------
    inline void notifyRTActivity()
    {
        m_CtRTActivity++;
    }

#ifdef MONITOR
    // -------------------------------------------------------------------------
    // startRTMonitoring
    //
    // Description: Starts real-time performance monitoring.
    // -------------------------------------------------------------------------
    inline void startRTMonitoring()
    {
        m_Monitor.startMonitoring();
    }

    // -------------------------------------------------------------------------
    // stopRTMonitoring
    //
    // Description: Stops real-time performance monitoring.
    // -------------------------------------------------------------------------
    inline void stopRTMonitoring()
    {
        m_Monitor.stopMonitoring();
    }

    // -------------------------------------------------------------------------
    // getCPULoad
    //
    // Description: Returns the current CPU load percentage.
    // -------------------------------------------------------------------------
    inline float getCPULoad()
    {
        return m_CPULoad;
    }

    // -------------------------------------------------------------------------
    // getEffectTime
    //
    // Description: Returns the average effect execution time in microseconds.
    // -------------------------------------------------------------------------
    inline float getEffectTime()
    {
        return m_EffectTime;
    }

    // -------------------------------------------------------------------------
    // getFrequency
    //
    // Description: Returns the average processing frequency in Hertz.
    // -------------------------------------------------------------------------
    inline float getFrequency()
    {
        return m_Frequency;
    }
#endif

protected:
    // -------------------------------------------------------------------------
    // Protected Methods
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // MIDI_On_CallBack
    //
    // Description: MIDI callback for system ON command.
    // -------------------------------------------------------------------------
    static void MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_Off_CallBack
    //
    // Description: MIDI callback for system OFF command.
    // -------------------------------------------------------------------------
    static void MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // MIDI_ByPass_CallBack
    //
    // Description: MIDI callback for system BYPASS command.
    // -------------------------------------------------------------------------
    static void MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // -------------------------------------------------------------------------
    // ThemeChange_CallBack
    //
    // Description: Callback for theme change notification.
    // -------------------------------------------------------------------------
    static void ThemeChange_CallBack(void* parameter, uint32_t contextValue);

private:
    // -------------------------------------------------------------------------
    // Private Member Variables
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // UI Components
    // -------------------------------------------------------------------------

    iUIComponent* m_pMainComponent;          // Currently active main component
    iUIComponent* m_pBackComponent;          // Currently active background component

    // -------------------------------------------------------------------------
    // Update Management
    // -------------------------------------------------------------------------

    uint32_t m_updateID;                     // Active update family ID
    uint32_t m_fastUpdateID;                 // Active fast update family ID

    // -------------------------------------------------------------------------
    // Parameter Change Notification
    // -------------------------------------------------------------------------

    DadUtilities::cCallBackIterator m_ParameterCallBackIterator;  // Iterator for parameter change listeners

    // -------------------------------------------------------------------------
    // Restore Notification
    // -------------------------------------------------------------------------

    DadUtilities::cCallBackIterator m_EndRestoreCallBackIterator;    // Iterator for end restore listeners
    DadUtilities::cCallBackIterator m_StartRestoreCallBackIterator;  // Iterator for start restore listeners

    // -------------------------------------------------------------------------
    // Serialization Management
    // -------------------------------------------------------------------------

    uint32_t m_SerializeID;                  // Current serialization family ID

    // -------------------------------------------------------------------------
    // Font Resources
    // -------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------
    // Monitoring Variables
    // -------------------------------------------------------------------------

    uint32_t m_CtRTActivity;                 // Real-time activity counter for LED blinking

#ifdef MONITOR
    DadUtilities::cMonitor m_Monitor;        // Performance monitor instance
    float m_CPULoad;                         // Current CPU load percentage
    float m_EffectTime;                      // Average effect execution time (us)
    float m_Frequency;                       // Average processing frequency (Hz)
#endif
};

} // namespace DadGUI

//***End of file**************************************************************
