//==================================================================================
//==================================================================================
// File: cVuMeter.cpp
// Description: VU meter processing implementation with peak detection and clipping
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cVuMeter.h"

namespace DadDSP {


//**********************************************************************************
// Class: cVuMeter
// Description: Audio level meter with peak hold and clipping detection
//**********************************************************************************

// =============================================================================
// Signal Processing
// =============================================================================

// -----------------------------------------------------------------------------
// Process
// Description: Processes audio input to calculate current level, peak detection and clipping
// Parameters: input - Audio sample to process
// -----------------------------------------------------------------------------
void cVuMeter::CalcPeakAndLevel(float input)
{
    // Convert input to absolute value and clamp to valid range
    float absValue = std::abs(input);
    absValue = std::min(absValue, 1.0f);

    // Apply attack/release envelope for smooth level tracking
    if (absValue > m_currentLevel) {
        m_currentLevel += (absValue - m_currentLevel) * m_attackCoeff;  // Fast attack
    } else {
        m_currentLevel += (absValue - m_currentLevel) * m_releaseCoeff; // Slow release
    }

    // Peak detection with hold time algorithm
    if (m_currentLevel > m_peakLevel) {
        m_peakLevel = m_currentLevel;
        m_peakHoldCounter = m_peakHoldSamples;
    } else if (m_peakHoldCounter > 0) {
        m_peakHoldCounter--;  // Decrement hold counter
    } else {
        m_peakLevel *= 0.999f;  // Gradual peak decay
    }

    // Clipping detection with hold time
    if (m_currentLevel >= CLIP_THRESHOLD) {
        m_clipActive = true;
        m_clipCounter = m_clipHoldSamples;
    } else if (m_clipCounter > 0) {
        m_clipCounter--;
        if (m_clipCounter == 0) {
            m_clipActive = false;  // Reset clipping flag
        }
    }
}

// =============================================================================
// Conversion Utilities
// =============================================================================

// -----------------------------------------------------------------------------
// linearToDb
// Description: Converts linear amplitude value to decibel scale
// Parameters: linear - Linear amplitude value (0.0 to 1.0)
// Returns: Value in decibels
// -----------------------------------------------------------------------------
float cVuMeter::linearToDb(float linear) const
{
    if (linear <= 0.0f) {
        return DB_MIN;  // Minimum dB value for silence
    }
    float db = 20.0f * std::log10(linear);  // Convert linear to dB using log10 formula
    return std::max(db, DB_MIN);  // Ensure minimum dB value
}

// -----------------------------------------------------------------------------
// dbToPercent
// Description: Converts decibel value to percentage for meter display
// Parameters: db - Value in decibels
// Returns: Percentage value (0.0 to 1.0)
// -----------------------------------------------------------------------------
float cVuMeter::dbToPercent(float db) const
{
    if (db <= DB_MIN) return 0.0f;   // Bottom of scale
    if (db >= DB_MAX) return 1.0f;   // Top of scale
    return (db - DB_MIN) / (DB_MAX - DB_MIN);  // Linear interpolation between min and max dB
}

// =============================================================================
// Coefficient Updates
// =============================================================================

// -----------------------------------------------------------------------------
// updateAttackRelease
// Description: Updates attack and release coefficients based on sample rate
//              and calculates hold time sample counts
// -----------------------------------------------------------------------------
void cVuMeter::updateAttackRelease()
{
    // Calculate sample counts for peak and clip hold times
    m_peakHoldSamples = static_cast<int32_t>(PEAK_HOLD_TIME * m_sampleRate);
    m_clipHoldSamples = static_cast<int32_t>(CLIP_HOLD_TIME * m_sampleRate);

    // Calculate attack and release coefficients using exponential formulas
    m_attackCoeff = 1.0f - std::exp(-1.0f / (ATTACK_TIME * m_sampleRate));
    m_releaseCoeff = 1.0f - std::exp(-1.0f / (RELEASE_TIME * m_sampleRate));
}

} // namespace DadDSP

//***End of file**************************************************************
