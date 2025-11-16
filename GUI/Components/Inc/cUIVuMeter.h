//==================================================================================
//==================================================================================
// File: cUIVuMeter.h
// Description: VU-meter display component for real-time audio level visualization
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "AudioManager.h"
#include "iUIComponent.h"
#include "iGUIProcessObject.h"
#include "cVuMeter.h"
#include "cDisplay.h"

namespace DadGUI {

//**********************************************************************************
// Class: cUIVuMeter
// Description: Handles graphical display of stereo VU-meters with real-time audio processing
//**********************************************************************************
class cUIVuMeter : public iUIComponent, public iGUIProcessObject {
public:
    virtual ~cUIVuMeter() = default;

    // ---------------------------------------------------------------------------------
    // Function: Init
    // Description: Initializes the VU-meter display layer and DSP meters
    // ---------------------------------------------------------------------------------
    void Init();

    // ---------------------------------------------------------------------------------
    // Function: Activate
    // Description: Called when the component becomes active and visible
    // ---------------------------------------------------------------------------------
    void Activate() override;

    // ---------------------------------------------------------------------------------
    // Function: Deactivate
    // Description: Called when the component is deactivated or hidden
    // ---------------------------------------------------------------------------------
    void Deactivate() override;

    // ---------------------------------------------------------------------------------
    // Function: Update
    // Description: Called every frame while the component is active to refresh the display
    // ---------------------------------------------------------------------------------
    void Update() override;

    // ---------------------------------------------------------------------------------
    // Function: GUIProcess
    // Description: Processes real-time audio buffers to compute current and peak levels
    // ---------------------------------------------------------------------------------
    ITCM void GUIProcess(AudioBuffer* pIn) override;

    // ---------------------------------------------------------------------------------
    // Function: Redraw
    // Description: Forces a full redraw of the static and dynamic elements
    // ---------------------------------------------------------------------------------
    void Redraw() override;

protected:
    // ---------------------------------------------------------------------------------
    // Function: drawStatPartOffLayer
    // Description: Draws static parts (frames, labels, background)
    // ---------------------------------------------------------------------------------
    void drawStatPartOffLayer();

    // ---------------------------------------------------------------------------------
    // Function: drawDynPartOffLayer
    // Description: Draws dynamic parts (levels, peaks, clip indicators)
    // ---------------------------------------------------------------------------------
    void drawDynPartOffLayer();

    // Member variables
    DadGFX::cLayer*    m_pVuMeterLayer;    // Pointer to the dedicated display layer
    bool               m_isActive;         // Indicates whether the UI component is active
    DadDSP::cVuMeter   m_VuMeterLeft;      // Left channel VU-meter processor
    DadDSP::cVuMeter   m_VuMeterRight;     // Right channel VU-meter processor
    bool               m_MemClippingLeft;  // Stores previous clipping state for left channel
    bool               m_MemClippingRight; // Stores previous clipping state for right channel
};

} // namespace DadGUI
//***End of file**************************************************************
