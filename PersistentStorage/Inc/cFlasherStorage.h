//==================================================================================
//==================================================================================
// File: cFlasherStorage.h
// Description: Header for QSPI flash memory file system management
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "PersistentDefine.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Class cFlasherStorage
// This class manages a simple file system in QSPI flash memory
//**********************************************************************************

// Maximum length for file names in the directory
#define MAX_ENTRY_NAME      40

// Maximum number of files that can be stored in the directory
#define DIR_FILE_COUNT      40

// ---------------------------------------------------------------------------------
// Directory structure for file entries
// ---------------------------------------------------------------------------------
typedef struct stFile
{
    char     Name[MAX_ENTRY_NAME];   // File name
    uint32_t Size;                   // File size in bytes
    uint32_t DataAddress;            // Address where file data is stored in flash
} Directory[DIR_FILE_COUNT];

// ---------------------------------------------------------------------------------
// Structure to hold image information
// ---------------------------------------------------------------------------------
class cImageInfo
{
public:
    const uint8_t* pLayerFrame;  // Pointer to image frame buffer
    uint16_t Width;              // Image width in pixels
    uint16_t Height;             // Image height in pixels
    uint8_t  NbFrame;            // Number of animation frames

    // Constructor with default values
    cImageInfo(const uint8_t* frame = nullptr, uint16_t w = 0, uint16_t h = 0, uint8_t frames = 1)
        : pLayerFrame(frame), Width(w), Height(h), NbFrame(frames) {}
};

// ================================================================================
// Class managing flash storage for files
// ================================================================================
class cFlasherStorage
{
public:
    // -----------------------------------------------------------------------------
    // Retrieves pointer to file data in flash memory
    // -----------------------------------------------------------------------------
    uint8_t* GetFilePtr(const char *pFileName) const;

    // -----------------------------------------------------------------------------
    // Gets the size of a file in bytes
    // -----------------------------------------------------------------------------
    uint32_t GetFileSize(const char* pFileName) const;

    // -----------------------------------------------------------------------------
    // Retrieves image information from a file stored in directory structure
    // -----------------------------------------------------------------------------
    bool GetImgInformation(const char* pFileName, uint8_t* &ImgPtr, uint8_t &NbFrame, uint16_t &Width, uint16_t &Height);

protected:
    stFile  Dir[DIR_FILE_COUNT];                         // Directory entries
    uint8_t Data[FLASHER_MEM_SIZE - sizeof(Directory)];  // Storage area for file data
};

} // namespace DadPersistentStorage

//***End of file**************************************************************
