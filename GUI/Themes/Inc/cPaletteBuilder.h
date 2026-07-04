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
// Copyright (c) 2025-2026 DadDesign-Projects.
//****************************************************************************
#include "main.h"
#include "cBuff.h"
#include "GUI_Defines.h"
#include <cstring>
#include <cctype>

namespace DadGUI {
// =============================================================================
// Color Palette Definition
// =============================================================================

// Structure defining a complete color palette for the GUI
struct sColorPalette {
    // Menu colors
    DadGFX::sColor MenuBack;        // Menu background color
    DadGFX::sColor MenuText;        // Menu text color
    DadGFX::sColor MenuActiveText;  // Active menu text color
    DadGFX::sColor MenuActive;      // Active menu item color
    DadGFX::sColor MenuArrow;       // Menu arrow color

    // Parameter area colors
    DadGFX::sColor ParameterBack;   // Parameter background color
    DadGFX::sColor ParameterName;   // Parameter name color
    DadGFX::sColor ParameterNum;    // Parameter number color
    DadGFX::sColor ParameterValue;  // Parameter value color
    DadGFX::sColor ParameterLines;  // Parameter lines color
    DadGFX::sColor ParameterCursor; // Parameter cursor color

    // Parameter info colors
    DadGFX::sColor ParamInfoBack;   // Parameter info background
    DadGFX::sColor ParamInfoName;   // Parameter info name color
    DadGFX::sColor ParamInfoValue;  // Parameter info value color

    // Main info colors
    DadGFX::sColor MainInfoBack;    // Main info background
    DadGFX::sColor MainInfoMem;     // Memory indicator color
    DadGFX::sColor MainInfoDirty;   // Dirty state indicator color
    DadGFX::sColor MainInfoState;   // System state color

    // Memory View colors
    DadGFX::sColor MemViewBack;     // Memory view background
    DadGFX::sColor MemViewText;     // Memory view text color
    DadGFX::sColor MemViewLine;     // Memory view line color
    DadGFX::sColor MemViewActive;   // Active memory item color

    // VuMeter View colors
    DadGFX::sColor VuMeterBack;     // VU meter background
    DadGFX::sColor VuMeterText;     // VU meter text color
    DadGFX::sColor VuMeterLine;     // VU meter line color
    DadGFX::sColor VuMeterCursor;   // VU meter cursor color
    DadGFX::sColor VuMeterPeak;     // VU meter peak indicator color
    DadGFX::sColor VuMeterClip;     // VU meter clip indicator color

    // Splash screen colors
    DadGFX::sColor SplatchBack;     // Splash screen background
    DadGFX::sColor SplatchText;     // Splash screen text color
};


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
    cPaletteBuilder() : m_State(State::WAITING_NAME) {
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

} // namespace DadGUI


//***End of file**************************************************************
