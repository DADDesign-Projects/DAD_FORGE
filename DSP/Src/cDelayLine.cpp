//==================================================================================
//==================================================================================
// File: cDelayLine.cpp
// Description: Implementation of a delay line buffer for digital signal processing
//              applications including audio effects and time-based signal modifications
//
// Copyright (c) 2024-2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cDelayLine.h"

namespace DadDSP {

//**********************************************************************************
// class cDelayLine
//**********************************************************************************

// -----------------------------------------------------------------------------
// Initializes the delay line with external buffer
void cDelayLine::Initialize(float* buffer, uint32_t bufferSize) {
    m_Buffer       = buffer;           // Pointer to external buffer
    m_NumElements  = bufferSize; 	   // Buffer size with safety offset
    m_CurrentIndex = 0;                // Initialize current position
}

// -----------------------------------------------------------------------------
// Clears the entire buffer by setting all values to zero
void cDelayLine::Clear() {
    memset(m_Buffer, 0, (m_NumElements * sizeof(float))); // Zero out buffer memory
}

// -----------------------------------------------------------------------------
// Adds a new sample to the delay line
void cDelayLine::Push(float inputSample) {
    if (m_Buffer) {
        // Increment current index with circular buffer wrapping
        m_CurrentIndex++;
        if (m_CurrentIndex == m_NumElements)
            m_CurrentIndex = 0; // Wrap around to beginning

        m_Buffer[m_CurrentIndex] = inputSample; // Store new sample
    }
}

// -----------------------------------------------------------------------------
// Retrieves a delayed sample without interpolation
float cDelayLine::Pull(uint32_t delay) {
    //assert(delay < m_NumElements);
    if (m_Buffer) {
        // Calculate output index with circular buffer wrapping
        int32_t outputIndex = m_CurrentIndex - delay;
        if (outputIndex < 0)
            outputIndex += m_NumElements; // Handle negative index wrap-around

        return m_Buffer[outputIndex]; // Return delayed sample
    }
    else return 0.0f; // Return zero if buffer not initialized
}

// -----------------------------------------------------------------------------
// Retrieves a delayed sample with linear interpolation
float cDelayLine::Pull(float delay) {
    //assert(delay < m_NumElements);

    if (m_Buffer) {
        // Calculate integer part and fractional part manually
        int32_t delayInt = static_cast<int32_t>(delay);
        float interpFactor = delay - delayInt;

        // Return exact sample if no interpolation needed
        if (interpFactor == 0.0f)
            return Pull(static_cast<uint32_t>(delay));

        // Calculate indices for the two adjacent samples
        // Instead of ceil(delay), we use delayInt + 1 when delay is not an integer
        int32_t index1 = m_CurrentIndex - delayInt - 1;
        if (index1 < 0)
            index1 += m_NumElements; // Wrap around if negative

        int32_t index2 = index1 + 1;
        if (index2 == m_NumElements)
            index2 = 0; // Wrap around to beginning

        // Retrieve the two adjacent samples
        float sample1 = m_Buffer[index1];
        float sample2 = m_Buffer[index2];

        // Perform linear interpolation between samples
        float interpolatedValue = sample2 + ((sample1 - sample2) * interpFactor);

        return interpolatedValue; // Return interpolated value
    }
    else return 0.0f; // Return zero if buffer not initialized
}
} // namespace DadDSP

//***End of file**************************************************************
