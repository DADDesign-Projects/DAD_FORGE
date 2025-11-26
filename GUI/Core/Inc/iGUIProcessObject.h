//==================================================================================
//==================================================================================
// File: iGUIProcessObject.h
// Description: Abstract interface for GUI process objects
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

//**********************************************************************************
//**********************************************************************************
// Namespace: DadGUI
// Description: Contains GUI interface definitions
//**********************************************************************************
//**********************************************************************************

#include "main.h"
#include "AudioManager.h"

namespace DadGUI {

//**********************************************************************************
// Class: iGUIProcessObject
// Description: Abstract interface for GUI process objects that handle audio processing
//**********************************************************************************
class iGUIProcessObject {
public:
    // -----------------------------------------------------------------------------
    // Virtual Destructor
    // -----------------------------------------------------------------------------
    virtual ~iGUIProcessObject() = default;

    // -----------------------------------------------------------------------------
    // Pure Virtual Methods
    // -----------------------------------------------------------------------------

    // Process audio buffer through GUI object before audio process
    virtual void GUIProcessIn(AudioBuffer *pIn) = 0;

    // Process audio buffer through GUI object after audio process
    virtual void GUIProcessOut(AudioBuffer *pOut) = 0;

};

} // namespace DadGUI

//***End of file**************************************************************
