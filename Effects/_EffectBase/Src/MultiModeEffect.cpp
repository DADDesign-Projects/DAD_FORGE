//==================================================================================
//==================================================================================
// File: MultiModeEffect.cpp
// Description: Multi-mode effect implementation with menu management and effect switching
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "MultiModeEffect.h"
#include "GPIO.h"

namespace DadEffect {

//**********************************************************************************
// Class: cMultiModeEffect
// Description: Base class for multi-mode audio effects with menu management
//**********************************************************************************

    // -----------------------------------------------------------------------------
    // Init
    // Description: Initializes the effect and its menu system
    //
    void cMultiModeEffect::Initialize(){
        m_Menu.Init();          // Initialize menu system
        onInitialize();         // Call derived class initialization
    }

    // -----------------------------------------------------------------------------
    // Activate
    // Description: Activates the effect and sets up GUI components
    //
    void cMultiModeEffect::Activate(){
        __GUI.setUpdateID(m_ID);            // Set GUI update identifier
        __GUI.setSerializeID(m_ID);         // Set serialization identifier
        __GUI.setRtProcessID(m_ID);         // Set real-time processing identifier
        __GUI.activeMainComponent(&m_Menu); // Activate main menu component
        onActivate();                       // Call derived class activation
    }

    // -----------------------------------------------------------------------------
    // Desactivate
    // Description: Deactivates the effect and performs cleanup
    //
    void cMultiModeEffect::Desactivate(){
        onDesactivate();        // Call derived class deactivation
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
        // Initialize effect selection panel with callback
        m_PanelOfEffectChoice.Initialize(0, EffectChange, (uint32_t) this);
        m_VuMeterPanel.Init();                              // VU meter display
        m_PanelOfSystemView.Initialize(0); 					// System view panel

        // Iterate through all available effects and set up their menus
        uint8_t NumEffect = 0;              // Effect counter
        cMultiModeEffect* pEffect;          // Current effect pointer
        while(nullptr != (pEffect = getEffect(NumEffect++))){
            pEffect->Initialize();          // Initialize the effect
            m_PanelOfEffectChoice.addEffect(pEffect->getShortName(), pEffect->getLongName());
            m_PanelOfSystemView.addToSerializeFamily(pEffect->getID());
            m_PanelOfEffectChoice.addToSerializeFamily(pEffect->getID());
            __GUI.addSerializeObject(&m_PanelOfEffectChoice, pEffect->getID());
            // Set up menu structure for each effect
            DadGUI::cUIMenu* pMenu = pEffect->getMenu();
            pMenu->addMenuItem(&m_MemoryPanel,          "Memory");      // Memory management
            pMenu->addMenuItem(&m_VuMeterPanel,         "Vu-Meter");    // Audio level display
            pMenu->addMenuItem(&m_PanelOfSystemView,    "System");      // System information
            pMenu->addMenuItem(&m_PanelOfEffectChoice,  "Effect");      // Effect selection
        }

        m_IndexChange = 0xFF;

        // Initialize various UI panels
        m_MemoryPanel.Init(m_pActiveEffect->getID());       // Memory panel with active effect ID

        // Initialize info view and hardware switches
        m_InfoView.Init();                                  // Information display
        m_SwitchOnOff.Init(&__Switch1, 0);                  // On/off switch
        m_SwitchTempoMem.Init(&__Switch2, nullptr, 0, DadGUI::eTempoType::none); // Tempo/memory switch

        // Set up GUI identifiers and activate components
        __GUI.setUpdateID(m_pActiveEffect->getID());        // Set update ID to active effect
        __GUI.setSerializeID(m_pActiveEffect->getID());     // Set serialization ID
        __GUI.setRtProcessID(m_pActiveEffect->getID());     // Set real-time processing ID

        __GUI.activeBackComponent(&m_InfoView);             // Activate background info view
        m_pActiveEffect->Activate();                        // Activate current effect
        __GUI.activeMainComponent(m_pActiveEffect->getMenu()); // Activate effect menu
        SetPIN(AUDIO_MUTE);                                 // Mute audio during setup
    }

    // -----------------------------------------------------------------------------
    // Process
    // Description: Real-time audio processing delegate to active effect
    //
    ITCM void cMainMultiModeEffect::Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff){
    	if(m_IndexChange < 0xFF){
    		__DryWet.forceMix(0.0f);
            // Apply state change only when input is near silence to avoid clicks
            if ((fabs(pIn->Right + pIn->Left) < 0.001f)) {
        		setEffect(m_IndexChange);
        		m_IndexChange = 0xFF;
        	}
    	}else{
    		__DryWet.Process(OnOff == eOnOff::On);
    		m_pActiveEffect->Process(pIn, pOut, OnOff); // Delegate processing to active effect
    	}
    }

    // -----------------------------------------------------------------------------
    // EffectChange
    // Description: Callback for effect selection change
    //
    void cMainMultiModeEffect::EffectChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData) {
        cMainMultiModeEffect *pthis = (cMainMultiModeEffect *)CallbackUserData; // Get instance pointer
        pthis->m_IndexChange = pParameter->getValue();          				// Get new effect
    }

    // -----------------------------------------------------------------------------
    // setEffect
    void cMainMultiModeEffect::setEffect(uint8_t IndexEffect){
        if(m_pActiveEffect) m_pActiveEffect->Desactivate();     // Deactivate current effect
        m_pActiveEffect = getEffect(IndexEffect);        		// Get new effect
        m_MemoryPanel.setSerializeID(m_pActiveEffect->getID());
        m_pActiveEffect->Activate();                            // Activate new effect
    }

    // -----------------------------------------------------------------------------
    // getEffect
    // Description: Retrieves effect pointer by index with bounds checking
    //
    cMultiModeEffect* cMainMultiModeEffect::getEffect(uint8_t NumEffect) const{
        if(NumEffect >= m_NbEffects){
            return nullptr;     // Return null if index out of bounds
        }else{
            return m_pTabEffects[NumEffect]; // Return effect pointer
        }
    }

} // namespace DadEffect

//***End of file**************************************************************
