//==================================================================================
//==================================================================================
// File: cUIParameter.cpp
// Description: Implementation of GUI-integrated parameter system
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUIParameter.h"
#include "HardwareAndCo.h"
#include "GUI_Define.h"

namespace DadGUI {

//**********************************************************************************
// Class: cUIParameter
// Description: Extends base parameter system with GUI integration capabilities
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------------------

//***********************************************************************************
// Method: Init
// Description:
// Initializes parameter with configuration and registers with GUI systems
//***********************************************************************************
void cUIParameter::Init(uint32_t SerializeID,
                        float InitValue, float Min, float Max,
                        float RapidIncrement, float SlowIncrement,
                        DadDSP::CallbackType Callback,
                        uint32_t CallbackUserData,
                        float Slope,
                        uint8_t Control)
{
    // Initialize base parameter functionality
    cParameter::Init(InitValue, Min, Max, RapidIncrement, SlowIncrement,
                     Callback, CallbackUserData, Slope, Control);

    // Register parameter with GUI serialization and real-time processing systems
    __GUI.addSerializeObject(this, SerializeID);
    __GUI.addRtProcessObject(this, SerializeID);
}

void cUIParameter::Init(uint32_t SerializeID,
						uint32_t RtProcessID,
                        float InitValue, float Min, float Max,
                        float RapidIncrement, float SlowIncrement,
                        DadDSP::CallbackType Callback,
                        uint32_t CallbackUserData,
                        float Slope,
                        uint8_t Control)
{
    // Initialize base parameter functionality
    cParameter::Init(InitValue, Min, Max, RapidIncrement, SlowIncrement,
                     Callback, CallbackUserData, Slope, Control);

    // Register parameter with GUI serialization and real-time processing systems
    __GUI.addSerializeObject(this, SerializeID);
    __GUI.addRtProcessObject(this, RtProcessID);
}
//***********************************************************************************
// Method: Save
// Description:
// Serializes parameter target value to persistent storage
//***********************************************************************************
void cUIParameter::Save(DadPersistentStorage::cSerialize* pSerializer)
{
    m_Dirty = false;                    // Reset dirty flag after save
    pSerializer->Push(m_TargetValue);   // Write target value to storage
}

//***********************************************************************************
// Method: Restore
// Description:
// Deserializes parameter value from storage and applies it
//***********************************************************************************
void cUIParameter::Restore(DadPersistentStorage::cSerialize* pSerializer)
{
    m_Dirty = false;                    // Reset dirty flag after restore
    float Value;                        // Temporary storage for retrieved value
    pSerializer->Pull(Value);           // Read value from storage
    setValue(Value);                    // Apply the restored value
}

//***********************************************************************************
// Method: isDirty
// Description:
// Checks if parameter has been modified since last save
//***********************************************************************************
bool cUIParameter::isDirty()
{
    return m_Dirty;                     // Return modification status
}

//***********************************************************************************
// Method: RtProcess
// Description:
// Performs real-time processing including value interpolation and callback
//***********************************************************************************
ITCM void cUIParameter::RtProcess()
{
    // Check if current value differs from target
    if (m_Value != m_TargetValue)
    {
        // Handle interpolation towards target value
        if (m_Value < m_TargetValue)
        {
            m_Value += m_Step;          // Increment towards target
            if (m_Value > m_TargetValue)
                m_Value = m_TargetValue; // Clamp to prevent overshoot
        }
        else if (m_Value > m_TargetValue)
        {
            m_Value -= m_Step;          // Decrement towards target
            if (m_Value < m_TargetValue)
                m_Value = m_TargetValue; // Clamp to prevent overshoot
        }

        // Execute callback if value has changed
        if (m_Callback)
        {
            m_Callback(this, m_CallbackUserData); // Notify listener of change
        }
    }
}

} // namespace DadGUI

//***End of file**************************************************************
