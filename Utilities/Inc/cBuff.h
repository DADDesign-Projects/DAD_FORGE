//==================================================================================
//==================================================================================
// File: cBuff.h
// Description: Memory buffer management class for FIFO-style byte storage
//
// Copyright (c) 2024-2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once
#include "stdint.h"

namespace DadUtilities {

//**********************************************************************************
// cBuff
//
// Implements a simple FIFO-style memory buffer for storing bytes, providing
// basic operations to add data, clear the buffer, and retrieve its pointer
// and data count.
//**********************************************************************************

class cBuff {
public:
    // =============================================================================
    // Constructor / Destructor
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor
    // Description: Allocates the buffer with the specified size in bytes
    // -----------------------------------------------------------------------------
    cBuff(uint16_t TailleFIFO) {
        m_pStartBuff = new uint8_t[TailleFIFO];  // Allocate buffer memory
        m_pEndBuff   = &m_pStartBuff[TailleFIFO]; // Set pointer to end of buffer
        Clear();                                  // Initialize buffer state
    };

    // -----------------------------------------------------------------------------
    // Destructor
    // Description: Frees the allocated buffer and resets internal pointers
    // -----------------------------------------------------------------------------
    ~cBuff() {
        delete [] m_pStartBuff;
        m_pStartBuff = nullptr;
        m_pEndBuff = nullptr;
        Clear();
    };

    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Function: addData
    // Description: Adds a byte to the buffer
    // Returns: true if successful, false if buffer is full
    // -----------------------------------------------------------------------------
    inline bool addData(uint8_t Data) {
        // Check if buffer has available space
        if (m_pNextData < m_pEndBuff) {
            *m_pNextData++ = Data;  // Store byte and increment pointer
            m_NbData++;             // Increase stored data count
            return true;
        } else {
            return false;           // No space left in buffer
        }
    }

    // -----------------------------------------------------------------------------
    // Function: Clear
    // Description: Clears the buffer content
    // -----------------------------------------------------------------------------
    inline void Clear() {
        m_pNextData = m_pStartBuff;  // Reset write pointer to start
        m_NbData = 0;                // Reset data counter
    }

    // -----------------------------------------------------------------------------
    // Function: getBuffPtr
    // Description: Gets pointer to the start of the buffer
    // -----------------------------------------------------------------------------
    inline uint8_t* getBuffPtr() {
        return m_pStartBuff;
    }

    // -----------------------------------------------------------------------------
    // Function: getNbData
    // Description: Gets the number of bytes currently stored in the buffer
    // -----------------------------------------------------------------------------
    inline uint16_t getNbData() {
        return m_NbData;
    }

protected:
    // =============================================================================
    // Member Variables
    // =============================================================================

    uint8_t* m_pStartBuff = nullptr;  // Pointer to the start of the buffer memory
    uint8_t* m_pEndBuff   = nullptr;  // Pointer to the end of the buffer memory
    uint8_t* m_pNextData  = nullptr;  // Pointer to the next free slot for data
    uint16_t m_NbData = 0;            // Number of bytes stored in the buffer
};

} // namespace DadUtilities
//***End of file**************************************************************
