//==================================================================================
//==================================================================================
// File: cParameter.cpp
// Description: Parameter management with smoothing and MIDI control
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "Serialize.h"
#include "cParameter.h"
#include "HardwareAndCo.h"
#include <algorithm>  // pour std::abs, std::min, etc.

namespace DadDSP {

//**********************************************************************************
// class cParameter
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialize the parameter with given attributes
void cParameter::Init(float InitValue, float Min, float Max,
                      float RapidIncrement, float SlowIncrement,
                      CallbackType Callback, uint32_t CallbackUserData,
                      float Slope,
                      uint8_t Control) {
    m_Min = Min;                                    // Minimum parameter value
    m_Max = Max;                                    // Maximum parameter value
    m_RapidIncrement = RapidIncrement;              // Fast adjustment step size
    m_SlowIncrement = SlowIncrement;                // Slow adjustment step size
    m_Callback = Callback;                          // Callback function pointer
    m_CallbackUserData = CallbackUserData;          // User data for callback
    m_Slope = Slope;                                // Smoothing slope factor

    // Calculate step size based on slope parameter
    calcStepValue();

    m_Value = m_Min;                                // Initialize value

    // Register MIDI control change callback if control specified
    if(Control != 0xFF){
        __Midi.addControlChangeCallback(Control, (uint32_t) this, MIDIControlChangeCallBack );
    }

    // Ensure the initial value is within bounds
    setValue(InitValue);

    m_Dirty = false;                                // Initialize dirty flag
}

// -----------------------------------------------------------------------------
// Set the parameter value directly with boundary checks
void cParameter::setValue(float value) {
    // Clamp value to valid range (support inverted ranges)
    if(m_Max >= m_Min) {
        if(value > m_Max) {
            m_TargetValue = m_Max;
        } else if(value < m_Min){
            m_TargetValue = m_Min;
        } else {
            m_TargetValue = value;
        }
    } else {
        if(value > m_Min) {
            m_TargetValue = m_Min;
        } else if(value < m_Max){
            m_TargetValue = m_Max;
        } else {
            m_TargetValue = value;
        }
    }
}

// -----------------------------------------------------------------------------
// Update the current value smoothly according to the slope
// Return true if the value was updated, false otherwise
bool cParameter::Process() {
    // Check if current value needs to approach target
    if(m_Value != m_TargetValue){
        // Determine direction
        bool increasing = (m_TargetValue > m_Value);

        if (increasing)
        {
            m_Value += m_Step;                      // Increment toward target
            if (m_Value > m_TargetValue)
                m_Value = m_TargetValue;            // Prevent overshoot
        }
        else
        {
            m_Value -= m_Step;                      // Decrement toward target
            if (m_Value < m_TargetValue)
                m_Value = m_TargetValue;            // Prevent overshoot
        }

        // Trigger callback if value changed and callback is defined
        if (m_Callback) {
            m_Callback(this, m_CallbackUserData);
        }
        return true;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Increment the parameter value by a number of steps
void cParameter::Increment(int32_t nbStep, bool Switch) {
    float Value = m_TargetValue;

    float increment = Switch ? m_SlowIncrement : m_RapidIncrement;

    // Reverse increment direction when range is inverted
    if (m_Max < m_Min) {
        increment = -increment;
    }

    Value += increment * nbStep;

    m_Dirty = true;                                 // Mark parameter as changed
    setValue(Value);                                // Set new target value
}

// -----------------------------------------------------------------------------
// Function call when this CC is received
void cParameter::MIDIControlChangeCallBack(uint8_t control, uint8_t value, uint32_t userData) {
    cParameter* pThis = reinterpret_cast<cParameter*>(userData);

    // Clamp MIDI value to valid range
    value = (value > 127) ? 127 : value;

    // Convert MIDI value to parameter range (works with inverted ranges)
    float NewVal = pThis->m_Min + (value * (pThis->m_Max - pThis->m_Min)) / 127.0f;

    pThis->setValue(NewVal);                        // Update parameter value
    pThis->m_Dirty = true;                          // Mark parameter as changed
}

} // namespace DadDSP

//***End of file**************************************************************
