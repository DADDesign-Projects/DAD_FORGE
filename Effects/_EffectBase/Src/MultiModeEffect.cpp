//==================================================================================
//==================================================================================
// File: MultiModeEffect.cpp
// Description: Multi-mode effect implementation with menu management and effect switching
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "MultiModeEffect.h"
#include "GPIO.h"
#include "DadUtilities.h"

namespace DadEffect {

//**********************************************************************************
// Class: cMultiModeEffectBase
// Description: Base class for multi-mode audio effects with menu management
//**********************************************************************************

    // -----------------------------------------------------------------------------
    // Initialize
    // Description: Initializes the effect and its menu system
    //
    void cMultiModeEffectBase::Initialize(){
        m_Menu.Init();          // Initialize base menu system
        onInitialize();         // Delegate to derived class initialization
    }

    // -----------------------------------------------------------------------------
    // Activate
    // Description: Activates the effect and sets up GUI components
    //
    void cMultiModeEffectBase::Activate(){
    	__GUI_EventManager.SetActiveFamily4AllEvents(m_ID);    // Set event family for this effect
        __GUI.activeMainComponent(&m_Menu);                    // Set menu as active main component
        onActivate();                                          // Delegate to derived class activation
    }

    // -----------------------------------------------------------------------------
    // Desactivate
    // Description: Deactivates the effect and performs cleanup
    //
    void cMultiModeEffectBase::Desactivate(){
        onDesactivate();        // Delegate to derived class deactivation
    }

//**********************************************************************************
// Class: cMainMultiModeEffect
// Description: Main controller for managing multiple effects and UI components
//**********************************************************************************

    // -----------------------------------------------------------------------------
    // Initialize
    // Description: Initializes all effects, panels, and GUI components
    //
    void cMainMultiModeEffect::Initialize(){
    	// Initialize panels common to all effects
        m_PanelOfEffectChoice.Initialize(0, EffectChange, (uint32_t) this);
        m_VuMeterPanel.Init();                          // Initialize VU meter display
        m_PanelOfSystemView.Initialize(0); 				// Initialize system view panel
        m_PanelOfTone.Initialize(0);					// Initialize tone control panel

        // Iterate through all available effects and set up their menus
        uint8_t NumEffect = 0;              // Effect counter
        cMultiModeEffectBase* pEffect;      // Current effect pointer
        while(nullptr != (pEffect = getEffect(NumEffect++))){

        	// Initialize each effect
        	pEffect->Initialize();          // Initialize effect and its menu

            // Add effect to selection panel
            m_PanelOfEffectChoice.addEffect(pEffect->getShortName(), pEffect->getLongName());

            // Build common menu structure for each effect
            DadGUI::cUIMenu* pMenu = pEffect->getMenu();
            pMenu->addMenuItem(&m_PanelOfTone,          "Tone");      // Tone control menu item
            pMenu->addMenuItem(&m_MemoryPanel,          "Memory");    // Memory management menu item
            pMenu->addMenuItem(&m_VuMeterPanel,         "Vu-Meter");  // VU meter display menu item
            pMenu->addMenuItem(&m_PanelOfSystemView,    "System");    // System information menu item
            pMenu->addMenuItem(&m_PanelOfEffectChoice,  "Effect");    // Effect selection menu item
        }

        // Initialize UI components
        m_MemoryPanel.Init(m_pActiveEffect->getID());       // Initialize memory panel with active effect ID
        m_InfoView.Init();                                  // Initialize information display
        m_SwitchOnOff.Init(&__Switch1, 0);                  // Initialize on/off hardware switch
        m_SwitchTempoMem.Init(&__Switch2, nullptr, 0, DadGUI::eTempoType::none); // Initialize tempo/memory switch
        m_cParameterInfoView.Init();                        // Initialize parameter info view

        // Configure GUI event routing
        __GUI_EventManager.SetActiveFamily4AllEvents(m_pActiveEffect->getID()); // Set event family for active effect

        // Activate GUI components
        __GUI.activeBackComponent(&m_InfoView);             // Set info view as background component
        m_pActiveEffect->Activate();                        // Activate current effect
        __GUI.activeMainComponent(m_pActiveEffect->getMenu()); // Set effect menu as main component

        // Initialize audio processing settings
        __DryWet.setMix(0);                                 // Set initial dry/wet mix to 0%

        // Register memory restore event listeners
        __GUI.RegisterStartRestoreListener(StartRestoreEvent, (uint32_t) this);
        __GUI.RegisterEndRestoreListener(EndRestoreEvent, (uint32_t) this);

        // Subscribe to fast GUI update events
        __GUI_EventManager.Subscribe_FastUpdate(this);
    }

