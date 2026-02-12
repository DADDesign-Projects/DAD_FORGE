//==================================================================================
//==================================================================================
// File: cParameter.h
// Description: Parameter management with smoothing, normalization and MIDI control
// 
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "sections.h"

namespace DadDSP {

//**********************************************************************************
// class cParameter
//**********************************************************************************

// Forward declaration
class cParameter;

// Define the callback function type
using CallbackType = void(*)(cParameter*, uint32_t);

class cParameter {
public:
    virtual ~cParameter() {}

    // -----------------------------------------------------------------------------
    // Initialize the parameter with given attributes
    void Init(float InitValue, float Min, float Max,
              float RapidIncrement, float SlowIncrement,
              CallbackType Callback = nullptr,
              uint32_t CallbackUserData = 0,
              float Slope = 0,
              uint8_t Control = 0xFF);

    // -----------------------------------------------------------------------------
    // Increment the parameter value by a number of steps
    void Increment(int32_t nbStep, bool Switch);

    // -----------------------------------------------------------------------------
    // Get the current value of the parameter
    inline float getValue() const {
        return m_Value;
    }

    // -----------------------------------------------------------------------------
    // Set the parameter value directly with boundary checks
    void setValue(float value);

    // -----------------------------------------------------------------------------
    // Get the target value of the parameter
    inline float getTargetValue() const {
        return m_TargetValue;
    }

    // -----------------------------------------------------------------------------
    // Assignment operator overload for direct value assignment
    inline cParameter& operator=(float value) {
        setValue(value);
        return *this;
    }

    // -----------------------------------------------------------------------------
    // Implicit conversion to float for ease of use
    inline operator float() const {
        return m_Value;
    }

    // -----------------------------------------------------------------------------
    // Get the normalized value of the parameter (value scaled between 0 and 1)
    inline float getNormalizedValue() const {
        if(m_Max == m_Min) return 0;
        return (m_Value - m_Min) / (m_Max - m_Min);
    }

    // -----------------------------------------------------------------------------
    // Set the value of the parameter using a normalized value (between 0 and 1)
    inline void setNormalizedValue(float normalizedValue) {
        if (normalizedValue < 0.0f) {
            normalizedValue = 0.0f; // Clamp to minimum
        } else if (normalizedValue > 1.0f) {
            normalizedValue = 1.0f; // Clamp to maximum
        }
        setValue(m_Min + normalizedValue * (m_Max - m_Min));
    }

    // -----------------------------------------------------------------------------
    // Get the normalized value of the parameter (value scaled between 0 and 1)
    inline float getNormalizedTargetValue() const {
        if(m_Max == m_Min) return 0;
        return (m_TargetValue - m_Min) / (m_Max - m_Min);
    }

    // -----------------------------------------------------------------------------
    // Set the Max value of the parameter
    inline void setMaxValue(float MaxValue) {
        m_Max = MaxValue;
        calcStepValue();
    }

    // -----------------------------------------------------------------------------
    // Get the Max value of the parameter
    inline float getMaxValue() {
        return m_Max;
    }

    // -----------------------------------------------------------------------------
    // Set the Min value of the parameter
    inline void setMinValue(float MinValue) {
        m_Min = MinValue;
        calcStepValue();
    }

    // -----------------------------------------------------------------------------
    // Get the Min value of the parameter
    inline float getMinValue() {
        return m_Min;
    }

    // -----------------------------------------------------------------------------
    // Refresh the current value smoothly according to the slope
    // Return true if the value was updated, false otherwise
    bool Process();

    // -----------------------------------------------------------------------------
    // Function call when this CC is received
    static void MIDIControlChangeCallBack(uint8_t control, uint8_t value, uint32_t userData);

protected:
    // -----------------------------------------------------------------------------
    // Function calcStepValue
    // Calculate step size based on slope parameter
    inline void calcStepValue(){
        if(m_Slope == 0){
            m_Step = std::abs(m_Max-m_Min);          // Direct step when slope is zero
        }else{
            m_Step = std::abs(m_Max-m_Min)/ m_Slope;   // Scaled step based on slope
        }
    }

    // =============================================================================
    // Member Variables
    // =============================================================================

    float         m_Min = 0.0f;              // Minimum parameter value
    float         m_Max = 1.0f;              // Maximum parameter value
    float         m_RapidIncrement = 0.1f;   // Rapid increment step size
    float         m_SlowIncrement = 0.01f;   // Slow increment step size
    float         m_Value = 0.0;             // Current smoothed value
    float         m_Step;                    // Step size for smoothing
    float         m_TargetValue;             // Target parameter value
    float         m_Slope;                   // Smoothing slope factor
    CallbackType  m_Callback;                // Callback function pointer
    uint32_t      m_CallbackUserData;        // User data for callback
    bool          m_Dirty;                   // Dirty flag for change tracking

};

} // namespace DadDSP

//***End of file**************************************************************
