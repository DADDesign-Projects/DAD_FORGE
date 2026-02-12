//==================================================================================
//==================================================================================
// File: cEncoder.cpp
// Description: Management of a rotary encoder with debouncing functionality
//
// Copyright (c) 2024, 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cEncoder.h"

namespace DadDrivers {

#define GPIO_READ_PIN(GPIOx, PIN_MASK) \
    ((GPIOx->IDR & (PIN_MASK)) != 0)

//**********************************************************************************
// class cEncoder
//**********************************************************************************

// ==============================================================================
// Initialization methods
// ==============================================================================

// ------------------------------------------------------------------------------
// Initializes the rotary encoder with specified GPIO pins and update periods
// pAPort, APIn: GPIO port and pin for channel A
// pBPort, BPIn: GPIO port and pin for channel B
// pSWPort, SWPIn: GPIO port and pin for switch
// EncoderUpdatePeriod: Update period for encoder in 1/SAMPLING_RATE units
// SwitchUpdatePeriod: Update period for switch in 1/SAMPLING_RATE units
void cEncoder::Init(GPIO_TypeDef* pAPort, uint16_t APIn,
                    GPIO_TypeDef* pBPort, uint16_t BPIn,
                    GPIO_TypeDef* pSWPort, uint16_t SWPIn,
                    uint32_t EncoderUpdatePeriod, uint32_t SwitchUpdatePeriod)
{
    m_pGPIO_APort = pAPort;                    // GPIO port for encoder channel A
    m_GPIO_APin = APIn;                        // GPIO pin for encoder channel A
    m_pGPIO_BPort = pBPort;                    // GPIO port for encoder channel B
    m_GPIO_BPin = BPIn;                        // GPIO pin for encoder channel B
    m_pGPIO_SWPort = pSWPort;                  // GPIO port for encoder switch
    m_GPIO_SWPin = SWPIn;                      // GPIO pin for encoder switch
    m_A = 0;                                   // Current state of channel A
    m_B = 0;                                   // Current state of channel B
    m_Inc = 0;                                 // Encoder increment value
    m_EncoderUpdatePeriod = EncoderUpdatePeriod; // Encoder update period
    m_SwitchUpdatePeriod = SwitchUpdatePeriod;   // Switch update period
    m_ctEncoderPeriod = 0;                     // Counter for encoder debouncing
    m_ctSwitchPeriod = 0;                      // Counter for switch debouncing
    m_ctSwitchIntegrate = 0;                   // Counter for switch signal integration
}

// ==============================================================================
// Processing methods
// ==============================================================================

// ------------------------------------------------------------------------------
// Handles debouncing for rotary encoder and switch, updates encoder position
// and switch state based on elapsed time since last update
void cEncoder::Debounce()
{
    // Process encoder position reading
    m_ctEncoderPeriod++;                       // Increment encoder period counter

    // Check if it's time to update encoder position
    if (m_ctEncoderPeriod > m_EncoderUpdatePeriod) {
        m_ctEncoderPeriod = 0;                 // Reset encoder period counter

        // Update encoder channel states with debouncing
        m_A = ((m_A << 1) | (uint8_t)GPIO_READ_PIN(m_pGPIO_APort, m_GPIO_APin)) & 0x03;
        m_B = ((m_B << 1) | (uint8_t)GPIO_READ_PIN(m_pGPIO_BPort, m_GPIO_BPin)) & 0x03;

        // Detect encoder rotation direction and update increment
        if ((m_A == 0x02) && (m_B == 0x00)) {
            m_Inc += 1;                        // Clockwise rotation detected
        }
        else if ((m_B == 0x02) && (m_A == 0x00)) {
            m_Inc -= 1;                        // Counter-clockwise rotation detected
        }
    }

    // Process switch debouncing with integration
    constexpr int32_t INTEGRATION_FACTOR = 10; // Integration threshold for stable state
    m_ctSwitchPeriod++;                        // Increment switch period counter

    // Check if it's time to update switch state
    if (m_ctSwitchPeriod > m_SwitchUpdatePeriod) {
        m_ctSwitchPeriod = 0;                  // Reset switch period counter

        // Integrate switch signal over time for stable reading
        if (GPIO_READ_PIN(m_pGPIO_SWPort, m_GPIO_SWPin) == GPIO_PIN_SET) {
            m_ctSwitchIntegrate++;             // Increment integration counter for released state
            if (m_ctSwitchIntegrate > INTEGRATION_FACTOR) {
                m_ctSwitchIntegrate = INTEGRATION_FACTOR; // Clamp to maximum
                m_SwitchState = 0;             // Switch is released
            }
        }
        else {
            m_ctSwitchIntegrate--;             // Decrement integration counter for pressed state
            if (m_ctSwitchIntegrate < -INTEGRATION_FACTOR) {
                m_ctSwitchIntegrate = -INTEGRATION_FACTOR; // Clamp to minimum
                m_SwitchState = 1;             // Switch is pressed
            }
        }
    }
}

} // namespace DadDrivers

//***End of file**************************************************************
