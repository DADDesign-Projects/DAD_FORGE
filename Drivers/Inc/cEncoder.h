//==================================================================================
//==================================================================================
// File: cEncoder.h
// Description: Rotary encoder management with debouncing functionality
//
//  Copyright (c) 2024, 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "Sections.h"

namespace DadDrivers {

//**********************************************************************************
// class cEncoder
//**********************************************************************************

class cEncoder
{
public:
    // ==============================================================================
    // Constructor and destructor
    // ==============================================================================

    // ------------------------------------------------------------------------------
    // Default constructor
    cEncoder() {}

    // ------------------------------------------------------------------------------
    // Destructor
    ~cEncoder() {}

    // ==============================================================================
    // Public methods
    // ==============================================================================

    // ------------------------------------------------------------------------------
    // Initializes rotary encoder with GPIO pins and timing parameters
    // pAPort, APIn: GPIO port and pin for encoder channel A
    // pBPort, BPIn: GPIO port and pin for encoder channel B
    // pSWPort, SWPIn: GPIO port and pin for encoder switch
    // EncoderUpdatePeriod: Update period for encoder processing
    // SwitchUpdatePeriod: Update period for switch processing
    void Init(GPIO_TypeDef* pAPort, uint16_t APIn,
              GPIO_TypeDef* pBPort, uint16_t BPIn,
              GPIO_TypeDef* pSWPort, uint16_t SWPIn,
              uint32_t EncoderUpdatePeriod, uint32_t SwitchUpdatePeriod);

    // ------------------------------------------------------------------------------
    // Processes encoder and switch debouncing, updates position and state
    ITCM void Debounce();

    // ------------------------------------------------------------------------------
    // Returns current encoder increment and resets the counter
    // return: Signed increment value since last read
    inline int8_t getIncrement() {
        uint8_t result = m_Inc;                // Store current increment value
        m_Inc = 0;                             // Reset increment counter
        return result;                         // Return stored value
    }

    // ------------------------------------------------------------------------------
    // Returns current state of the encoder switch
    // return: Switch state (0 = released, 1 = pressed)
    inline uint8_t getSwitchState() const {
        return m_SwitchState;                  // Return current switch state
    }

protected:
    // ==============================================================================
    // Protected data members
    // ==============================================================================

    // GPIO configuration members
    GPIO_TypeDef*  m_pGPIO_APort;              // GPIO port for encoder channel A
    uint16_t       m_GPIO_APin;                // GPIO pin for encoder channel A
    GPIO_TypeDef*  m_pGPIO_BPort;              // GPIO port for encoder channel B
    uint16_t       m_GPIO_BPin;                // GPIO pin for encoder channel B
    GPIO_TypeDef*  m_pGPIO_SWPort;             // GPIO port for encoder switch
    uint16_t       m_GPIO_SWPin;               // GPIO pin for encoder switch

    // Encoder processing members
    uint32_t       m_ctEncoderPeriod;          // Counter for encoder debouncing
    uint32_t       m_EncoderUpdatePeriod;      // Update period for encoder processing
    uint8_t        m_A;                        // Current state of encoder channel A
    uint8_t        m_B;                        // Current state of encoder channel B
    int8_t         m_Inc;                      // Encoder increment value

    // Switch processing members
    uint8_t        m_SwitchState;              // Current state of the switch
    uint32_t       m_ctSwitchPeriod;           // Counter for switch debouncing
    uint32_t       m_SwitchUpdatePeriod;       // Update period for switch processing
    int32_t        m_ctSwitchIntegrate;        // Switch signal integration counter
};

} // namespace DadDrivers

//***End of file**************************************************************
