//==================================================================================
//==================================================================================
// File: cUIParameter.h
// Description: GUI-integrated parameter system with serialization and real-time processing
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "iRtObject.h"
#include "Serialize.h"
#include "cParameter.h"

namespace DadGUI {

//**********************************************************************************
// Class: cUIParameter
// Description:
// Extends base DSP parameter system with GUI integration, supporting serialization,
// deserialization, and real-time interpolation of parameter values
//**********************************************************************************
class cUIParameter
    : public iRtObject,
      public DadPersistentStorage::cSerializedObject,
      public DadDSP::cParameter
{
public:
    // -----------------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------------

    //***********************************************************************************
    // Method: Init
    // Description:
    // Initializes parameter with configuration and registers with GUI systems
    //***********************************************************************************
    void Init(uint32_t SerializeID,
              uint32_t RTProcessID,
              float InitValue,
              float Min,
              float Max,
              float RapidIncrement,
              float SlowIncrement,
              DadDSP::CallbackType Callback = nullptr,
              uint32_t CallbackUserData = 0,
              float Slope = 0,
              uint8_t Control = 0xFF);

    //***********************************************************************************
    // Method: Save
    // Description:
    // Serializes parameter target value to persistent storage
    //***********************************************************************************
    void Save(DadPersistentStorage::cSerialize* pSerializer) override;

    //***********************************************************************************
    // Method: Restore
    // Description:
    // Restores parameter value from storage and applies it
    //***********************************************************************************
    void Restore(DadPersistentStorage::cSerialize* pSerializer) override;

    //***********************************************************************************
    // Method: isDirty
    // Description:
    // Checks if parameter has been modified since last synchronization
    //***********************************************************************************
    bool isDirty() override;

    //***********************************************************************************
    // Method: RtProcess
    // Description:
    // Performs real-time value updates and smoothing between current and target values
    //***********************************************************************************
    void RtProcess() override;

    // Note: No protected or private sections declared in this class
    // Note: Inherits functionality from base classes cParameter, cSerializedObject, and iRtObject

};

} // namespace DadGUI

//***End of file**************************************************************
