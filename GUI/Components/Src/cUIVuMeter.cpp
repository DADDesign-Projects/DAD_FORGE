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
constexpr uint16_t VuMeterHeight       = 18; // Height of each VU-meter bar
constexpr uint16_t VuMeterWidth        = 210;// Width of each VU-meter bar
constexpr uint16_t VuMeterCenterOffset = 10; // Vertical offset between meters
constexpr uint16_t VuMeterLROffset     = 6;  // Vertical offset between meters

constexpr uint16_t TextXOffset       = 50;   // X offset for channel labels
constexpr uint16_t ClipXOffset       = 5;    // X offset for clipping indicators
constexpr uint16_t LayerHalfHeight   = PARAM_HEIGHT / 2;  										// Half height of the layer
constexpr uint16_t XVuMeter          = ((SCREEN_WIDTH - VuMeterWidth) / 2) + 20;				// X position centered

constexpr uint16_t YVuMeterInL       = LayerHalfHeight - VuMeterCenterOffset - (2 * VuMeterHeight) - VuMeterLROffset; // Y position left channel
constexpr uint16_t YVuMeterInR       = LayerHalfHeight - VuMeterCenterOffset - VuMeterHeight;     				      // Y position right channel
constexpr uint16_t YVuMeterOutL      = LayerHalfHeight + VuMeterCenterOffset;  										  // Y position left channel
constexpr uint16_t YVuMeterOutR      = LayerHalfHeight + VuMeterCenterOffset + VuMeterHeight + VuMeterLROffset ;  	  // Y position right channel

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
    m_VuMeterInLeft.Init(SAMPLING_RATE);
    m_VuMeterInRight.Init(SAMPLING_RATE);
    m_VuMeterOutLeft.Init(SAMPLING_RATE);
    m_VuMeterOutRight.Init(SAMPLING_RATE);

    m_isActive = false;           // UI active state flag
    m_MemClippingInLeft  = false;   // Left channel clipping memory
    m_MemClippingOutRight = false;   // Right channel clipping memory
    m_MemClippingOutLeft  = false;   // Left channel clipping memory
    m_MemClippingOutRight = false;   // Right channel clipping memory
}

// ---------------------------------------------------------------------------------
// Function: Activate
// Description: Called when the component becomes active and visible
// ---------------------------------------------------------------------------------
void cUIVuMeter::Activate() {
    m_isActive = true;  // Mark UI as active

    // Reset meter states
    m_VuMeterInLeft.reset();
    m_VuMeterInLeft.resetPeak();
    m_VuMeterInRight.reset();
    m_VuMeterInRight.resetPeak();

    m_VuMeterOutLeft.reset();
    m_VuMeterOutLeft.resetPeak();
    m_VuMeterOutRight.reset();
    m_VuMeterOutRight.resetPeak();

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
    m_VuMeterInLeft.reset();
    m_VuMeterInLeft.resetPeak();
    m_VuMeterInRight.reset();
    m_VuMeterInRight.resetPeak();

    m_VuMeterOutLeft.reset();
    m_VuMeterOutLeft.resetPeak();
    m_VuMeterOutRight.reset();
    m_VuMeterOutRight.resetPeak();

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
    m_pVuMeterLayer->setFont(FONTSB);
    m_pVuMeterLayer->setTextFrontColor(__pActivePalette->VuMeterText);

    // Draw channel labels
    m_pVuMeterLayer->setCursor(5, YVuMeterInL + VuMeterLROffset);
    m_pVuMeterLayer->drawText("In");

    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterInL - 2);
    m_pVuMeterLayer->drawChar('L');
    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterInR - 2);
    m_pVuMeterLayer->drawChar('R');

    m_pVuMeterLayer->setCursor(5, YVuMeterOutR - VuMeterHeight);
    m_pVuMeterLayer->drawText("Out");

    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterOutL - 2);
    m_pVuMeterLayer->drawChar('L');
    m_pVuMeterLayer->setCursor(TextXOffset, YVuMeterOutR - 2);
    m_pVuMeterLayer->drawChar('R');

    // Draw VU-meter frames
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterInL - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterInR - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterOutL - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);
    m_pVuMeterLayer->drawRect(XVuMeter - 1, YVuMeterOutR - 1, VuMeterWidth + 2, VuMeterHeight + 2, 1, __pActivePalette->VuMeterLine);

}

