//==================================================================================
//==================================================================================
// File: cMidi.h
// Description: MIDI interface management with callback registration system
//
// Copyright (c) 2025 Dad Design. All rights reserved.
//==================================================================================
//==================================================================================

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include "main.h"
#include <vector>


// =============================================================================
// Constants and Definitions
// =============================================================================

#define MIDI_BUFFER_SIZE 128   // Size of the MIDI ring buffer
#define MULTI_CHANNEL 0xFF     // Special value to listen on all MIDI channels

// =============================================================================
// Callback Type Definitions
// =============================================================================

using ControlChangeCallback = void (*)(uint8_t control, uint8_t value, uint32_t userData);  			// CC message callback
using ProgramChangeCallback = void (*)(uint8_t program, uint32_t userData);                 			// PC message callback
using NoteChangeCallback = void (*)(uint8_t OnOff, uint8_t note, uint8_t velocity, uint32_t userData);  // Note message callback

// =============================================================================
// Callback Entry Structures
// =============================================================================

//**********************************************************************************
// CC_CallbackEntry
// Structure to store Control Change callback information
//**********************************************************************************
struct CC_CallbackEntry {
    uint8_t control;                    // Control Change number (0-127)
    uint32_t userData;                  // User-defined data passed to callback
    ControlChangeCallback callback;     // Function to call when this CC is received
};

//**********************************************************************************
// PC_CallbackEntry
// Structure to store Program Change callback information
//**********************************************************************************
struct PC_CallbackEntry {
    uint32_t userData;                  // User-defined data passed to callback
    ProgramChangeCallback callback;     // Function to call when this PC is received
};

//**********************************************************************************
// Note_CallbackEntry
// Structure to store Note On/Off callback information
//**********************************************************************************
struct Note_CallbackEntry {
    uint32_t userData;                  // User-defined data passed to callback
    NoteChangeCallback callback;        // Function to call when Note On/Off is received
};

namespace DadDrivers {

//**********************************************************************************
// class cMidi
// MIDI message parser and event handler with callback registration
//**********************************************************************************
class cMidi {
public:
    // =========================================================================
    // Public Methods
    // =========================================================================

    // -------------------------------------------------------------------------
    // Constructor/destructor
    // -------------------------------------------------------------------------
    cMidi(){}  // Default constructor

    // -------------------------------------------------------------------------
    // Initialize the MIDI interface
    // @param phuart - Pointer to UART handle that will receive MIDI data
    // @param Channel - MIDI listening channel (default: MULTI_CHANNEL)
    // -------------------------------------------------------------------------
    void Initialize(UART_HandleTypeDef* phuart, uint8_t Channel = MULTI_CHANNEL);

    // -------------------------------------------------------------------------
    // Change MIDI channel
    // @param Channel - new MIDI listening channel
    // -------------------------------------------------------------------------
    void ChangeChannel(uint8_t Channel){
        m_Channel = Channel;  // Update MIDI channel
    }

    // -------------------------------------------------------------------------
    // Process any MIDI messages in the buffer
    // Should be called regularly from the main loop
    // -------------------------------------------------------------------------
    void ProcessBuffer();

    // -------------------------------------------------------------------------
    // Register a callback for a specific Control Change message
    // @param control - Control Change number (0-127)
    // @param userData - User-defined data to pass to callback
    // @param pCallback - Function to call when this CC is received
    // -------------------------------------------------------------------------
    void addControlChangeCallback(uint8_t control, uint32_t userData, ControlChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Remove a previously registered Control Change callback
    // @param pCallback - The callback function to remove
    // -------------------------------------------------------------------------
    void removeControlChangeCallback(ControlChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Register a callback for a specific Program Change message
    // @param userData - User-defined data to pass to callback
    // @param pCallback - Function to call when this PC is received
    // -------------------------------------------------------------------------
    void addProgramChangeCallback(uint32_t userData, ProgramChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Remove a previously registered Program Change callback
    // @param pCallback - The callback function to remove
    // -------------------------------------------------------------------------
    void removeProgramChangeCallback(ProgramChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Register a callback for Note On/Off messages on a specific channel
    // @param userData - User-defined data to pass to callback
    // @param pCallback - Function to call when Note messages are received
    // -------------------------------------------------------------------------
    void addNoteChangeCallback(uint32_t userData, NoteChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Remove a previously registered Note callback
    // @param pCallback - The callback function to remove
    // -------------------------------------------------------------------------
    void removeNoteChangeCallback(NoteChangeCallback pCallback);

    // -------------------------------------------------------------------------
    // Handle Note On MIDI messages
    // @param channel - MIDI channel (0-15)
    // @param note - MIDI note number (0-127)
    // @param velocity - Note velocity (0-127)
    // -------------------------------------------------------------------------
    void OnNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) const;

    // -------------------------------------------------------------------------
    // Handle Note Off MIDI messages
    // @param channel - MIDI channel (0-15)
    // @param note - MIDI note number (0-127)
    // @param velocity - Release velocity (0-127)
    // -------------------------------------------------------------------------
    void OnNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) const;

    // -------------------------------------------------------------------------
    // Handle Control Change MIDI messages
    // @param channel - MIDI channel (0-15)
    // @param control - Control Change number (0-127)
    // @param value - Control value (0-127)
    // -------------------------------------------------------------------------
    void OnControlChange(uint8_t channel, uint8_t control, uint8_t value) const;

    // -------------------------------------------------------------------------
    // Handle Program Change MIDI messages
    // @param channel - MIDI channel (0-15)
    // @param program - Program number (0-127)
    // -------------------------------------------------------------------------
    void OnProgramChange(uint8_t channel, uint8_t program) const;

    // -------------------------------------------------------------------------
    // Determine the number of data bytes expected for a given status byte
    // @param status - MIDI status byte
    // @return Number of data bytes (1 or 2)
    // -------------------------------------------------------------------------
    uint8_t getDataLength(uint8_t status) const;

    // -------------------------------------------------------------------------
    // Parse and dispatch a complete MIDI message
    // @param status - MIDI status byte
    // @param data - Array of data bytes
    // -------------------------------------------------------------------------
    void parseMessage(uint8_t status, uint8_t* data) const;

protected:
    // =========================================================================
    // Protected Member Variables
    // =========================================================================

    UART_HandleTypeDef*              m_phuart;           // UART interface for MIDI communication
    uint8_t                          m_Channel;          // Current MIDI channel (0-15 or MULTI_CHANNEL)
    uint8_t                          m_status;           // Current MIDI status byte
    uint8_t                          m_data[2];          // Data bytes for current message
    uint8_t                          m_dataIndex;        // Number of data bytes received so far
    std::vector<CC_CallbackEntry>    m_ccCallbacks;      // Vector of Control Change callbacks
    std::vector<PC_CallbackEntry>    m_pcCallbacks;      // Vector of Program Change callbacks
    std::vector<Note_CallbackEntry>  m_noteCallbacks;    // Vector of Note On/Off callbacks
};

} // namespace DadDrivers

//***End of file**************************************************************
