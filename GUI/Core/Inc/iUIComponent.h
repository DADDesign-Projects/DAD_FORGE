//==================================================================================
//==================================================================================
// File: iUIComponent.h
// Description: Abstract interface for all user interface components
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

//**********************************************************************************
//**********************************************************************************
// Namespace: DadGUI
// Description: Contains user interface component interface definitions
//**********************************************************************************
//**********************************************************************************

#include "main.h"

namespace DadGUI {

//**********************************************************************************
// Class: iUIComponent
// Description: Abstract interface for all user interface components
//
// Components derived from iUIComponent implement three key methods:
// - Activate(): called when the component becomes visible or active
// - Deactivate(): called when the component is hidden or replaced
// - Update(): called every frame or control tick to refresh visuals or logic
//
// This class also forbids copy construction to avoid unintended duplication
// of components and resource conflicts.
//**********************************************************************************
class iUIComponent {
public:
    // -----------------------------------------------------------------------------
    // Virtual Destructor
    // -----------------------------------------------------------------------------
    virtual ~iUIComponent() = default;

    // -----------------------------------------------------------------------------
    // Component Lifecycle Methods
    // -----------------------------------------------------------------------------

    // Called when the component becomes active and visible
    virtual void Activate() {};

    // Called when the component is deactivated or hidden
    virtual void Deactivate() {};

    // Called every frame while the component is active
    virtual void Update() {};

    // -----------------------------------------------------------------------------
    // Pure Virtual Methods
    // -----------------------------------------------------------------------------

    // Force to redraw UI component
    virtual void Redraw() = 0;
};

} // namespace DadGUI

//***End of file**************************************************************
