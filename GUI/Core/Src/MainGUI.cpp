//==================================================================================
//==================================================================================
// File: MainGUI.cpp
// Description: Implementation of the main GUI controller class
//
// Copyright (c) 2025 DadDesign-Projects.
//==================================================================================
//==================================================================================

#include "HardwareAndCo.h"
#include "cPaletteBuilder.h"
#include "GPIO.h"

namespace DadGUI {

//----------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------

cThemesManager   __ThemesManager;        // Themes manager instance
GUI_EventManager __GUI_EventManager;     // Event manager instance
cMemoryManager   __MemoryManager;        // Global memory manager instance


//**********************************************************************************
// Class: cMainGUI
//
// Description:
//   Implementation of main GUI controller methods.
//**********************************************************************************

//----------------------------------------------------------------------------
// System Initialization
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Initialize
//
// Description: Prepares the GUI system by initializing fonts, palettes,
//   and layout data. Allocates font objects from binary font files stored
//   in flash memory.
//----------------------------------------------------------------------------
void cMainGUI::Initialize()
{
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
    m_pMainComponent = nullptr;
    m_pBackComponent = nullptr;

    // Initialize state variables
    __GUI_EventManager.Clear();
    __GUI_EventManager.Subscribe_Update(this);

    __ThemesManager.Initialize();
    __ThemesManager.RegisterThemeChangeListener(ThemeChange_CallBack, (uint32_t) this);

    m_CtRTActivity = 0;
#ifdef MONITOR
    m_Monitor.Init();
    m_CPULoad = 0;
    m_EffectTime = 0;
    m_Frequency = 0;
#endif
}

//----------------------------------------------------------------------------
// Start
//
// Description: Initializes the memory manager and sets up MIDI callbacks
//   for preset management and system control commands.
//----------------------------------------------------------------------------
void cMainGUI::Start()
{
    // Initialize memory management system
    __MemoryManager.Init();

    // Register MIDI callbacks for preset and system control
    __Midi.addControlChangeCallback(MIDI_CC_PRESET_UP, reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_PresetUp_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_PRESET_DOWN, reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_PresetDown_CallBack);
    __Midi.addProgramChangeCallback(reinterpret_cast<uint32_t>(&__MemoryManager), &cMemoryManager::MIDI_ProgramChange_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_ON, 0, &MIDI_On_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_OFF, 0, &MIDI_Off_CallBack);
    __Midi.addControlChangeCallback(MIDI_CC_BYPASS, 0, &MIDI_ByPass_CallBack);
}

//----------------------------------------------------------------------------
// Update Management
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// on_GUI_Update
//
// Description: Processes GUI update events, updates components and manages
//   palette changes.
//----------------------------------------------------------------------------
void cMainGUI::on_GUI_Update()
{
    // Update main UI components
    if (m_pBackComponent) m_pBackComponent->Update();
    if (m_pMainComponent) m_pMainComponent->Update();
}

//----------------------------------------------------------------------------
// Component Activation
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// activeMainComponent
//
// Description: Activates a new main component, deactivating the previous
//   one if exists. Only one main component can be active at a time.
//----------------------------------------------------------------------------
void cMainGUI::activeMainComponent(iUIComponent* pMainComponent)
{
    // Deactivate current component if exists
    if (m_pMainComponent) m_pMainComponent->Deactivate();

    // Activate new component if provided
    if (pMainComponent)
    {
        m_pMainComponent = pMainComponent;
        m_pMainComponent->Activate();
    }
}

//----------------------------------------------------------------------------
// activeBackComponent
//
// Description: Activates a new background component, deactivating the
//   previous one if exists. Only one background component can be active at a time.
//----------------------------------------------------------------------------
void cMainGUI::activeBackComponent(iUIComponent* pBackComponent)
{
    // Deactivate current component if exists
    if (m_pBackComponent) m_pBackComponent->Deactivate();

    // Activate new component if provided
    if (pBackComponent)
    {
        m_pBackComponent = pBackComponent;
        m_pBackComponent->Activate();
    }
}

//----------------------------------------------------------------------------
// ThemeChange_CallBack
//
// Description: Callback for theme change notification.
//----------------------------------------------------------------------------
void cMainGUI::ThemeChange_CallBack(void* parameter, uint32_t contextValue)
{
    cMainGUI* pThis = (cMainGUI*)contextValue;
    if (pThis->m_pBackComponent) pThis->m_pBackComponent->Redraw();
    if (pThis->m_pMainComponent) pThis->m_pMainComponent->Redraw();
}


//**********************************************************************************
// MainLoop
// Main GUI application loop
//**********************************************************************************
void cMainGUI::MainLoop()
{
    uint32_t lastGUIFast    = HAL_GetTick();
    uint32_t lastGUI        = HAL_GetTick();
    uint32_t lastMIDI       = HAL_GetTick();
    uint32_t lastGeneral    = HAL_GetTick();

#ifdef MONITOR
    uint32_t lastMonitor    = HAL_GetTick();
#endif

    while (1)
    {
        uint32_t currentTick = HAL_GetTick();

        // 1. GUI Fast Update
        if (currentTick - lastGUIFast >= GUI_FAST_UPDATE_MS)
        {
            lastGUIFast += GUI_FAST_UPDATE_MS;  // Drift correction
            DadGUI::__GUI_EventManager.sendEventToActive_FastUpdate();
        }

        // 2. GUI Standard
        if (currentTick - lastGUI >= GUI_UPDATE_MS)
        {
            lastGUI += GUI_UPDATE_MS;
            DadGUI::__GUI_EventManager.sendEventToActive_Update();
            __Display.flush();
        }

        // 3. MIDI
        if (currentTick - lastMIDI >= MIDI_UPDATE_MS)
        {
            lastMIDI += MIDI_UPDATE_MS;
            __Midi.ProcessBuffer();
            //ProcessMidiFifo();
        }

#ifdef MONITOR
        // 4. Monitoring
        if (currentTick - lastMonitor >= MONITOR_UPDATE_MS)
        {
            lastMonitor += MONITOR_UPDATE_MS;
            m_CPULoad    = m_Monitor.getCPULoad_percent();
            m_EffectTime = m_Monitor.getAverageExecutionTime_us();
            m_Frequency  = m_Monitor.getAverageFrequency_Hz();
            m_Monitor.reset();
        }
#endif

        // 5. General Updates (LED blinking)
        if (currentTick - lastGeneral >= GENERAL_UPDATE_MS)
        {
            lastGeneral += GENERAL_UPDATE_MS;

            // Calculate blink threshold: 0.5 seconds based on audio buffer processing
            constexpr uint32_t LED_BLINK_THRESHOLD = (uint32_t)(((float)SAMPLING_RATE / AUDIO_BUFFER_SIZE) * 0.5f);
            if (m_CtRTActivity >= LED_BLINK_THRESHOLD)
            {
                m_CtRTActivity = 0;
                TogglePIN(LED);
            }
        }
    }
}


//----------------------------------------------------------------------------
// MIDI Callback Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// MIDI_On_CallBack
//
// Description: MIDI callback for system ON command.
//----------------------------------------------------------------------------
void cMainGUI::MIDI_On_CallBack(uint8_t control, uint8_t value, uint32_t userData)
{
    __OnOffCmd = On;  // Set system state to ON
}

//----------------------------------------------------------------------------
// MIDI_Off_CallBack
//
// Description: MIDI callback for system OFF command.
//----------------------------------------------------------------------------
void cMainGUI::MIDI_Off_CallBack(uint8_t control, uint8_t value, uint32_t userData)
{
    __OnOffCmd = Off;  // Set system state to OFF
}

//----------------------------------------------------------------------------
// MIDI_ByPass_CallBack
//
// Description: MIDI callback for system BYPASS command.
//----------------------------------------------------------------------------
void cMainGUI::MIDI_ByPass_CallBack(uint8_t control, uint8_t value, uint32_t userData)
{
    __OnOffCmd = ByPass;  // Set system state to BYPASS
}

} // namespace DadGUI

//***End of file**************************************************************
