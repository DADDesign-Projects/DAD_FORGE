//==================================================================================
//==================================================================================
// File: cVuMeter.h
// Description: VU meter class declaration with peak detection and clipping
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "sections.h"
#include <cmath>
#include <algorithm>

namespace DadDSP {

// =============================================================================
// Configuration Constants
// =============================================================================

constexpr float PEAK_HOLD_TIME  = 1.0f;   // Peak hold time in seconds
constexpr float CLIP_THRESHOLD  = 0.70f;   // Clipping threshold (85%)
constexpr float CLIP_HOLD_TIME  = 4.0f;   // Clip hold time in seconds
constexpr float DB_MIN          = -45.0f; // Minimum level in dB
constexpr float DB_MAX          = -6.0f;   // Maximum level in dB
constexpr float ATTACK_TIME     = 0.15f;  // Attack time in seconds
constexpr float RELEASE_TIME    = 0.3f;   // Release time in seconds


//**********************************************************************************
// Class: cVuMeter
// Description: Audio level meter with peak hold and clipping detection
//**********************************************************************************

class cVuMeter
{
public:

    // =============================================================================
    // Constructor & Initialization
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor
    // Description: Default constructor for VU meter class
    cVuMeter() = default;

    // -----------------------------------------------------------------------------
    // Init
    // Description: Initializes the VU meter with specified sample rate
    // Parameters: sampleRate - Audio sample rate in Hz
    void Init(float sampleRate)
    {
        reset();
        setSampleRate(sampleRate);
    }

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Process
    // Description: Processes audio input to calculate current level, peak detection and clipping
    // Parameters: input - Audio sample to process
    void CalcPeakAndLevel(float input);

    // -----------------------------------------------------------------------------
    // getLevelPercent
    // Description: Gets current audio level as percentage (0.0 to 1.0)
    // Returns: Current level as linear percentage
    float getLevelPercent() const { return m_currentLevel; }

    // -----------------------------------------------------------------------------
    // getLevelDB
    // Description: Gets current audio level in decibels
    // Returns: Current level in dB
    float getLevelDB() const { return linearToDb(m_currentLevel); }

    // -----------------------------------------------------------------------------
    // getPeakPercent
    // Description: Gets peak audio level as percentage (0.0 to 1.0)
    // Returns: Peak level as linear percentage
    float getPeakPercent() const { return m_peakLevel; }

    // -----------------------------------------------------------------------------
    // getPeakDB
    // Description: Gets peak audio level in decibels
    // Returns: Peak level in dB
    float getPeakDB() const { return linearToDb(m_peakLevel); }

    // -----------------------------------------------------------------------------
    // getLevelPercentDB
    // Description: Gets current level normalized to dB range for display
    // Returns: Current level normalized between DB_MIN and DB_MAX
    float getLevelPercentDB() const
    {
        float db = getLevelDB();
        return dbToPercent(db);
    }

    // -----------------------------------------------------------------------------
    // getPeakPercentDB
    // Description: Gets peak level normalized to dB range for display
    // Returns: Peak level normalized between DB_MIN and DB_MAX
    float getPeakPercentDB() const
    {
        float db = getPeakDB();
        return dbToPercent(db);
    }

    // -----------------------------------------------------------------------------
    // isClipping
    // Description: Checks if audio is currently clipping
    // Returns: True if clipping is detected
    bool isClipping() const { return m_clipActive; }

    // -----------------------------------------------------------------------------
    // resetPeak
    // Description: Resets peak level to current level and clears hold counter
    void resetPeak()
    {
        m_peakLevel = m_currentLevel;
        m_peakHoldCounter = 0;
    }

    // -----------------------------------------------------------------------------
    // reset
    // Description: Resets all VU meter state to initial values
    void reset()
    {
        m_currentLevel = 0.0f;
        m_peakLevel = 0.0f;
        m_peakHoldCounter = 0;
        m_clipCounter = 0;
        m_clipActive = false;
    }

    // -----------------------------------------------------------------------------
    // setSampleRate
    // Description: Sets audio sample rate and updates internal coefficients
    // Parameters: sampleRate - Audio sample rate in Hz
    void setSampleRate(float sampleRate)
    {
        m_sampleRate = sampleRate;
        updateAttackRelease();
    }

protected:

    // =============================================================================
    // Protected Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // linearToDb
    // Description: Converts linear amplitude value to decibel scale
    // Parameters: linear - Linear amplitude value (0.0 to 1.0)
    // Returns: Value in decibels
    float linearToDb(float linear) const;

    // -----------------------------------------------------------------------------
    // dbToPercent
    // Description: Converts decibel value to percentage for meter display
    // Parameters: db - Value in decibels
    // Returns: Percentage value (0.0 to 1.0)
    float dbToPercent(float db) const;

    // -----------------------------------------------------------------------------
    // updateAttackRelease
    // Description: Updates attack and release coefficients based on sample rate
    void updateAttackRelease();

    // =============================================================================
    // Member Variables
    // =============================================================================

    float    m_sampleRate;         // Audio sample rate in Hz
    float    m_currentLevel;       // Current signal level (0.0 to 1.0)
    float    m_peakLevel;          // Peak signal level with hold (0.0 to 1.0)
    uint32_t m_peakHoldCounter;    // Counter for peak hold time in samples
    uint32_t m_clipCounter;        // Counter for clip hold time in samples
    bool     m_clipActive;         // Clipping detection flag

    uint32_t m_peakHoldSamples;    // Peak hold time in samples
    uint32_t m_clipHoldSamples;    // Clip hold time in samples
    float    m_attackCoeff;        // Attack coefficient for envelope
    float    m_releaseCoeff;       // Release coefficient for envelope
};

} // namespace DadDSP

//***End of file**************************************************************
