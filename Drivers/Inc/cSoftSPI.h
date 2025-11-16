//==================================================================================
//==================================================================================
// File: cSoftSPI.h
// Description: Software-emulated 3-wire SPI interface header
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "Sections.h"

namespace DadDrivers {

//**********************************************************************************
// Forward Declarations
//**********************************************************************************

// -----------------------------------------------------------------------------
// Timer interrupt callback function to handle SPI transmission
ITCM void TimerCallback();

//**********************************************************************************
// Enumerations
//**********************************************************************************

// -----------------------------------------------------------------------------
// Transmission states of the SPI interface
enum class eTransState {
    Stop,        // Transmission stopped
    Start,       // Start transmission
    Cs_Down,     // Chip select goes low
    Data_Change, // Data bit is updated
    Clk_Up,      // Clock signal goes high
    Nop,         // No operation (small delay)
    Clk_Down,    // Clock signal goes low
    Cs_Up        // Chip select goes high (end of transmission)
};

//**********************************************************************************
// Class: cSoftSPI
// Description: Software-emulated 3-wire SPI interface
//**********************************************************************************

class cSoftSPI {
public:
    // =============================================================================
    // Constructor
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Default constructor
    cSoftSPI() {}

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initializes the software SPI with specified GPIO pins and a timer
    void Initialize(GPIO_TypeDef *MOSI_GPIO, uint16_t MOSI_Pin,
                    GPIO_TypeDef *CLK_GPIO, uint16_t CLK_Pin,
                    GPIO_TypeDef *CS_GPIO, uint16_t CS_Pin,
                    TIM_HandleTypeDef *phtim,
					uint8_t NbBit = 16);

    // -----------------------------------------------------------------------------
    // Timer interrupt callback function to handle SPI transmission
    ITCM void TimerCallback();

    // -----------------------------------------------------------------------------
    // Starts a new SPI transmission
    void Transmit(uint32_t Data);

protected:
    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    // -----------------------------------------------------------------------------
    // GPIO Configuration Section
    GPIO_TypeDef*      m_Port_MOSI; // MOSI GPIO port
    uint16_t           m_Pin_MOSI;  // MOSI pin number
    GPIO_TypeDef*      m_Port_CLK;  // Clock GPIO port
    uint16_t           m_Pin_CLK;   // Clock pin number
    GPIO_TypeDef*      m_Port_CS;   // Chip select GPIO port
    uint16_t           m_Pin_CS;    // Chip select pin number
    TIM_HandleTypeDef* m_phtim;     // Pointer to timer handle for timing control

    // -----------------------------------------------------------------------------
    // Transmission Data Section
    uint32_t           m_Data;          // Data to be transmitted
    uint32_t           m_DataBit;       // Current bit being transmitted
    volatile eTransState m_TransState;  // State of the transmission
    uint32_t           m_NextData;      // Next data to send
    uint8_t            m_ValidNextData; // Flag indicating next data is valid
    uint32_t		   m_MSBmask;
};

} // namespace DadDrivers

//***End of file**************************************************************
