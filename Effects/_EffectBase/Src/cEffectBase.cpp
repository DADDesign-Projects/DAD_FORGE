//==================================================================================
//==================================================================================
// File: cEffectBase.cpp
// Description: Base effect implementation for DSP audio processing
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "cEffectBase.h"
#include "GPIO.h"
#include "DadUtilities.h"

namespace DadEffect {

// -----------------------------------------------------------------------------
// Initializes DSP components and user interface parameters
void cEffectBase::Initialize()
{
    m_Menu.Init();
    m_pTapTempoParameter = nullptr;
    onInitialize();
    uint32_t EffectID = getEffectID();

    // Initialize parameter panels
    m_MemoryPanel.Init(EffectID);
    m_VuMeterPanel.Init();
    m_PanelOfSystemView.Initialize(EffectID);

    // Initialize UI components
    m_InfoView.Init();
    m_SwitchOnOff.Init(&__Switch1, EffectID);
    if (m_pTapTempoParameter == nullptr) m_TempoType = DadGUI::eTempoType::none;
    m_SwitchTempoMem.Init(&__Switch2, m_pTapTempoParameter, EffectID, m_TempoType);

    // Build menu structure
    m_Menu.addMenuItem(&m_MemoryPanel,           "Memory");
    m_Menu.addMenuItem(&m_VuMeterPanel,          "Vu-Meter");
    m_Menu.addMenuItem(&m_PanelOfSystemView,     "System");

    // Configure GUI identifiers and components
    __GUI_EventManager.SetActiveFamily4AllEvents(EffectID);

    __GUI.activeBackComponent(&m_InfoView);
    __GUI.activeMainComponent(&m_Menu);

    // Initialize audio processing settings
    __DryWet.setMix(0);

    m_cParameterInfoView.Init();

    // Register memory restore event listeners
    __GUI.RegisterStartRestoreListener(StartRestoreEvent, (uint32_t)this);
    __GUI.RegisterEndRestoreListener(EndRestoreEvent, (uint32_t)this);

    // Subscribe to fast GUI update events
    __GUI_EventManager.Subscribe_FastUpdate(this);
}

// -----------------------------------------------------------------------------
// Audio processing function: processes one input/output audio buffer
void cEffectBase::Process(AudioBuffer *pIn, AudioBuffer *pOut, eOnOff OnOff, bool Silence)
{
    AudioBuffer OutEffect;

    // Process audio through child effect
    onProcess(pIn, &OutEffect, OnOff, Silence);

    // Apply wet gain fade for smooth effect/memory switching
    if (!isZero(m_FadeIncrement)) {
        m_FadGain += m_FadeIncrement;           // Update fade gain

        // Handle fade boundaries
        if (m_FadGain <= 0.0f) {
            m_FadGain = 0.0f;                   // Clamp to minimum
            m_FadeIncrement = 0.0f;             // Stop fading
            m_ChangeEffect = true;              // Trigger effect change at zero crossing
        } else if (m_FadGain >= 1.0) {
            m_FadGain = 1.0f;                   // Clamp to maximum
            m_FadeIncrement = 0.0f;             // Stop fading
        }
    }

    // Apply faded gain to effect output
    pOut->Left = OutEffect.Left * m_FadGain;
    pOut->Right = OutEffect.Right * m_FadGain;
}

constexpr float FADE_TIME = 0.200f;                     // Fade duration in seconds
constexpr float FADE_INCREMENT = 1 / (SAMPLING_RATE * FADE_TIME); // Per-sample fade increment

// -----------------------------------------------------------------------------
// Static callback for memory restore start event
// Triggered when memory manager begins restoring a preset
void cEffectBase::StartRestoreEvent(void *pTargetSlot, uint32_t Data)
{
    // Recover instance pointer from user data
    cEffectBase *pthis = (cEffectBase *)Data;

    // Extract target memory slot
    pthis->m_TargetSlot = (uint8_t) * ((uint32_t *)pTargetSlot);

    // Start fade-out before memory restore
    pthis->m_FadeIncrement = -FADE_INCREMENT;
}

// -----------------------------------------------------------------------------
// Static callback for memory restore end event
// Triggered by memory manager after preset restore completes
void cEffectBase::EndRestoreEvent(void *pID, uint32_t Data)
{
    // Recover instance pointer from user data
    cEffectBase *pthis = (cEffectBase *)Data;

    // Start fade-in
    pthis->m_FadeIncrement = FADE_INCREMENT;
}

// -----------------------------------------------------------------------------
// Periodically updates switch state and detects user actions
// Called at fast GUI update rate
void cEffectBase::on_GUI_FastUpdate()
{
    if (m_ChangeEffect) {
        // Restore memory slot if target is specified
        DadGUI::__MemoryManager.RestoreSlot(m_TargetSlot);
        m_FadeIncrement = FADE_INCREMENT;       // Start fade-in
        m_ChangeEffect = false;                 // Clear pending flag
    }
}

} // namespace DadEffect

//***End of file**************************************************************
