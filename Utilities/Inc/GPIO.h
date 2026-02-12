//==================================================================================
//==================================================================================
// File: GPIO.h
// Description: GPIO utilities and pin manipulation macros
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "stm32h7xx_hal_gpio.h"

//**********************************************************************************
// GPIO Macros
//
// Provides simplified macros for GPIO pin manipulation using register
//**********************************************************************************
#define GPIO_READ_PIN(GPIOx, PIN_MASK) \
    ((GPIOx->IDR & (PIN_MASK)) != 0)

#define GPIO_SET_PIN(GPIOx, PIN_MASK) \
    ((GPIOx)->BSRR = (PIN_MASK))

#define GPIO_RESET_PIN(GPIOx, PIN_MASK) \
    ((GPIOx)->BSRR = ((uint32_t)(PIN_MASK) << 16U))

#define GPIO_WRITE_PIN(GPIOx, PIN_MASK, STATE) \
    do { \
        if ((STATE) != 0) { \
            (GPIOx)->BSRR = (PIN_MASK); \
        } else { \
            (GPIOx)->BSRR = ((uint32_t)(PIN_MASK) << 16U); \
        } \
    } while(0)

#define GPIO_TOGGLE_PIN(GPIOx, PIN_MASK) \
    ((GPIOx)->ODR ^= (PIN_MASK))

// -----------------------------------------------------------------------------
// Macro: SetPIN
// Description: Sets specified GPIO pin to HIGH state
// -----------------------------------------------------------------------------
#define SetPIN(PIN)\
    ((PIN##_GPIO_Port)->BSRR = (PIN##_Pin))

// -----------------------------------------------------------------------------
// Macro: ResetPIN
// Description: Sets specified GPIO pin to LOW state
// -----------------------------------------------------------------------------
#define ResetPIN(PIN)\
    ((PIN##_GPIO_Port)->BSRR = ((uint32_t)(PIN##_Pin) << 16U))

// -----------------------------------------------------------------------------
// Macro: TogglePIN
// Description: Toggles specified GPIO pin state (HIGH to LOW or LOW to HIGH)
// -----------------------------------------------------------------------------
#define TogglePIN(PIN)\
    ((PIN##_GPIO_Port)->ODR ^= (PIN##_Pin))

//***End of file**************************************************************
