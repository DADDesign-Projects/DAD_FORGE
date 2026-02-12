//==================================================================================
//==================================================================================
// File: cDelayLine.h
// Description: Delay line buffer implementation for digital signal processing
//              applications including audio effects and time-based signal modifications
//
// Copyright (c) 2024-2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "Sections.h"
#include <math.h>
#include <cstring>

namespace DadDSP {

//**********************************************************************************
// class cDelayLine
//**********************************************************************************

class cDelayLine
{
public:
    // -----------------------------------------------------------------------------
    // Constructor / destructor
    cDelayLine() {};
    ~cDelayLine() {};

    // -----------------------------------------------------------------------------
    // Initializes the FIFO buffer
    void Initialize(float* buffer, uint32_t bufferSize);

    // -----------------------------------------------------------------------------
    // Clears the FIFO buffer
    void Clear();

    // -----------------------------------------------------------------------------
    // Adds an element to the delay line
    void Push(float inputSample);

    // -----------------------------------------------------------------------------
    // Retrieves a sample without interpolation
    float Pull(uint32_t delay);

    // -----------------------------------------------------------------------------
    // Retrieves a sample with interpolation
    float Pull(float delay);

private:
    // =============================================================================
    // Data Members
    // =============================================================================

    float*  m_Buffer = nullptr;       // Pointer to allocated memory buffer
    int32_t m_NumElements = 0;        // Number of elements in the buffer
    int32_t m_CurrentIndex = 0;       // Current index (zero delay position)
};

} // namespace DadDSP

//***End of file**************************************************************
