//==================================================================================
//==================================================================================
// File: DigitalOscillator.h
// Description: Digital Controlled Oscillator (DCO) implementation
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
// =============================================================================
#include "main.h"
#include "arm_math.h"

namespace DadDSP {

//**********************************************************************************
// cDCO
// Implements a Digital Controlled Oscillator (DCO)
//**********************************************************************************
class cDCO {

public:
    // -----------------------------------------------------------------------------
    // Initializes the DCO with sample rate, frequency range, and duty cycle
    void Initialize(float sampleRate, float frequency, float minFreq, float maxFreq, float dutyCycle) {
        m_sampleRate = sampleRate;    // Sample rate in Hz
        m_minFreq = minFreq;          // Minimum frequency in Hz
        m_maxFreq = maxFreq;          // Maximum frequency in Hz
        m_dcoValue = 0;               // Current phase position
        m_dcoStep = 0;                // Phase increment per sample

        setNormalizedFreq(frequency);
        setNormalizedDutyCycle(dutyCycle);
    }

    // -----------------------------------------------------------------------------
    // Sets the frequency between 0 and 1 (0 = minFreq, 1 = maxFreq)
    inline void setNormalizedFreq(float frequency) {
        // Convert normalized frequency to actual step size
        m_dcoStep = ((m_minFreq + (m_maxFreq - m_minFreq) * frequency) / m_sampleRate);
    }

    // -----------------------------------------------------------------------------
    // Sets the frequency in Hz
    inline void setFreq(float frequency) {
        m_dcoStep = frequency / m_sampleRate;    // Calculate phase increment
    }

    // -----------------------------------------------------------------------------
    // Sets the duty cycle of the DCO between 0 and 1
    inline void setNormalizedDutyCycle(float dutyCycle) {
        // Clamp duty cycle to avoid extreme values
        constexpr float minDuty = 0.1f;
        constexpr float maxDuty = 0.9f;
        m_dutyCycle = minDuty + ((maxDuty - minDuty) * dutyCycle);
    }

    // -----------------------------------------------------------------------------
    // Advances the oscillator by one step (-1/sampleRate)
    inline void Step() {
        m_dcoValue += m_dcoStep;      // Increment phase
        if (m_dcoValue > 1.0f) {      // Wrap phase to [0, 1)
            m_dcoValue -= 1.0f;
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the square wave output value
    inline float getSquareValue() {
        constexpr float riseTime = 0.04f;        // Rise time for smooth edges
        constexpr float fallStart = 0.7f;        // Start of fall time
        constexpr float fallEnd = fallStart + riseTime;  // End of fall time

        // Generate waveform with defined rising and falling edges
        if (m_dcoValue > fallEnd) {
            return 0;                            // Low state after fall
        } else if (m_dcoValue > fallStart) {
            return 1 - ((m_dcoValue - fallStart) / riseTime);  // Falling edge
        } else if (m_dcoValue > riseTime) {
            return 1;                            // High state
        } else {
            return (m_dcoValue / riseTime);      // Rising edge
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the square wave output value with duty cycle variation
    inline float getSquareModValue() {
        constexpr float riseTime = 0.04f;        // Rise time for smooth edges

        // Adjust waveform based on duty cycle
        if (m_dcoValue > (m_dutyCycle + riseTime)) {
            return 0;                            // Low state after fall
        } else if (m_dcoValue > m_dutyCycle) {
            return 1 - ((m_dcoValue - m_dutyCycle) / riseTime);  // Falling edge
        } else if (m_dcoValue > riseTime) {
            return 1;                            // High state
        } else {
            return (m_dcoValue / riseTime);      // Rising edge
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the triangle wave output value
    inline float getTriangleValue() {
        // Create symmetrical triangle waveform
        if (m_dcoValue > 0.5f) {
            return 2 - (m_dcoValue * 2);         // Descending slope
        } else {
            return (m_dcoValue * 2);             // Ascending slope
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the phase-shifted triangle wave output value
    inline float getTriangleValuePhased(float phaseShift) {
        // Apply phase shift and wrap to [0, 1)
        float t = fmod(m_dcoValue + phaseShift, 1.0f);
        if (t < 0.0f) t += 1.0f;

        // Create symmetrical triangle waveform
        if (t > 0.5f) {
            return 2.0f - (t * 2.0f);           // Descending slope
        } else {
            return t * 2.0f;                    // Ascending slope
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the triangle wave output value with duty cycle variation
    inline float getTriangleModValue() {
        // Adjust waveform based on duty cycle
        if (m_dcoValue > m_dutyCycle) {
            return (1 - m_dcoValue) / (1 - m_dutyCycle);  // Descending slope
        } else {
            return (m_dcoValue / m_dutyCycle);            // Ascending slope
        }
    }

    // -----------------------------------------------------------------------------
    // Reads the sine wave output value
    inline float getSineValue() {
        // Generate sine wave 0 -> 1
        return 0.5f + (arm_sin_f32((m_twoPI * m_dcoValue) + m_halfPI) * 0.5f);
    }

    // -----------------------------------------------------------------------------
    // Reads the sine wave output value
    inline float getSymetricalSineValue() {
        // Generate sine wave -1 -> 1
        return arm_sin_f32((m_twoPI * m_dcoValue) + m_halfPI);
    }

    // -----------------------------------------------------------------------------
    // Reads the rectified sine wave output value
    inline float getRectifiedSineValue() {
        // Use absolute sine to create half-wave rectified signal
        return arm_sin_f32(m_PI * m_dcoValue);
    }

    // -----------------------------------------------------------------------------
    // Sets the oscillator position
    inline void setPosition(float position) {
        m_dcoValue = position;        // Set phase position directly
    }

private:
    // =============================================================================
    // Constants
    // =============================================================================
    const float m_twoPI = 6.28318530717959F;    // 2 * PI
    const float m_PI = 3.14159265358979F;       // PI constant
    const float m_halfPI = 1.5707963267949F;    // PI/2 constant

    // =============================================================================
    // Member variables
    // =============================================================================
    float m_sampleRate = 0.0f;        // Sample rate in Hz
    float m_minFreq = 0.0f;           // Minimum frequency in Hz
    float m_maxFreq = 0.0f;           // Maximum frequency in Hz
    float m_dutyCycle;                // Duty cycle for modulated waveforms
    float m_dcoValue = 0.0f;          // Current phase position [0, 1)
    float m_dcoStep = 0.0f;           // Phase increment per sample

}; // class cDCO

} // namespace DadDSP
//***End of file**************************************************************
