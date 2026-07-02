//==================================================================================
//==================================================================================
// File: cMidi.h
// Description: MIDI interface management
//
// Copyright (c) 2025 Dad Design. All rights reserved.
//==================================================================================
//==================================================================================

#pragma once

// =============================================================================
// Includes
// =============================================================================

#include "main.h"
#include "GUI_Event.h"
#include <vector>

// =============================================================================
// Constants and Definitions
// =============================================================================

#define MIDI_BUFFER_SIZE   128   // Size of the MIDI ring buffer
#define MIDI_USB_FIFO_SIZE 40    // Size of the MIDI USB FIFO buffer

#define MULTI_CHANNEL 0xFF     // Special value to listen on all MIDI channels

// =============================================================================
// MIDI USB Code Index Numbers (CIN) definitions
// =============================================================================

#define MIDI_CIN_MISC               0x00  // Miscellaneous function codes
#define MIDI_CIN_CABLE_EVENT        0x01  // Cable events
#define MIDI_CIN_2BYTE_SYS_COMMON   0x02  // System common messages (2 bytes)
#define MIDI_CIN_3BYTE_SYS_COMMON   0x03  // System common messages (3 bytes)
#define MIDI_CIN_SYSEX_START        0x04  // System exclusive start or continue
#define MIDI_CIN_SYSEX_END_1BYTE    0x05  // System exclusive end with 1 byte
#define MIDI_CIN_SYSEX_END_2BYTE    0x06  // System exclusive end with 2 bytes
#define MIDI_CIN_SYSEX_END_3BYTE    0x07  // System exclusive end with 3 bytes
#define MIDI_CIN_NOTE_OFF           0x08  // Note Off message
#define MIDI_CIN_NOTE_ON            0x09  // Note On message
#define MIDI_CIN_POLY_KEYPRESS      0x0A  // Polyphonic Key Pressure
#define MIDI_CIN_CONTROL_CHANGE     0x0B  // Control Change message
#define MIDI_CIN_PROGRAM_CHANGE     0x0C  // Program Change message
#define MIDI_CIN_CHANNEL_PRESSURE   0x0D  // Channel Pressure message
#define MIDI_CIN_PITCH_BEND         0x0E  // Pitch Bend Change message
#define MIDI_CIN_SINGLE_BYTE        0x0F  // Single byte message
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


//**********************************************************************************
// UsbMidiCallback
// Callback function for handling incoming MIDI events originating from the USB bus.
//**********************************************************************************

extern "C" {
void UsbMidiCallback(uint8_t code, uint8_t channel, uint8_t data1, uint8_t data2);
}

namespace DadDrivers {
// =============================================================================
// MIDI message reception from the USB bus.
// =============================================================================

//**********************************************************************************
// MIDI event structure
//**********************************************************************************
typedef struct {
    uint8_t code;  			// Event type
    uint8_t channel;
    uint8_t data1;
    uint8_t data2;
} stMidiEvent_t;

//**********************************************************************************
// class cMidiFifo
// FIFO buffer management for USB MIDI messages.
//**********************************************************************************
class cMidiFifo{
public:
	cMidiFifo()=default;
	~cMidiFifo()=default;

	//**********************************************************************************
	// MidiFifoPush
	// Add MIDI event to FIFO buffer (thread-safe)
	//**********************************************************************************
	bool Push(const stMidiEvent_t* event);

	//**********************************************************************************
	// Pull
	// get and Remove MIDI event from FIFO buffer
	//**********************************************************************************
	bool Pull(stMidiEvent_t* event);

protected:

	// FIFO buffer and management variables
	stMidiEvent_t 	m_midiFifoBuffer[MIDI_USB_FIFO_SIZE];  	// FIFO buffer
	uint16_t 		m_midiFifoHead = 0;                		// Head index
	uint16_t 		m_midiFifoTail = 0;                		// Tail index
	uint16_t 		m_midiFifoCount = 0;               		// Number of events in FIFO
};


//**********************************************************************************
// class cMidi
// MIDI message parser and event handler with callback registration
//**********************************************************************************
class cMidi : public DadGUI::iGUI_EventListener{
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
    void on_GUI_FastUpdate() override;

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
