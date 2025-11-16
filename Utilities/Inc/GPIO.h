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
// Provides simplified macros for GPIO pin manipulation using STM32 HAL functions
//**********************************************************************************

// -----------------------------------------------------------------------------
// Macro: SetPIN
// Description: Sets specified GPIO pin to HIGH state
// -----------------------------------------------------------------------------
#define SetPIN(PIN) HAL_GPIO_WritePin(PIN##_GPIO_Port, PIN##_Pin, GPIO_PIN_SET)

// -----------------------------------------------------------------------------
// Macro: ResetPIN
// Description: Sets specified GPIO pin to LOW state
// -----------------------------------------------------------------------------
#define ResetPIN(PIN) HAL_GPIO_WritePin(PIN##_GPIO_Port, PIN##_Pin, GPIO_PIN_RESET)

// -----------------------------------------------------------------------------
// Macro: TogglePIN
// Description: Toggles specified GPIO pin state (HIGH to LOW or LOW to HIGH)
// -----------------------------------------------------------------------------
#define TogglePIN(PIN) HAL_GPIO_TogglePin(PIN##_GPIO_Port, PIN##_Pin)

//***End of file**************************************************************
