//****************************************************************************
// File: cPaletteBuilder.cpp
//
// Description:
// Implements the cPaletteBuilder class, which parses ASCII-formatted
// color definitions from USB serial input and updates the corresponding
// fields in a GUI color palette structure.
//
// This parser is lightweight and incremental: data can arrive byte by byte,
// and the parser maintains internal state across calls.
//
// Copyright (c) 2025 Dad Design.
//****************************************************************************
#include "main.h"
#ifndef PALETTE_BUILDER
extern "C" void UsbCallback(uint8_t* buf, uint32_t* len){
}
#else
#include "cPaletteBuilder.h"

#define FIFO_SIZE 2000

//***********************************************************************************
// Global FIFO for USB serial input data
// Stores all received bytes until processed by the parser.
//***********************************************************************************
DadUtilities::cBuff __DataBuff(FIFO_SIZE);

//-----------------------------------------------------------------------------------
// Function: UsbCallback
// Description:
// Called by the USB driver when new data arrives over the virtual COM port.
// The function pushes each received byte into the global FIFO buffer, making
// it available for the palette parser during the next update cycle.
//-----------------------------------------------------------------------------------
void UsbCallback(uint8_t* buf, uint32_t* len) {
    uint8_t* pBuff = buf;
    for (uint32_t Index = *len; Index != 0; Index--) {
        __DataBuff.addData(*pBuff++); // Push byte into FIFO
    }
}

// Global parser instance accessible project-wide
DadUtilities::cPaletteBuilder __cPaletteBuilder;

