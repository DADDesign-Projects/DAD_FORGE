//==================================================================================
//==================================================================================
// File: cFlasherStorage.h
// Description: Header for QSPI flash memory file system management
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "PersistentDefine.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Class cFlasherStorage
// Manages a simple file system stored in QSPI flash memory.
//**********************************************************************************

// Maximum length for file names in the directory
#define MAX_ENTRY_NAME  40

// Maximum number of files that can be stored in the directory
#define DIR_FILE_COUNT  40

// ---------------------------------------------------------------------------------
// File type identifiers
// ---------------------------------------------------------------------------------
enum eFileType : uint32_t {
    FILE_TYPE_BIN   = 0x2851,   // Binary file
    FILE_TYPE_IMG   = 0x2852,   // Image file
    FILE_TYPE_ELF   = 0x2853,   // ELF executable file
    FILE_TYPE_MIN   = FILE_TYPE_BIN,
    FILE_TYPE_MAX   = FILE_TYPE_ELF,
    FILE_TYPE_INVALID = 0xFFFFFFFF
};

// ---------------------------------------------------------------------------------
// Directory entry – describes one stored file
// ---------------------------------------------------------------------------------
struct stFile {
    char        Name[MAX_ENTRY_NAME];   // File name (null-terminated)
    uint32_t    Size;                   // File size in bytes
    uint32_t    DataAddress;            // Absolute address of file data in flash
    eFileType   FileType;               // Type of file (BIN / IMG / ELF)
};

// ---------------------------------------------------------------------------------
// Image metadata (decoded from the trailing magic block of an IMG file)
// ---------------------------------------------------------------------------------
class cImageInfo {
public:
    const uint8_t* pLayerFrame; // Pointer to image frame buffer in flash
    uint16_t       Width;       // Image width in pixels
    uint16_t       Height;      // Image height in pixels
    uint8_t        NbFrame;     // Number of animation frames

    cImageInfo(const uint8_t* frame = nullptr,
               uint16_t w = 0, uint16_t h = 0, uint8_t frames = 1)
        : pLayerFrame(frame), Width(w), Height(h), NbFrame(frames) {}
};

// ---------------------------------------------------------------------------------
// ELF region descriptor – one loadable segment to copy/zero into RAM
// ---------------------------------------------------------------------------------
struct sRegionInfo {
    uint32_t file_offset;   // Read offset inside the ELF file
    uint8_t* dest_addr;     // Destination address in RAM
    uint8_t* source_addr;   // Source address (in flash)
    uint32_t copy_size;     // Number of bytes to copy
    uint32_t zero_size;     // Number of bytes to zero after copy (BSS)
};

// ================================================================================
// cFlasherStorage
// Provides read-only access to files stored in the QSPI flash directory.
// ================================================================================
class cFlasherStorage {
public:

    // -------------------------------------------------------------------------
    // Directory queries
    // -------------------------------------------------------------------------

    // Returns the file name at position FileIndex, or nullptr if invalid.
    const char*  GetFileName(uint16_t FileIndex) const;

    // Returns the file type at position FileIndex, or FILE_TYPE_INVALID.
    eFileType    GetFileType(uint16_t FileIndex) const;

    // Returns the file type for the file identified by pFileName,
    // or FILE_TYPE_INVALID if not found.
    eFileType    GetFileType(const char* pFileName) const;

    // -------------------------------------------------------------------------
    // Data access
    // -------------------------------------------------------------------------

    // Returns a pointer to the file data in flash, or nullptr if not found.
    uint8_t*     GetFilePtr(const char* pFileName) const;

    // Returns the file size in bytes, or 0 if not found.
    uint32_t     GetFileSize(const char* pFileName) const;

    // -------------------------------------------------------------------------
    // Typed file helpers
    // -------------------------------------------------------------------------

    // Decodes IMG metadata from the trailing magic block.
    // Returns true and fills ImgPtr / NbFrame / Width / Height on success.
    bool GetImgInformation(const char* pFileName,
                           uint8_t*& ImgPtr,
                           uint8_t&  NbFrame,
                           uint16_t& Width,
                           uint16_t& Height);

    // Decodes ELF region descriptors from the trailing magic block.
    // Returns true and fills FilePtr / pRegions / nbRegions on success.
    bool GetElfRegionsInformation(const char* pFileName,
                                  uint8_t*&    FilePtr,
                                  sRegionInfo*& pRegions,
                                  uint32_t&    nbRegions);

protected:

    // Returns the directory index of pFileName, or -1 if not found / invalid.
    int16_t findFileIndex(const char* pFileName) const;

    // Returns true if the FileType field of entry i is a known valid type.
    bool    isValidEntry(uint16_t Index) const;

    // -------------------------------------------------------------------------
    // Flash memory layout
    // -------------------------------------------------------------------------
    stFile  Dir[DIR_FILE_COUNT];                    // Directory table
    uint8_t Data[FLASHER_MEM_SIZE - sizeof(Dir)];   // Raw file data area
};

} // namespace DadPersistentStorage

//***End of file**************************************************************
