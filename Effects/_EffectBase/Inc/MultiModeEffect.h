#pragma once
//==================================================================================
//==================================================================================
// File: MultiModeEffect.h
// Description: Multi-mode audio effect base classes and main controller interface
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "GUI_Include.h"
#include "cPanelOfSystemView.h"
#include "cPanelOfEffectChoice.h"
#include "cPanelOfTone.h"
#include "cDryWet.h"

namespace DadEffect {

//**********************************************************************************
// Class: cMultiModeEffect
// Description: Base class for multi-mode audio effects
//**********************************************************************************
class cMultiModeEffect{
public:
    // -----------------------------------------------------------------------------
    // Constructor
    //
    cMultiModeEffect() = default;

    // -----------------------------------------------------------------------------
    // Destructor
    //
    virtual ~cMultiModeEffect() = default;

    // -----------------------------------------------------------------------------
    // Init
    // Description: Initializes the effect and its menu system
    //
    void Initialize();

    // -----------------------------------------------------------------------------
    // Activate
    // Description: Activates the effect and sets up GUI components
    //
    void Activate();

    // -----------------------------------------------------------------------------
    // Desactivate
    // Description: Deactivates the effect and performs cleanup
    //
    void Desactivate();

    // =============================================================================
    // Virtual Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // onInitialize
    // Description: Pure virtual method for effect-specific initialization
    //
    virtual void onInitialize() = 0;

    // -----------------------------------------------------------------------------
    // onActivate
    // Description: Pure virtual method for effect-specific activation
    //
    virtual void onActivate() = 0;

    // -----------------------------------------------------------------------------
    // onDesactivate
    // Description: Pure virtual method for effect-specific deactivation
    //
    virtual void onDesactivate() = 0;

    // -----------------------------------------------------------------------------
    // Process
    // Description: Pure virtual method for real-time audio processing
    //
    virtual void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence) = 0;

    // =============================================================================
    // Getter Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // getShortName
    // Description: Returns the short name identifier of the effect
    //
    inline const char * getShortName() const{
        return m_pShortName;
    }

    // -----------------------------------------------------------------------------
    // getLongName
    // Description: Returns the long descriptive name of the effect
    //
    inline const char * getLongName() const{
        return m_pLongName;
    }

    // -----------------------------------------------------------------------------
    // getMenu
    // Description: Returns pointer to the effect's menu object
    //
    inline DadGUI::cUIMenu* getMenu(){
        return &m_Menu;
    }

    // -----------------------------------------------------------------------------
    // getID
    // Description: Returns the unique identifier of the effect
    //
    inline uint32_t getID(){
        return m_ID;
    }

protected:
    // -----------------------------------------------------------------------------
    // Protected Member Data
    //
    uint32_t            m_ID;               // Unique effect identifier
    DadGUI::cUIMenu     m_Menu;             // Effect menu system
    const char*         m_pShortName;       // Short name identifier
    const char*         m_pLongName;        // Long descriptive name
};

//**********************************************************************************
// Class: cMainMultiModeEffect
// Description: Main controller for managing multiple effects and UI components
//**********************************************************************************
class cMainMultiModeEffect{
public:
    // -----------------------------------------------------------------------------
    // Initialize
    // Description: Initializes all effects, panels, and GUI components
    //
    void Initialize();

    // -----------------------------------------------------------------------------
    // Process
    // Description: Real-time audio processing delegate to active effect
    //
    ITCM void Process(AudioBuffer* pIn, AudioBuffer* pOut, eOnOff OnOff, bool Silence);

    // -----------------------------------------------------------------------------
    // getEffect
    // Description: Retrieves effect pointer by index with bounds checking
    //
    cMultiModeEffect* getEffect(uint8_t NumEffect) const;

protected:
    // -----------------------------------------------------------------------------
    // EffectChange
    // Description: Static callback for effect selection change
    //
    static void EffectChange(DadDSP::cParameter *pParameter, uint32_t CallbackUserData);

    // -----------------------------------------------------------------------------
    // setEffect
    void setEffect(uint8_t IndexEffect);

    // -----------------------------------------------------------------------------
    // Protected Member Data
    //
    DadGUI::cUIMemory               m_MemoryPanel;          // Memory management panel
    DadGUI::cUIVuMeter              m_VuMeterPanel;         // Audio level display panel
    DadGUI::cPanelOfSystemView      m_PanelOfSystemView;    // System information panel
    DadGUI::cPanelOfEffectChoice    m_PanelOfEffectChoice;  // Effect selection panel
    DadGUI::cPanelOfTone			m_PanelOfTone;			// Tone panel

    DadGUI::cInfoView               m_InfoView;             // Information display view
    DadGUI::cSwitchOnOff            m_SwitchOnOff;          // On/off switch controller
    DadGUI::cTapTempoMemChange      m_SwitchTempoMem;       // Tempo/memory switch controller

    cMultiModeEffect*               m_pActiveEffect = nullptr;  // Currently active effect
    cMultiModeEffect**              m_pTabEffects = nullptr;    // Array of available effects
    uint8_t                         m_NbEffects = 0;            // Number of available effects

    uint8_t							m_IndexChange = 0;          // Index of changing effect
};

} // namespace DadEffect

//***End of file**************************************************************
