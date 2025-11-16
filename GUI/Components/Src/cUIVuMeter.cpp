//==================================================================================
//==================================================================================
// File: cUIVuMeter.cpp
// Description: Implementation of the VU-meter UI component for audio level display
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUIVuMeter.h"
#include "HardwareAndCo.h"

namespace DadGUI {

//**********************************************************************************
// Layer declaration
//**********************************************************************************
DECLARE_LAYER(VuMeterLayer, SCREEN_WIDTH, PARAM_HEIGHT);

//**********************************************************************************
// Static layout constants
//**********************************************************************************
constexpr uint16_t VuMeterHeight     = 26;  // Height of each VU-meter bar
constexpr uint16_t VuMeterWidth      = 240; // Width of each VU-meter bar
constexpr uint16_t VuMeterOffset     = 10;  // Vertical offset between meters
constexpr uint16_t TextXOffset       = 5;   // X offset for channel labels
constexpr uint16_t ClipXOffset       = 5;   // X offset for clipping indicators
constexpr uint16_t LayerHalfHeight   = PARAM_HEIGHT / 2;  // Half height of the layer
constexpr uint16_t XVuMeter          = (SCREEN_WIDTH - VuMeterWidth) / 2;  // X position centered
constexpr uint16_t YVuMeterL         = LayerHalfHeight - VuMeterOffset - VuMeterHeight;  // Y position left channel
constexpr uint16_t YVuMeterR         = LayerHalfHeight + VuMeterOffset;     // Y position right channel
constexpr uint16_t ClipRadius        = 6;   // Radius of clipping indicator circles

//**********************************************************************************
// Public methods
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initializes the VU-meter layer and DSP objects
// ---------------------------------------------------------------------------------
void cUIVuMeter::Init() {
    // Attach display layer and initialize meters
    m_pVuMeterLayer  = ADD_LAYER(__Display, VuMeterLayer, 0, MENU_HEIGHT, 0);
    m_VuMeterLeft.Init(SAMPLING_RATE);
    m_VuMeterRight.Init(SAMPLING_RATE);

    m_isActive = false;           // UI active state flag
    m_MemClippingLeft  = false;   // Left channel clipping memory
    m_MemClippingRight = false;   // Right channel clipping memory
}

// ---------------------------------------------------------------------------------
// Function: Activate
// Description: Called when the component becomes active and visible
// ---------------------------------------------------------------------------------
void cUIVuMeter::Activate() {
    m_isActive = true;  // Mark UI as active

    // Reset meter states
    m_VuMeterLeft.reset();
    m_VuMeterLeft.resetPeak();
    m_VuMeterRight.reset();
    m_VuMeterRight.resetPeak();

    // Register GUI process callback
    __GUI.setGUIProcess(this);

    // Adjust Z-order to bring the layer forward
    m_pVuMeterLayer->changeZOrder(41);

    // Draw static and dynamic parts
    drawStatPartOffLayer();
    drawDynPartOffLayer();
}

// ---------------------------------------------------------------------------------
// Function: Deactivate
// Description: Called when the component is deactivated or hidden
// ---------------------------------------------------------------------------------
void cUIVuMeter::Deactivate() {
    m_isActive = false;  // Mark UI as inactive

    // Remove GUI process
    __GUI.setGUIProcess(nullptr);

    // Reset VU-meter states
    m_VuMeterLeft.reset();
    m_VuMeterLeft.resetPeak();
    m_VuMeterRight.reset();
    m_VuMeterRight.resetPeak();

    // Move layer to background
    m_pVuMeterLayer->changeZOrder(0);
}

// ---------------------------------------------------------------------------------
// Function: Update
// Description: Called every frame while the component is active to refresh visuals
// ---------------------------------------------------------------------------------
void cUIVuMeter::Update() {
    if (m_isActive) {
        drawDynPartOffLayer();
    }
}

// ---------------------------------------------------------------------------------
// Function: Redraw
// Description: Forces a complete redraw of static and dynamic parts
// ---------------------------------------------------------------------------------
void cUIVuMeter::Redraw() {
    if (m_isActive) {
        drawStatPartOffLayer();
        drawDynPartOffLayer();
    }
}

//**********************************************************************************
// Private methods
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: drawStatPartOffLayer
// Description: Draws static elements such as frames, labels, and background
// ---------------------------------------------------------------------------------
void cUIVuMeter::drawStatPartOffLayer() {
    // Clear layer with background color
    m_pVuMeterLayer->eraseLayer(__pActivePalette->VuMeterBack);

    // Configure text rendering
    m_pVuMeterLayer->setFont(FONTMB);
    m_pVuMeterLayer->setTextFrontColor(__pActivePalette->VuMeterText);

    // Draw channel labels
    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterL - 2);
    m_pVuMeterLayer->drawChar('L');
    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterR - 2);
    m_pVuMeterLayer->drawChar('R');

    // Draw VU-meter frames
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterL - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterR - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);
}

// ---------------------------------------------------------------------------------
// Function: drawDynPartOffLayer
// Description: Draws dynamic elements such as fill levels, peaks, and clip indicators
// ---------------------------------------------------------------------------------
void cUIVuMeter::drawDynPartOffLayer() {
    // Erase old levels by filling with background color
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterL, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterR, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);

    // Draw current signal levels based on dB percentage
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterL, VuMeterWidth * m_VuMeterLeft.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterR, VuMeterWidth * m_VuMeterRight.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);

    // Draw peak indicators for left channel
    uint16_t XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterLeft.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterL, XPeakLine, YVuMeterL + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterL, XPeakLine - 1, YVuMeterL + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Draw peak indicators for right channel
    XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterRight.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterR, XPeakLine, YVuMeterR + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterR, XPeakLine - 1, YVuMeterR + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Update left channel clipping indicator if state changed
    bool IsClipping = m_VuMeterLeft.isClipping();
    if (m_MemClippingLeft != IsClipping) {
        m_MemClippingLeft = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterL + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }

    // Update right channel clipping indicator if state changed
    IsClipping = m_VuMeterRight.isClipping();
    if (m_MemClippingRight != IsClipping) {
        m_MemClippingRight = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterR + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }
}

// ---------------------------------------------------------------------------------
// Function: GUIProcess
// Description: Processes audio input buffers and updates VU-meter readings in real time
// ---------------------------------------------------------------------------------
ITCM void cUIVuMeter::GUIProcess(AudioBuffer* pIn) {
    if (m_isActive) {
        // Process left and right audio channels
        m_VuMeterLeft.Process(pIn->Left);
        m_VuMeterRight.Process(pIn->Right);
    }
}

} // namespace DadGUI
//***End of file**************************************************************
