//==================================================================================
//==================================================================================
// File: cPanelOfEffectChoice.cpp
// Description: Effect choice panel implementation for GUI
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cPanelOfEffectChoice.h"
//#include "HardwareAndCo.h"

namespace DadGUI {

//**********************************************************************************
// Layer declaration
//**********************************************************************************

DECLARE_LAYER(EffectChoiceLayer, PARAM_WIDTH * 2, PARAM_HEIGHT);

//**********************************************************************************
// Public methods
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes the effect choice panel
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Initialize(uint32_t SerializeID,
									  EffectChangeCallback_t Callback,
                                      uint32_t ContextCallback) {
    m_isActive = false;                                 // Initially inactive

    // Initialize callback system
    m_Callback = Callback;
    m_ContextCallback = ContextCallback;

    // Attach display layer
    m_pLayer = ADD_LAYER(__Display, EffectChoiceLayer, PARAM_WIDTH, MENU_HEIGHT, 0);

    // Initialize parameter choice and view
    m_ParameterChoice.Init(SerializeID, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    m_ParameterChoiceView.Init(&m_ParameterChoice, "Effect", "Effect");

    cPanelOfParameterView::Init(&m_ParameterChoiceView, nullptr, nullptr);
}

// -----------------------------------------------------------------------------
// Adds an effect to the choice list
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::addEffect(const char* ShortName, const char* LongName) {
    m_ParameterChoiceView.AddDiscreteValue(ShortName, LongName);
}

// -----------------------------------------------------------------------------
// Activates the effect choice panel
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Activate() {
    m_isActive = true;                                  // Set as active
    m_pLayer->changeZOrder(30);                         // Bring to front
    Draw();                                             // Draw panel contents
    cPanelOfParameterView::Activate();                  // Activate base class
}

// -----------------------------------------------------------------------------
// Deactivates the effect choice panel
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Deactivate() {
    m_isActive = false;                                 // Set as inactive
    m_pLayer->changeZOrder(0);                          // Send to back
    cPanelOfParameterView::Deactivate();                // Deactivate base class
}

// -----------------------------------------------------------------------------
// Updates the panel state
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Update() {
    // Only update if active or callback needed
    if (m_isActive){
        // Process parameter changes
        cPanelOfParameterView::Update();                // Update base class

        // Check for encoder switch activation
        uint8_t SwitchState = __Encoder0.getSwitchState();
        if (SwitchState == 1){
            // Trigger callback if available
            if (m_Callback) {
                m_Callback(&m_ParameterChoice, m_ContextCallback);
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Redraws the panel if active
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Redraw() {
    if (m_isActive) {
        Draw();                                         // Redraw only if active
    }
    cPanelOfParameterView::Redraw();                    // Redraw base class
}

//**********************************************************************************
// Private methods
//**********************************************************************************

// -----------------------------------------------------------------------------
// Draws the panel contents with instructional text
// -----------------------------------------------------------------------------
void cPanelOfEffectChoice::Draw() {
    // Constants for layout and text
    constexpr uint16_t     MidXLayer   = (PARAM_WIDTH);
    constexpr uint16_t     MidYLayer   = (PARAM_HEIGHT / 2);
    constexpr uint16_t     TextYOffset = 1;
    constexpr const char*  Text1       = "Press";
    constexpr const char*  Text2       = "the Menu button";
    constexpr const char*  Text3       = "to load";
    constexpr const char*  Text4       = "the selected effect";

    // Clear layer with background color
    m_pLayer->eraseLayer(__pActivePalette->ParameterBack);

    // Configure text rendering
    m_pLayer->setFont(FONTS);
    m_pLayer->setTextFrontColor(__pActivePalette->ParamInfoName);

    // Calculate text dimensions for centering
    uint16_t TextHeight    = m_pLayer->getTextHeight();
    uint16_t TextHalfWidth = m_pLayer->getTextWidth(Text1) / 2;

    // Draw instructional text lines centered on panel
    m_pLayer->setCursor(MidXLayer - TextHalfWidth, MidYLayer - (2 * (TextYOffset + TextHeight)));
    m_pLayer->drawText(Text1);

    TextHalfWidth = m_pLayer->getTextWidth(Text2) / 2;
    m_pLayer->setCursor(MidXLayer - TextHalfWidth, MidYLayer - (TextYOffset + TextHeight));
    m_pLayer->drawText(Text2);

    TextHalfWidth = m_pLayer->getTextWidth(Text3) / 2;
    m_pLayer->setCursor(MidXLayer - TextHalfWidth, MidYLayer + TextYOffset);
    m_pLayer->drawText(Text3);

    TextHalfWidth = m_pLayer->getTextWidth(Text4) / 2;
    m_pLayer->setCursor(MidXLayer - TextHalfWidth, MidYLayer + (2 * TextYOffset) + TextHeight);
    m_pLayer->drawText(Text4);
}

} // namespace DadGUI

//*** End of file ************************************************************