namespace DadUtilities {

//-----------------------------------------------------------------------------------
// Function: Init
// Description:
// Placeholder for potential initialization routines.
// Currently unused but maintained for API consistency.
//-----------------------------------------------------------------------------------
void Init() {
}

//-----------------------------------------------------------------------------------
// Function: ParseBuffer
// Description:
// Processes all bytes currently present in the FIFO buffer. Each valid
// definition of the form "Name=#RRGGBB" triggers an update in the palette.
// The buffer is cleared afterward to avoid double-processing.
//-----------------------------------------------------------------------------------
uint16_t cPaletteBuilder::ParseBuffer() {
    uint16_t parsedCount = 0;
    uint8_t* data = __DataBuff.getBuffPtr();
    uint16_t nbData = __DataBuff.getNbData();

    // Sequentially parse all bytes in the buffer
    for (uint16_t i = 0; i < nbData; i++) {
        if (ProcessByte(data[i])) {
            parsedCount++;
        }
    }

    __DataBuff.Clear(); // Prevent reprocessing old data
    return parsedCount;
}

//-----------------------------------------------------------------------------------
// Function: ProcessByte
// Description:
// State machine that interprets one character at a time. It reconstructs
// valid name-value pairs of the format "Name=#RRGGBB" and triggers palette
// updates when a full entry is received.
//-----------------------------------------------------------------------------------
bool cPaletteBuilder::ProcessByte(uint8_t byte) {
    char c = static_cast<char>(byte);

    switch (m_State) {
        case State::WAITING_NAME:
            // Start only if character is alphabetic or '_'
            if (isalpha(c) || c == '_') {
                m_NameBuffer[0] = c;
                m_NameLen = 1;
                m_State = State::READING_NAME;
            }
            break;

        case State::READING_NAME:
            // '=' marks the end of the name
            if (c == '=') {
                m_NameBuffer[m_NameLen] = '\0';
                m_State = State::WAITING_HASH;
            }
            // Accept alphanumeric or '_' characters
            else if ((isalnum(c) || c == '_') && m_NameLen < MAX_NAME_LEN - 1) {
                m_NameBuffer[m_NameLen++] = c;
            }
            // Invalid character → reset parser
            else if (!isalnum(c) && c != '_') {
                Reset();
            }
            break;

        case State::WAITING_HASH:
            // Expecting '#' to start color sequence
            if (c == '#') {
                m_ColorLen = 0;
                m_State = State::READING_COLOR;
            }
            // Any non-space invalidates the entry
            else if (!isspace(c)) {
                Reset();
            }
            break;

        case State::READING_COLOR:
            // Accept hexadecimal digits only
            if (isxdigit(c)) {
                if (m_ColorLen < 6) {
                    m_ColorBuffer[m_ColorLen++] = c;

                    // When 6 hex digits read, attempt color assignment
                    if (m_ColorLen == 6) {
                        if (StoreColor()) {
                            m_ChangedPalette = true;
                            Reset();
                            return true; // Completed successfully
                        }
                        Reset(); // Invalid color name → reset
                    }
                }
            }
            // Ignore line breaks and spaces (incomplete entry)
            else if (isspace(c) || c == '\n' || c == '\r') {
                Reset();
            }
            // Any other character → abort parsing
            else {
                Reset();
            }
            break;
    }

    return false;
}

//-----------------------------------------------------------------------------------
// Function: Clear
// Description:
// Clears temporary buffers and resets parsing to the initial state.
//-----------------------------------------------------------------------------------
void cPaletteBuilder::Clear() {
    Reset();
}

//-----------------------------------------------------------------------------------
// Function: Reset
// Description:
// Returns the parser to the WAITING_NAME state and clears name/color lengths.
// Used both for initialization and error recovery.
//-----------------------------------------------------------------------------------
void cPaletteBuilder::Reset() {
    m_State = State::WAITING_NAME;
    m_NameLen = 0;
    m_ColorLen = 0;
}

//-----------------------------------------------------------------------------------
// Function: HexToValue
// Description:
// Converts a single hexadecimal character into its numeric 4-bit equivalent.
// Returns 0 if the character is outside the valid hex range.
//-----------------------------------------------------------------------------------
uint8_t cPaletteBuilder::HexToValue(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
    if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
    return 0;
}

//-----------------------------------------------------------------------------------
// Function: ParseHexColor
// Description:
// Builds a DadGFX::sColor from the current 6-character color buffer.
// Defaults alpha to 255 (fully opaque).
//-----------------------------------------------------------------------------------
DadGFX::sColor cPaletteBuilder::ParseHexColor() {
    DadGFX::sColor color;
    color.m_A = 0xFF; // Full opacity
    color.m_R = (HexToValue(m_ColorBuffer[0]) << 4) | HexToValue(m_ColorBuffer[1]);
    color.m_G = (HexToValue(m_ColorBuffer[2]) << 4) | HexToValue(m_ColorBuffer[3]);
    color.m_B = (HexToValue(m_ColorBuffer[4]) << 4) | HexToValue(m_ColorBuffer[5]);
    return color;
}

//-----------------------------------------------------------------------------------
// Function: StoreColor
// Description:
// Compares the parsed name to known palette field identifiers. If matched,
// assigns the decoded color to the appropriate member of m_Palette.
// Returns true if a valid name was found, false otherwise.
//
// Non-trivial note:
// This method performs a linear string comparison for all possible names,
// trading simplicity for reliability and low flash usage. Future optimizations
// could use a hash table or lookup map if more color fields are added.
//-----------------------------------------------------------------------------------
bool cPaletteBuilder::StoreColor() {
    DadGFX::sColor color = ParseHexColor();

    if (strcmp(m_NameBuffer, "MenuBack") == 0)          { m_Palette.MenuBack = color; }
    else if (strcmp(m_NameBuffer, "MenuText") == 0)     { m_Palette.MenuText = color; }
    else if (strcmp(m_NameBuffer, "MenuActiveText") == 0) { m_Palette.MenuActiveText = color; }
    else if (strcmp(m_NameBuffer, "MenuActive") == 0)   { m_Palette.MenuActive = color; }
    else if (strcmp(m_NameBuffer, "MenuArrow") == 0)    { m_Palette.MenuArrow = color; }

    else if (strcmp(m_NameBuffer, "ParameterBack") == 0) { m_Palette.ParameterBack = color; }
    else if (strcmp(m_NameBuffer, "ParameterName") == 0) { m_Palette.ParameterName = color; }
    else if (strcmp(m_NameBuffer, "ParameterNum") == 0)  { m_Palette.ParameterNum = color; }
    else if (strcmp(m_NameBuffer, "ParameterValue") == 0) { m_Palette.ParameterValue = color; }
    else if (strcmp(m_NameBuffer, "ParameterLines") == 0) { m_Palette.ParameterLines = color; }
    else if (strcmp(m_NameBuffer, "ParameterCursor") == 0) { m_Palette.ParameterCursor = color; }

    else if (strcmp(m_NameBuffer, "ParamInfoBack") == 0)  { m_Palette.ParamInfoBack = color; }
    else if (strcmp(m_NameBuffer, "ParamInfoName") == 0)  { m_Palette.ParamInfoName = color; }
    else if (strcmp(m_NameBuffer, "ParamInfoValue") == 0) { m_Palette.ParamInfoValue = color; }

    else if (strcmp(m_NameBuffer, "MainInfoBack") == 0)  { m_Palette.MainInfoBack = color; }
    else if (strcmp(m_NameBuffer, "MainInfoMem") == 0)   { m_Palette.MainInfoMem = color; }
    else if (strcmp(m_NameBuffer, "MainInfoDirty") == 0) { m_Palette.MainInfoDirty = color; }
    else if (strcmp(m_NameBuffer, "MainInfoState") == 0) { m_Palette.MainInfoState = color; }

    else if (strcmp(m_NameBuffer, "MemViewBack") == 0)   { m_Palette.MemViewBack = color; }
    else if (strcmp(m_NameBuffer, "MemViewText") == 0)   { m_Palette.MemViewText = color; }
    else if (strcmp(m_NameBuffer, "MemViewLine") == 0)   { m_Palette.MemViewLine = color; }
    else if (strcmp(m_NameBuffer, "MemViewActive") == 0) { m_Palette.MemViewActive = color; }

    else if (strcmp(m_NameBuffer, "VuMeterBack") == 0)   { m_Palette.VuMeterBack = color; }
    else if (strcmp(m_NameBuffer, "VuMeterText") == 0)   { m_Palette.VuMeterText = color; }
    else if (strcmp(m_NameBuffer, "VuMeterLine") == 0)   { m_Palette.VuMeterLine = color; }
    else if (strcmp(m_NameBuffer, "VuMeterCursor") == 0) { m_Palette.VuMeterCursor = color; }
    else if (strcmp(m_NameBuffer, "VuMeterPeak") == 0)   { m_Palette.VuMeterPeak = color; }
    else if (strcmp(m_NameBuffer, "VuMeterClip") == 0)   { m_Palette.VuMeterClip = color; }

    else if (strcmp(m_NameBuffer, "SplatchBack") == 0)   { m_Palette.SplatchBack = color; }
    else if (strcmp(m_NameBuffer, "SplatchText") == 0)   { m_Palette.SplatchText = color; }

    else {
        return false; // Unknown color name
    }

    return true;
}

} // namespace DadUtilities
#endif
//***End of file**************************************************************
