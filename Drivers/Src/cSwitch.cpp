//==================================================================================
//==================================================================================
// File: cSwitch.cpp
// Description: Switch debouncing and state tracking implementation
//
// Copyright (c) 2024 DadDrivers.
//==================================================================================
//==================================================================================

#include "cSwitch.h"

namespace DadDrivers {

//**********************************************************************************
// cSwitch Class Implementation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initialization and Configuration
// -----------------------------------------------------------------------------

//**********************************************************************************
// Method: Init
// Description: Initializes switch parameters and resets internal state
// Parameters:
//   - pPort: GPIO port for switch input
//   - Pin: GPIO pin for switch input
//   - DebounceRate: Debounce filtering rate
//   - UpdateInterval: State update interval
//   - MinPeriod: Minimum valid period between presses
//   - MaxPeriod: Maximum valid period between presses
//   - AbordMaxPeriod: Maximum period before timeout
//**********************************************************************************
void cSwitch::Init(GPIO_TypeDef* pPort, uint16_t Pin,
                   float  DebounceRate,
                   uint32_t UpdateInterval, uint32_t MinPeriod,
                   uint32_t MaxPeriod, uint32_t AbordMaxPeriod)
{
    // Configure hardware interface
    m_GPIO_Port = pPort;  // GPIO port for switch input
    m_GPIO_Pin = Pin;     // GPIO pin for switch input

    // Set timing parameters
    m_DebounceRate = DebounceRate;        // Debounce filtering rate
    m_UpdateInterval = UpdateInterval;    // State update interval
    m_MinPeriod = MinPeriod;              // Minimum valid period between presses
    m_MaxPeriod = MaxPeriod;              // Maximum valid period between presses
    m_AbordMaxPeriod = AbordMaxPeriod;    // Maximum period before timeout

    // Initialize state variables
    m_Stop = 1;               // Start in stopped state
    m_SwitchState = 0;        // Current switch state (0=released, 1=pressed)
    m_PressDuration = 0;      // Duration of current press
    m_CtPress = 0;            // Total press counter
    m_CurrentPeriod = 0;      // Current period between presses
    m_AvgPeriod = 0;          // Exponential moving average of periods
    m_PeriodUpdateCount = 0;  // Counter for period updates
    m_DebouncePeriod = -1;    // Debounce timer (-1 = inactive)
}

// -----------------------------------------------------------------------------
// Public Methods
// -----------------------------------------------------------------------------

//**********************************************************************************
// Method: Debounce
// Description: Processes switch input with debouncing and timing analysis
// Implements:
// - Contact bounce filtering
// - Minimum/maximum period validation
// - Exponential Moving Average (EMA) period calculation
// - Automatic timeout handling
//**********************************************************************************
ITCM void cSwitch::Debounce()
{
    // Step 1: Increment timing counters
    m_CurrentPeriod++;  // Increment period between presses
    if(m_SwitchState == 1) {
        m_PressDuration++;  // Track press duration when switch is pressed
    }

    // Step 2: Handle timeout condition
    if (m_CurrentPeriod > m_AbordMaxPeriod) {
        m_PeriodUpdateCount = 0;  // Reset period update counter
        m_AvgPeriod = 0;          // Reset average period
        m_Stop = 1;               // Enter stopped state
    }

    // Step 3: Read physical switch state (active low configuration)
    bool isSwitchPressed = (HAL_GPIO_ReadPin(m_GPIO_Port, m_GPIO_Pin) == GPIO_PIN_RESET);

    if (isSwitchPressed) {
        // Step 4: Detect new press events
        if (m_SwitchState == 0) {
            m_SwitchState = 1;    // Set pressed state
            m_PressDuration = 0;  // Reset press duration
            m_CtPress++;          // Increment press counter

            // Step 5: Handle first press after timeout
            if (m_Stop == 1) {
                m_Stop = 0;           // Exit stopped state
                m_CurrentPeriod = 0;  // Reset current period
            }

            // Step 6: Validate and process period between presses
            if ((m_CurrentPeriod > m_MinPeriod) && (m_CurrentPeriod < m_MaxPeriod)) {
                // Calculate new EMA period with smoothing factor
                float newAvgPeriod = (float)m_CurrentPeriod;
                if (m_PeriodUpdateCount > 0) {
                    const float ALPHA = 0.2f; // EMA smoothing factor (between 0 and 1)
                    // Apply EMA formula: newAvg = α * current + (1-α) * previous
                    newAvgPeriod = ALPHA * newAvgPeriod + (1.0f - ALPHA) * m_AvgPeriod;

                    // Step 7: Limit variations to ±15% for stability
                    float minAllowed = m_AvgPeriod * 0.85f;  // Lower variation limit
                    float maxAllowed = m_AvgPeriod * 1.15f;  // Upper variation limit
                    if (newAvgPeriod < minAllowed) newAvgPeriod = minAllowed;
                    if (newAvgPeriod > maxAllowed) newAvgPeriod = maxAllowed;
                }
                // Update tracking variables
                m_AvgPeriod = newAvgPeriod;  // Store new average period
                m_PeriodUpdateCount++;       // Increment update counter
                m_CurrentPeriod = 0;         // Reset current period after processing
            }
        }
    } else if(m_SwitchState == 1){
        // Step 8: Handle switch release with debounce delay
        if((m_DebouncePeriod == -1)){
            m_DebouncePeriod = m_UpdateInterval;  // Start debounce timer
        } else {
            m_DebouncePeriod--;  // Decrement debounce timer
            if(m_DebouncePeriod <= 0){
                m_SwitchState = 0;      // Set released state
                m_DebouncePeriod = -1;  // Reset debounce timer
            }
        }
    }
}

} // namespace DadDrivers

//***End of file**************************************************************
