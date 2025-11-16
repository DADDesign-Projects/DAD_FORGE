//==================================================================================
//==================================================================================
// File: Serialize.cpp
// Description: Implementation of serialization/deserialization utilities for data types
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "Serialize.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Class cSerialize
//**********************************************************************************

// =============================================================================
// Raw Data Operations
// =============================================================================

// -----------------------------------------------------------------------------
// Push raw data into the buffer
void cSerialize::PushRaw(const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    buffer.insert(buffer.end(), bytes, bytes + size);
}

// -----------------------------------------------------------------------------
// Pull raw data from the buffer
void cSerialize::PullRaw(void* data, size_t size) {
    if (readIndex + size <= buffer.size()) {
        std::memcpy(data, buffer.data() + readIndex, size);
        readIndex += size;
    }
}

// =============================================================================
// String Operations
// =============================================================================

// -----------------------------------------------------------------------------
// Push a string into the buffer
void cSerialize::Push(const std::string& str) {
    uint32_t length = static_cast<uint32_t>(str.size());
    Push(length);                           // Push string length first
    PushRaw(str.data(), length);            // Push string character data
}

// -----------------------------------------------------------------------------
// Pull a string from the buffer
std::string cSerialize::PullString() {
    uint32_t length = 0;
    Pull(length);                           // Pull string length first

    if (readIndex + length <= buffer.size()) {
        std::string str(length, '\0');
        PullRaw(&str[0], length);           // Pull string character data
        return str;
    }
    return std::string();                   // Return empty string on error
}

// =============================================================================
// Buffer Management
// =============================================================================

// -----------------------------------------------------------------------------
// Get the size and content of the buffer
size_t cSerialize::getBuffer(const uint8_t** outBuffer) const {
    *outBuffer = buffer.data();
    return buffer.size();
}

// -----------------------------------------------------------------------------
// Set the buffer with new data
void cSerialize::setBuffer(const void* data, size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    buffer.assign(bytes, bytes + size);     // Replace buffer content
    readIndex = 0;                          // Reset read position
}

} // namespace DadPersistentStorage

//***End of file**************************************************************