// ---------------------------------------------------------------------------------
// Function: drawDynPartOffLayer
// Description: Draws dynamic elements such as fill levels, peaks, and clip indicators
// ---------------------------------------------------------------------------------
void cUIVuMeter::drawDynPartOffLayer() {
    // Erase old levels by filling with background color
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterInL, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterInR, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);

    // Draw current signal levels based on dB percentage
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterInL, VuMeterWidth * m_VuMeterInLeft.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterInR, VuMeterWidth * m_VuMeterInRight.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);

    // Erase old levels by filling with background color
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterOutL, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterOutR, VuMeterWidth, VuMeterHeight, __pActivePalette->VuMeterBack);

    // Draw current signal levels based on dB percentage
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterOutL, VuMeterWidth * m_VuMeterOutLeft.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);
    m_pVuMeterLayer->drawFillRect(XVuMeter, YVuMeterOutR, VuMeterWidth * m_VuMeterOutRight.getLevelPercentDB(), VuMeterHeight, __pActivePalette->VuMeterCursor);

    // Draw peak indicators for left channel
    uint16_t XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterInLeft.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterInL, XPeakLine, YVuMeterInL + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterInL, XPeakLine - 1, YVuMeterInL + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Draw peak indicators for right channel
    XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterInRight.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterInR, XPeakLine, YVuMeterInR + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterInR, XPeakLine - 1, YVuMeterInR + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Draw peak indicators for left channel
    XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterOutLeft.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterOutL, XPeakLine, YVuMeterOutL + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterOutL, XPeakLine - 1, YVuMeterOutL + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Draw peak indicators for right channel
    XPeakLine = XVuMeter + VuMeterWidth * m_VuMeterOutRight.getPeakPercentDB();
    m_pVuMeterLayer->drawLine(XPeakLine, YVuMeterOutR, XPeakLine, YVuMeterOutR + VuMeterHeight, __pActivePalette->VuMeterPeak);
    m_pVuMeterLayer->drawLine(XPeakLine - 1, YVuMeterOutR, XPeakLine - 1, YVuMeterOutR + VuMeterHeight, __pActivePalette->VuMeterPeak);

    // Update left channel clipping indicator if state changed
    bool IsClipping = m_VuMeterInLeft.isClipping();
    if (m_MemClippingInLeft != IsClipping) {
        m_MemClippingInLeft = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterInL + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }

    // Update right channel clipping indicator if state changed
    IsClipping = m_VuMeterInRight.isClipping();
    if (m_MemClippingInRight != IsClipping) {
        m_MemClippingInRight = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterInR + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }

    // Update left channel clipping indicator if state changed
    IsClipping = m_VuMeterOutLeft.isClipping();
    if (m_MemClippingOutLeft != IsClipping) {
        m_MemClippingOutLeft = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterOutL + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }

    // Update right channel clipping indicator if state changed
    IsClipping = m_VuMeterOutRight.isClipping();
    if (m_MemClippingOutRight != IsClipping) {
        m_MemClippingOutRight = IsClipping;
        m_pVuMeterLayer->drawFillCircle(
            XVuMeter + VuMeterWidth + ClipXOffset + ClipRadius,
            YVuMeterOutR + (VuMeterHeight / 2),
            ClipRadius,
            IsClipping ? __pActivePalette->VuMeterClip : __pActivePalette->VuMeterBack
        );
    }

}

// ---------------------------------------------------------------------------------
// Function: GUIProcessIn
// Description: Processes audio input buffers and updates VU-meter readings in real time
// ---------------------------------------------------------------------------------
ITCM void cUIVuMeter::GUIProcessIn(AudioBuffer* pIn) {
    if (m_isActive) {
        // Process left and right audio channels
        m_VuMeterInLeft.CalcPeakAndLevel(pIn->Left);
        m_VuMeterInRight.CalcPeakAndLevel(pIn->Right);
    }
}

// ---------------------------------------------------------------------------------
// Function: GUIProcessOut
// Description: Processes audio output buffers and updates VU-meter readings in real time
// ---------------------------------------------------------------------------------
ITCM void cUIVuMeter::GUIProcessOut(AudioBuffer* pOut) {
    if (m_isActive) {
        // Process left and right audio channels
        m_VuMeterOutLeft.CalcPeakAndLevel(pOut->Left);
        m_VuMeterOutRight.CalcPeakAndLevel(pOut->Right);
    }
}

} // namespace DadGUI
//***End of file**************************************************************
