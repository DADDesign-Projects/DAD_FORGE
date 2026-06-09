#pragma once
//****************************************************************************
// File: cPaletteBuilder.h
//
// Description:
// This file defines the cPaletteBuilder class, which parses color palette
// definitions received over a USB serial connection. Each incoming line
// follows the format "ColorName=#RRGGBB", and the class updates the
// corresponding fields of a GUI color palette structure in real time.
//
// Copyright (c) 2025 Dad Design.
//****************************************************************************
#ifdef PALETTE_BUILDER
#include "main.h"
#include "cBuff.h"
#include "MainGUI.h"
#include <cstring>
#include <cctype>
#define PALETTE_BUILDER

//-----------------------------------------------------------------------------------
// Function: UsbCallback
// Description:
// External C linkage function triggered when USB serial data arrives.
// The received data is stored into a circular FIFO buffer for later parsing
// by cPaletteBuilder.
//-----------------------------------------------------------------------------------
extern "C" void UsbCallback(uint8_t* buf, uint32_t* len);

namespace DadUtilities {

//***********************************************************************************
// Class: cPaletteBuilder
// Description:
// Handles incremental parsing of incoming color data received as ASCII text.
// Supports partial data reads and reconstructs complete color definitions
// using a simple deterministic state machine. When a valid color definition
// is found, the corresponding field in the palette is updated immediately.
//***********************************************************************************
class cPaletteBuilder {
public:
    //-----------------------------------------------------------------------------------
    // Enum: State
    // Description:
    // Defines the internal state of the parser while decoding input text.
    //-----------------------------------------------------------------------------------
    enum class State {
        WAITING_NAME,   // Waiting for the first valid character of a color name
        READING_NAME,   // Reading color name until '=' is reached
        WAITING_HASH,   // Waiting for '#' that precedes color hex code
        READING_COLOR   // Reading 6-character hex sequence (RRGGBB)
    };

    //-----------------------------------------------------------------------------------
    // Constructor: cPaletteBuilder
    // Description:
    // Initializes the parser and resets all buffers and state flags.
    //-----------------------------------------------------------------------------------
    cPaletteBuilder()
        : m_State(State::WAITING_NAME)
    {
        Clear();
        m_ChangedPalette = false;
    }

    //-----------------------------------------------------------------------------------
    // Function: InitPalette
    // Description:
    // Initializes the local palette using a given reference palette structure.
    //-----------------------------------------------------------------------------------
    void InitPalette(const DadGUI::sColorPalette* pPalette) {
        m_Palette = *pPalette;
    }

    //-----------------------------------------------------------------------------------
    // Function: ParseBuffer
    // Description:
    // Processes all characters currently in the global FIFO buffer. Each valid
    // color definition found updates one entry in the internal palette.
    // Returns the number of successfully parsed color definitions.
    //-----------------------------------------------------------------------------------
    uint16_t ParseBuffer();

    //-----------------------------------------------------------------------------------
    // Function: ProcessByte
    // Description:
    // Feeds a single byte into the state machine. Each call updates the
    // parser’s internal state and may complete a color definition.
    // Returns true if a complete color entry was parsed and stored.
    //-----------------------------------------------------------------------------------
    bool ProcessByte(uint8_t byte);

    //-----------------------------------------------------------------------------------
    // Function: Clear
    // Description:
    // Clears all internal buffers and resets the parser to its initial state.
    //-----------------------------------------------------------------------------------
    void Clear();

    //-----------------------------------------------------------------------------------
    // Function: Reset
    // Description:
    // Resets parsing state variables without modifying stored palette data.
    //-----------------------------------------------------------------------------------
    void Reset();

    //-----------------------------------------------------------------------------------
    // Function: HexToValue
    // Description:
    // Converts a single hexadecimal character into its 4-bit integer value.
    // Returns 0 for invalid characters (safe default).
    //-----------------------------------------------------------------------------------
    uint8_t HexToValue(char hex);

    //-----------------------------------------------------------------------------------
    // Function: ParseHexColor
    // Description:
    // Converts a 6-character hex color string (RRGGBB) into a DadGFX::sColor
    // with full alpha (0xFF). Called once a valid color string is read.
    //-----------------------------------------------------------------------------------
    DadGFX::sColor ParseHexColor();

    //-----------------------------------------------------------------------------------
    // Function: StoreColor
    // Description:
    // Matches the parsed color name against known palette field names and
    // assigns the parsed color accordingly. Returns false if the name is
    // unknown (no assignment performed).
    //-----------------------------------------------------------------------------------
    bool StoreColor();

    //-----------------------------------------------------------------------------------
    // Function: getPalette
    // Description:
    // Provides direct access to the currently built palette.
    //-----------------------------------------------------------------------------------
    DadGUI::sColorPalette* getPalette() {
        return &m_Palette;
    }

    //-----------------------------------------------------------------------------------
    // Function: IsChangedPalette
    // Description:
    // Returns true if at least one color has changed since the last query.
    // The internal "changed" flag is automatically cleared afterward.
    //-----------------------------------------------------------------------------------
    bool IsChangedPalette() {
        bool ChangedPalette = m_ChangedPalette;
        m_ChangedPalette = false;
        return ChangedPalette;
    }

private:
    //***********************************************************************************
    // Internal Data
    //***********************************************************************************

    static constexpr uint8_t MAX_NAME_LEN = 32;   // Maximum allowed name length

    DadGUI::sColorPalette m_Palette;              // Local working color palette
    State                 m_State;                // Current parser state
    char                  m_NameBuffer[MAX_NAME_LEN]; // Temporary name buffer
    uint8_t               m_NameLen;              // Current name length
    char                  m_ColorBuffer[6];       // Holds hex color digits (RRGGBB)
    uint8_t               m_ColorLen;             // Current number of digits read
    bool                  m_ChangedPalette;       // True if palette was modified
};

} // namespace DadUtilities

// Global instance used by the system to update GUI palette
extern DadUtilities::cPaletteBuilder __cPaletteBuilder;
#endif

//***End of file**************************************************************
