//==================================================================================
//==================================================================================
// File: cSwitch.h
// Description: Switch debouncing and state tracking class with configurable timing parameters
//
// Copyright (c) 2024 DadDrivers.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "Sections.h"
#include <cstdint>

namespace DadDrivers {

//**********************************************************************************
// cSwitch - Digital switch debouncer with advanced timing analysis
//**********************************************************************************
class cSwitch
{
  public:
    // =============================================================================
    // Constructor/Destructor
    // =============================================================================

    cSwitch() {}   // Default constructor
    ~cSwitch() {}  // Default destructor

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initialization and Configuration
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: Init
    // Description: Initializes switch with hardware parameters and timing configuration
    // Parameters:
    //   - pPort: GPIO port handle
    //   - Pin: GPIO pin number
    //   - DebounceRate: Debounce rate in frequencies
    //   - UpdateInterval: Debounce time in samples
    //   - MinPeriod: Minimum valid period between presses (anti-noise)
    //   - MaxPeriod: Maximum valid period between presses
    //   - AbordMaxPeriod: Absolute timeout period for resetting tracking
    //**********************************************************************************
    void Init(GPIO_TypeDef* pPort, uint16_t Pin,
              float    DebounceRate,
              uint32_t UpdateInterval,
              uint32_t MinPeriod,
              uint32_t MaxPeriod,
              uint32_t AbordMaxPeriod);

    //**********************************************************************************
    // Method: Debounce
    // Description: Processes switch input with debouncing and state tracking
    // Note: Must be called regularly (typically from an interrupt service routine)
    //**********************************************************************************
    void Debounce();

    // -----------------------------------------------------------------------------
    // State Access Methods
    // -----------------------------------------------------------------------------

    //**********************************************************************************
    // Method: getState
    // Description: Gets current debounced switch state
    // Returns: Current switch state (0=released, 1=pressed)
    //**********************************************************************************
    inline uint8_t getState() const {
        return m_SwitchState;  // Current debounced switch state
    }

    //**********************************************************************************
    // Method: getState (with duration)
    // Description: Gets switch state and calculates press duration in milliseconds
    // Parameters:
    //   - PressDuration: Output parameter for press duration in milliseconds
    // Returns: Current switch state (0=released, 1=pressed)
    //**********************************************************************************
    inline uint8_t getState(float &PressDuration) const {
        PressDuration = static_cast<float>(m_PressDuration) / m_DebounceRate;  // Convert samples to milliseconds
        return m_SwitchState;  // Current debounced switch state
    }

    //**********************************************************************************
    // Method: getPeriodUpdateCount
    // Description: Gets count of valid period updates for reliability metrics
    // Returns: Count of valid period updates
    //**********************************************************************************
    inline uint32_t getPeriodUpdateCount() const {
        return m_PeriodUpdateCount;  // Valid period update counter
    }

    //**********************************************************************************
    // Method: getPressCount
    // Description: Gets total number of detected presses since initialization
    // Returns: Total press count
    //**********************************************************************************
    inline uint64_t getPressCount() const {
        return m_CtPress;  // Total press counter
    }

    //**********************************************************************************
    // Method: getPressPeriod
    // Description: Calculates average press frequency in Hz
    // Returns: Press frequency (0 if no valid period established)
    //**********************************************************************************
    inline float getPressPeriod() const {
        return (m_AvgPeriod == 0) ? 0.0f : (m_AvgPeriod / m_DebounceRate);  // Convert samples to frequency
    }

  private:
    // =============================================================================
    // Private Member Variables
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Hardware Configuration
    // -----------------------------------------------------------------------------
    GPIO_TypeDef* m_GPIO_Port;  // Hardware GPIO port reference
    uint16_t      m_GPIO_Pin;   // Hardware GPIO pin number
    float         m_DebounceRate; // Debounce rate in frequencies

    // -----------------------------------------------------------------------------
    // Timing Configuration
    // -----------------------------------------------------------------------------
    uint32_t      m_UpdateInterval;   // Debounce interval in samples
    int32_t       m_DebouncePeriod;   // Debounce counter samples
    uint32_t      m_MinPeriod;        // Minimum valid period between presses
    uint32_t      m_MaxPeriod;        // Maximum valid period between presses
    uint32_t      m_AbordMaxPeriod;   // Absolute timeout period
    uint8_t       m_Stop;             // State tracking flag

    // -----------------------------------------------------------------------------
    // Runtime State Variables
    // -----------------------------------------------------------------------------
    uint8_t       m_SwitchState;      // Current debounced state (0=released, 1=pressed)
    uint32_t      m_PressDuration;    // Current press duration in samples
    uint64_t      m_CtPress;          // Total press counter

    // -----------------------------------------------------------------------------
    // Period Analysis Variables
    // -----------------------------------------------------------------------------
    uint32_t      m_CurrentPeriod;    // Time since last valid press in samples
    float         m_AvgPeriod;        // EMA-smoothed period between presses
    uint32_t      m_PeriodUpdateCount; // Valid period update counter
};

} // namespace DadDrivers

//***End of file**************************************************************
