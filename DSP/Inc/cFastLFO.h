//==================================================================================
//==================================================================================
// File: cFastLFO.h
// Description: Fast LFO implementation using sine LUT (Look-Up Table)
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#include "main.h"
#include <cmath>
#include <array>

namespace DadDSP {

//**********************************************************************************
//**********************************************************************************
// Class: cFastLFO
// Description: Fast LFO implementation using sine LUT with linear interpolation
//**********************************************************************************
//**********************************************************************************

template<size_t TABLE_SIZE = 1024>
class cFastLFO
{
public:
    // =============================================================================
    // Constructors
    // =============================================================================

    // Default constructor
    cFastLFO() {}

    // =============================================================================
    // Public methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initializes the LFO with sample rate, frequency and initial phase
    void Initialise(float sampleRate, float frequency, float initialPhase)
    {
        m_sampleRate = sampleRate;     // Audio sample rate
        m_frequency = frequency;       // LFO frequency in Hz
        m_phase = initialPhase;        // Initial phase (0.0 to 1.0)

        initTable();
        updatePhaseIncrement();

        // Normalize initial phase between 0 and 1
        while (m_phase >= 1.0f) m_phase -= 1.0f;
        while (m_phase < 0.0f) m_phase += 1.0f;
    }

    // -----------------------------------------------------------------------------
    // Sets the LFO frequency
    inline void setFrequency(float freq)
    {
        m_frequency = freq;            // Update frequency
        updatePhaseIncrement();        // Recalculate phase increment
    }

    // -----------------------------------------------------------------------------
    // Gets the current frequency
    inline float getFrequency() const
    {
        return m_frequency;            // Current LFO frequency
    }

    // -----------------------------------------------------------------------------
    // Sets the phase (0.0 to 1.0)
    inline void setPhase(float newPhase)
    {
        m_phase = newPhase;            // Set new phase

        // Normalize phase to [0, 1) range
        while (m_phase >= 1.0f) m_phase -= 1.0f;
        while (m_phase < 0.0f) m_phase += 1.0f;
    }

    // -----------------------------------------------------------------------------
    // Gets the current phase
    inline float getPhase() const
    {
        return m_phase;                // Current phase (0.0 to 1.0)
    }

    // -----------------------------------------------------------------------------
    // Sets the phase in radians (0.0 to 2π)
    inline void setPhaseRad(float newPhaseRadian)
    {
        // Convert radians to normalized phase
        setPhase(newPhaseRadian / (2.0f * M_PI));
    }

    // -----------------------------------------------------------------------------
    // Gets the current phase in radians
    inline float getPhaseRad() const
    {
        return m_phase * 2.0f * M_PI;  // Convert normalized phase to radians
    }

    // -----------------------------------------------------------------------------
    // Sets the sample rate
    inline void setSampleRate(float sr)
    {
        m_sampleRate = sr;             // Update sample rate
        updatePhaseIncrement();        // Recalculate phase increment
    }

    // -----------------------------------------------------------------------------
    // Process method - called for each sample (with linear interpolation)
    inline float process()
    {
        // Calculate table index
        float tableIndex = m_phase * TABLE_SIZE;
        size_t index = static_cast<size_t>(tableIndex);

        // Linear interpolation for better quality
        float frac = tableIndex - index;                   // Fractional part
        size_t nextIndex = (index + 1) % TABLE_SIZE;       // Next index (wrapped)

        // Interpolated sample
        float sample = m_sineTable[index] * (1.0f - frac) +
                       m_sineTable[nextIndex] * frac;

        // Advance phase
        m_phase += m_phaseIncrement;
        if (m_phase >= 1.0f)
        {
            m_phase -= 1.0f;          // Wrap phase around
        }

        return sample;                // Return calculated sample
    }

    // -----------------------------------------------------------------------------
    // Fast process method - without interpolation (faster but less precise)
    inline float processFast()
    {
        // Direct table lookup (no interpolation)
        size_t index = static_cast<size_t>(m_phase * TABLE_SIZE) % TABLE_SIZE;
        float sample = m_sineTable[index];

        // Advance phase
        m_phase += m_phaseIncrement;
        if (m_phase >= 1.0f)
        {
            m_phase -= 1.0f;          // Wrap phase around
        }

        return sample;                // Return table value
    }

    // -----------------------------------------------------------------------------
    // Resets phase to 0
    inline void reset()
    {
        m_phase = 0.0f;               // Reset phase to beginning
    }

private:
    // =============================================================================
    // Static constants
    // =============================================================================

    // (Static members initialized at end of file)

    // =============================================================================
    // Private methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Initializes the sine table if not already done
    static void initTable()
    {
        if (!m_tableInitialized)
        {
            // Generate sine table
            for (size_t i = 0; i < TABLE_SIZE; ++i)
            {
                m_sineTable[i] = std::sin(2.0f * M_PI * i / TABLE_SIZE);
            }
            m_tableInitialized = true;    // Mark table as initialized
        }
    }

    // -----------------------------------------------------------------------------
    // Recalculates phase increment based on frequency
    inline void updatePhaseIncrement()
    {
        // Phase increment per sample
        m_phaseIncrement = m_frequency / m_sampleRate;
    }

    // =============================================================================
    // Private member variables
    // =============================================================================

    // Static sine table shared by all instances
    static std::array<float, TABLE_SIZE> m_sineTable;
    static bool m_tableInitialized;

    // Instance variables
    float m_sampleRate;               // Audio sample rate in Hz
    float m_frequency;                // LFO frequency in Hz
    float m_phase;                    // Current phase (0.0 to 1.0)
    float m_phaseIncrement;           // Phase increment per sample
};

// =============================================================================
// Static member initialization
// =============================================================================

template<size_t TABLE_SIZE>
std::array<float, TABLE_SIZE> cFastLFO<TABLE_SIZE>::m_sineTable;

template<size_t TABLE_SIZE>
bool cFastLFO<TABLE_SIZE>::m_tableInitialized = false;

} // namespace DadDSP

//***End of file**************************************************************
