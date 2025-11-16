//==================================================================================
//==================================================================================
// File: Serialize.h
// Description: Header for serialization/deserialization utilities for data types
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include "main.h"

namespace DadPersistentStorage {

// Forward declaration
class cSerialize;

//**********************************************************************************
// Class cSerializedObject
//**********************************************************************************

// -----------------------------------------------------------------------------
// Abstract base class for serializable objects
class cSerializedObject {
public:
    virtual ~cSerializedObject() {};

    // -------------------------------------------------------------------------
    // Serialize the object
    virtual void Save(cSerialize* pSerializer) = 0;

    // -------------------------------------------------------------------------
    // Deserialize the object
    virtual void Restore(cSerialize* pSerializer) = 0;

    // -------------------------------------------------------------------------
    // Check if the object is dirty
    virtual bool isDirty() = 0;
};

//**********************************************************************************
// Class cSerialize
//**********************************************************************************

// -----------------------------------------------------------------------------
// Serialization utility class for data types
class cSerialize {
public:
    // =========================================================================
    // Constructor/Destructor
    // =========================================================================

    // -------------------------------------------------------------------------
    // Constructor - initialize empty buffer
    cSerialize() {
        buffer.clear();
        readIndex = 0;
    }

    // =========================================================================
    // Template Methods
    // =========================================================================

    // -------------------------------------------------------------------------
    // Template method for pushing data into the buffer
    template<typename T>
    void Push(const T& value) {
        PushRaw(&value, sizeof(T));
    }
    
    // -------------------------------------------------------------------------
    // Template method for pulling data from the buffer
    template<typename T>
    void Pull(T& value) {
        PullRaw(&value, sizeof(T));
    }

    // =========================================================================
    // Raw Data Operations
    // =========================================================================

    // -------------------------------------------------------------------------
    // Push raw data into the buffer
    void PushRaw(const void* data, size_t size);
    
    // -------------------------------------------------------------------------
    // Pull raw data from the buffer
    void PullRaw(void* data, size_t size);

    // =========================================================================
    // String Operations
    // =========================================================================

    // -------------------------------------------------------------------------
    // Push a string into the buffer
    void Push(const std::string& str);

    // -------------------------------------------------------------------------
    // Pull a string from the buffer
    std::string PullString();

    // =========================================================================
    // Buffer Management
    // =========================================================================

    // -------------------------------------------------------------------------
    // Get the size and content of the buffer
    size_t getBuffer(const uint8_t** outBuffer) const;

    // -------------------------------------------------------------------------
    // Set the buffer with new data
    void setBuffer(const void* data, size_t size);

    // -------------------------------------------------------------------------
    // Clear the buffer
    void clearBuffer() {
        buffer.clear();
        readIndex = 0;
    }

    // -------------------------------------------------------------------------
    // Reset the read position to the beginning of the buffer
    void resetReadIndex() {
        readIndex = 0;
    }

private:
    // =========================================================================
    // Member Variables
    // =========================================================================

    std::vector<uint8_t> buffer;    // Buffer to hold serialized data
    size_t readIndex = 0;           // Current read position in the buffer
};

} // namespace DadPersistentStorage

//***End of file**************************************************************
