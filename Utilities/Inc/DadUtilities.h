//==================================================================================
//==================================================================================
// File: DadUtilities.h
// Description: Various utility functions for the DAD project
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include <cmath>

// -----------------------------------------------------------------------------
// Function: DadClampMAX
// Description: Clamp value to a maximum value
// Parameters:
//   - value: Value to clamp (modified in place)
//   - max: Maximum allowed value
// -----------------------------------------------------------------------------
template<typename T>
constexpr void DadClampMAX(T& value, T max) {
    if (value > max) value = max;
}

// -----------------------------------------------------------------------------
// Function: DadClampMIN
// Description: Clamp value to a minimum value
// Parameters:
//   - value: Value to clamp (modified in place)
//   - min: Minimum allowed value
// -----------------------------------------------------------------------------
template<typename T>
constexpr void DadClampMIN(T& value, T min) {
    if (value < min) value = min;
}

// -----------------------------------------------------------------------------
// Function: DadClamp
// Description: Clamp a value between min and max values
// Parameters:
//   - value: Value to clamp (modified in place)
//   - min: Minimum allowed value
//   - max: Maximum allowed value
// -----------------------------------------------------------------------------
template<typename T>
constexpr void DadClamp(T& value, T min, T max) {
    if (value < min) value = min;
    else if (value > max) value = max;
}

// -----------------------------------------------------------------------------
// Function: isZero
// Description: Check if a float value is approximately zero
// Parameters:
//   - value: Float value to check
// Returns: true if absolute value <= 1e-7f, false otherwise
// -----------------------------------------------------------------------------
inline bool isZero(float value) {
    return std::fabs(value) <= 1e-7f;
}

//***End of file**************************************************************
