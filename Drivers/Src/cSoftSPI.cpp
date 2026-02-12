//==================================================================================
//==================================================================================
// File: cSoftSPI.cpp
// Description: Software-emulated 3-wire SPI interface implementation
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cSoftSPI.h"
#include "GPIO.h"
// Pin manipulation macros
#define setMOSI()   m_Port_MOSI->BSRR = (m_Pin_MOSI)
#define resetMOSI() m_Port_MOSI->BSRR = ((uint32_t)(m_Pin_MOSI) << 16U)

#define setCS()     m_Port_CS->BSRR = (m_Pin_CS)
#define resetCS()   m_Port_CS->BSRR = ((uint32_t)(m_Pin_CS) << 16U)

#define setCLK()    m_Port_CLK->BSRR = (m_Pin_CLK)
#define resetCLK()  m_Port_CLK->BSRR = ((uint32_t)(m_Pin_CLK) << 16U)

namespace DadDrivers {

//**********************************************************************************
// Class: cSoftSPI
// Description: Software-emulated 3-wire SPI interface implementation
//**********************************************************************************

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initializes the software SPI with specified GPIO pins and a timer
void cSoftSPI::Initialize(GPIO_TypeDef *MOSI_GPIO, uint16_t MOSI_Pin,
                         GPIO_TypeDef *CLK_GPIO, uint16_t CLK_Pin,
                         GPIO_TypeDef *CS_GPIO, uint16_t CS_Pin,
                         TIM_HandleTypeDef *phtim,
						 uint8_t NbBit) {

    // Store GPIO port and pin configurations
    m_Port_MOSI = MOSI_GPIO;    // MOSI GPIO port
    m_Pin_MOSI  = MOSI_Pin;     // MOSI pin number
    m_Port_CLK  = CLK_GPIO;     // Clock GPIO port
    m_Pin_CLK   = CLK_Pin;      // Clock pin number
    m_Port_CS   = CS_GPIO;      // Chip select GPIO port
    m_Pin_CS    = CS_Pin;       // Chip select pin number
    m_phtim     = phtim;        // Timer handle for timing control
    if(NbBit > 32) NbBit = 32;
    m_MSBmask = 1 << (NbBit-1);
    // Initialize transmission state variables
    m_DataBit = 0;              // Current bit position in transmission
    m_Data = 0;                 // Data being transmitted
    m_TransState = eTransState::Stop; // Current transmission state
    m_ValidNextData = 0;        // Flag indicating valid next data

    // Set initial pin states
    setCS();                 // Chip select high (inactive)
    resetCLK();              // Clock low (idle state)
    resetMOSI();             // MOSI low (idle state)
}

// -----------------------------------------------------------------------------
// Starts a new SPI transmission with the specified data
void cSoftSPI::Transmit(uint32_t Data) {
    __disable_irq(); // Disable interrupts for atomic operation

    if(m_TransState != eTransState::Stop) {
        // Transmission in progress - queue next data
        m_NextData = Data;          // Store next data to transmit
        m_ValidNextData = 1;        // Set valid next data flag
    } else {
        // No transmission in progress - start new transmission
        m_Data = Data;              // Store data to transmit
        m_TransState = eTransState::Start; // Begin transmission process
        // HAL_TIM_Base_Start_IT(m_phtim); // Start timer interrupts (commented out)
    }

    __enable_irq(); // Re-enable interrupts
}

// =============================================================================
// Timer Interrupt Callback
// =============================================================================

// -----------------------------------------------------------------------------
// Timer interrupt callback function to handle SPI transmission state machine
void cSoftSPI::TimerCallback() {
    switch (m_TransState) {
    case eTransState::Stop:
        // Transmission stopped - no action required
        return;
        break;

    case eTransState::Start:
        // Transition to chip select activation state
        m_TransState = eTransState::Cs_Down;
        return;
        break;

    case eTransState::Cs_Down:
        // Activate chip select and prepare for data transmission
        resetCS();                   // Lower chip select (activate)
        m_DataBit = m_MSBmask;       // Initialize bit mask to MSB
        m_TransState = eTransState::Data_Change; // Move to data output state
        break;

    case eTransState::Data_Change:
        // Output current data bit to MOSI line
        if ((m_DataBit & m_Data) == 0) {
            resetMOSI();             // Set MOSI low for '0' bit
        } else {
            setMOSI();               // Set MOSI high for '1' bit
        }
        m_DataBit >>= 1;                // Shift to next bit position
        m_TransState = eTransState::Clk_Up; // Move to clock rising edge
        break;

    case eTransState::Clk_Up:
        // Generate clock rising edge
        setCLK();                    // Raise clock signal
        m_TransState = eTransState::Nop; // Insert small delay
        break;

    case eTransState::Nop:
        // No operation state for timing delay
        m_TransState = eTransState::Clk_Down; // Move to clock falling edge
        break;

    case eTransState::Clk_Down:
        // Generate clock falling edge and check transmission completion
        resetCLK();                  // Lower clock signal
        if (m_DataBit == 0) {
            m_TransState = eTransState::Cs_Up; // All bits transmitted
        } else {
            m_TransState = eTransState::Data_Change; // Continue with next bit
        }
        break;

    case eTransState::Cs_Up:
        // Deactivate chip select and handle transmission completion
        setCS();                     // Raise chip select (deactivate)
        if(m_ValidNextData == 1) {
            // Load next data and continue transmission
            m_Data = m_NextData;        // Load queued data
            m_TransState = eTransState::Cs_Down; // Restart transmission
            m_ValidNextData = 0;        // Clear next data flag
        } else {
            // No more data - stop transmission
            m_TransState = eTransState::Stop;
            // HAL_TIM_Base_Stop_IT(m_phtim); // Stop timer interrupts (commented out)
        }
        break;
    }
}

} // namespace DadDrivers

//***End of file**************************************************************