    // -----------------------------------------------------------------------------
    // Process
    // Description: Real-time audio processing delegate to active effect
    //
    void cMainMultiModeEffect::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence){
    		AudioBuffer OutEffect;
    		m_pActiveEffect->Process(pIn, &OutEffect, OnOff, Silence); // Process audio through active effect

    		// Apply wet gain fade for smooth effect/memory switching
        	if(!isZero(m_FadeIncrement)){
        		m_FadGain += m_FadeIncrement;           // Update fade gain

                // Handle fade boundaries
				if(m_FadGain <= 0.0f){
					m_FadGain = 0.0f;                   // Clamp to minimum
					m_FadeIncrement = 0.0f;             // Stop fading
					m_ChangeEffect = true;              // Trigger effect change at zero crossing
				}else if(m_FadGain >= 1.0){
					m_FadGain = 1.0f;                   // Clamp to maximum
					m_FadeIncrement = 0.0f;             // Stop fading
				}
        	}

    		// Apply faded gain to effect output
    		OutEffect.Left *= m_FadGain;
    		OutEffect.Right *= m_FadGain;

    		// Apply tone processing
    		m_PanelOfTone.Process(&OutEffect, pOut);
    }

    constexpr float FADE_TIME = 0.200f;                     // Fade duration in seconds
    constexpr float FADE_INCREMENT = 1/ (SAMPLING_RATE * FADE_TIME); // Per-sample fade increment

    // -----------------------------------------------------------------------------
    // EffectChange
    // Description: Static callback for effect selection change
    //              Triggered by effect choice panel
    //
    void cMainMultiModeEffect::EffectChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
    	// Recover instance pointer from user data
    	cMainMultiModeEffect *pthis = (cMainMultiModeEffect *)CallbackUserData;

    	// Start fade-out and prepare for effect change
    	pthis->m_TargetSlot = 0xFF;                         // Mark as effect change (not memory slot)
    	pthis->m_FadeIncrement = -FADE_INCREMENT;           // Start fade-out
    }

    // -----------------------------------------------------------------------------
    // StartRestoreEvent
    // Description: Static callback for memory restore start event
    //              Triggered when memory manager begins restoring a preset
    //
    void cMainMultiModeEffect::StartRestoreEvent(void *pTargetSlot, uint32_t Data){
    	// Recover instance pointer from user data
    	cMainMultiModeEffect* pthis = (cMainMultiModeEffect*) Data;

    	// Extract target memory slot
    	pthis->m_TargetSlot = (uint8_t) *((uint32_t*)pTargetSlot);

    	// Start fade-out before memory restore
    	pthis->m_FadeIncrement = -FADE_INCREMENT;
    }

    // -----------------------------------------------------------------------------
    // EndRestoreEvent
    // Description: Static callback for memory restore end event
    //              Triggered by memory manager after preset restore completes
    //
    void cMainMultiModeEffect::EndRestoreEvent(void * pID, uint32_t Data){
    	// Recover instance pointer from user data
    	cMainMultiModeEffect* pthis = (cMainMultiModeEffect*) Data;

    	// Switch to the effect associated with restored memory slot
    	pthis->setEffect(pthis->m_PanelOfEffectChoice.getEffect());

    	// Start fade-in
    	pthis->m_FadeIncrement = FADE_INCREMENT;
    }

    // -----------------------------------------------------------------------------
    // on_GUI_FastUpdate
    // Description: Periodically updates switch state and detects user actions
    //              Called at fast GUI update rate
    //
    void cMainMultiModeEffect::on_GUI_FastUpdate(){
    	if(m_ChangeEffect){
     		if(m_TargetSlot != 0xFF){
    			// Restore memory slot if target is specified
    			DadGUI::__MemoryManager.RestoreSlot(m_TargetSlot);
    		}else{
    			// Otherwise just switch effect
    	    	setEffect(m_PanelOfEffectChoice.getEffect());
    		}
	    	m_FadeIncrement = FADE_INCREMENT;   // Start fade-in
	    	m_ChangeEffect = false;             // Clear pending flag
    	}
    }

    // -----------------------------------------------------------------------------
    // setEffect
    // Description: Switches to the specified effect
    //
    void cMainMultiModeEffect::setEffect(uint8_t IndexEffect){
        if(m_pActiveEffect) m_pActiveEffect->Desactivate();     // Deactivate current effect
        m_pActiveEffect = getEffect(IndexEffect);        		// Set new active effect
        m_MemoryPanel.setSerializeID(m_pActiveEffect->getID()); // Update memory panel with new effect ID
        m_pActiveEffect->Activate();                            // Activate new effect
    }

    // -----------------------------------------------------------------------------
    // getEffect
    // Description: Retrieves effect pointer by index with bounds checking
    //
    cMultiModeEffectBase* cMainMultiModeEffect::getEffect(uint8_t NumEffect) const{
        if(NumEffect >= m_NbEffects){
            return nullptr;     // Return null for out-of-range index
        }else{
            return m_pTabEffects[NumEffect]; // Return effect pointer
        }
    }
} // namespace DadEffect

//***End of file**************************************************************
