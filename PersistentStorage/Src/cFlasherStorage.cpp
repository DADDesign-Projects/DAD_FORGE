//==================================================================================
//==================================================================================
// File: cFlasherStorage.cpp
// Description: Implementation for QSPI flash memory file system operations
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#include "main.h"
#include <cstring>
#include "cFlasherStorage.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Internal helpers
//**********************************************************************************

// Returns true when the directory entry at Index holds a recognised file type.
bool cFlasherStorage::isValidEntry(uint16_t Index) const
{
    return (Index < DIR_FILE_COUNT) &&
           (Dir[Index].FileType >= FILE_TYPE_MIN) &&
           (Dir[Index].FileType <= FILE_TYPE_MAX);
}

// Returns the directory index of pFileName, or -1 if not found / invalid.
int16_t cFlasherStorage::findFileIndex(const char* pFileName) const
{
    if (!pFileName) {
        return -1;
    }
    for (int16_t i = 0; i < DIR_FILE_COUNT; ++i) {
        if (isValidEntry(i) && (0 == strcmp(Dir[i].Name, pFileName))) {
            return i;
        }
    }
    return -1;
}

//**********************************************************************************
// Directory queries
//**********************************************************************************

// Returns the file name at position FileIndex, or nullptr if the entry is invalid.
const char* cFlasherStorage::GetFileName(uint16_t FileIndex) const
{
    if (!isValidEntry(FileIndex)) {
        return nullptr;
    }
    return Dir[FileIndex].Name;
}

// Returns the file type at position FileIndex, or FILE_TYPE_INVALID.
eFileType cFlasherStorage::GetFileType(uint16_t FileIndex) const
{
    if (!isValidEntry(FileIndex)) {
        return FILE_TYPE_INVALID;
    }
    return Dir[FileIndex].FileType;
}

// Returns the file type for the file identified by pFileName,
// or FILE_TYPE_INVALID if not found.
eFileType cFlasherStorage::GetFileType(const char* pFileName) const
{
    int16_t Index = findFileIndex(pFileName);
    if (Index == -1) {
        return FILE_TYPE_INVALID;
    }
    return Dir[Index].FileType;
}

//**********************************************************************************
// Data access
//**********************************************************************************

// Returns a pointer to the file data in flash, or nullptr if not found.
uint8_t* cFlasherStorage::GetFilePtr(const char* pFileName) const
{
    int16_t Index = findFileIndex(pFileName);
    if (Index == -1) {
        return nullptr;
    }
    return reinterpret_cast<uint8_t*>(Dir[Index].DataAddress);
}

// Returns the file size in bytes, or 0 if not found.
uint32_t cFlasherStorage::GetFileSize(const char* pFileName) const
{
    int16_t Index = findFileIndex(pFileName);
    if (Index == -1) {
        return 0;
    }
    return Dir[Index].Size;
}

//**********************************************************************************
// Typed file helpers
//**********************************************************************************

// Decodes IMG metadata from the 16-byte trailing magic block ("IMAG" signature).
// Layout of the last 16 bytes:
//   [0..3]  'I','M','A','G'
//   [4..7]  NbFrame  (uint32_t)
//   [8..11] Width    (uint32_t)
//   [12..15] Height  (uint32_t)
bool cFlasherStorage::GetImgInformation(const char* pFileName,
                                        uint8_t*&   ImgPtr,
                                        uint8_t&    NbFrame,
                                        uint16_t&   Width,
                                        uint16_t&   Height)
{
    if (!pFileName) {
        return false;
    }

    int16_t Index = findFileIndex(pFileName);
    if (Index == -1) {
        return false;
    }

    uint32_t Size = Dir[Index].Size;
    if (Size < 16) {
        return false;   // Not large enough to contain the magic block
    }

    ImgPtr = reinterpret_cast<uint8_t*>(Dir[Index].DataAddress);

    // The magic block sits in the last 16 bytes of the file.
    const uint8_t* pMagic = ImgPtr + Size - 16;

    if ((pMagic[0] != 'I') || (pMagic[1] != 'M') ||
        (pMagic[2] != 'A') || (pMagic[3] != 'G')) {
        return false;   // Missing or corrupt signature
    }

    const uint32_t* pData = reinterpret_cast<const uint32_t*>(&pMagic[4]);
    NbFrame = static_cast<uint8_t> (pData[0]);
    Width   = static_cast<uint16_t>(pData[1]);
    Height  = static_cast<uint16_t>(pData[2]);

    return true;
}

// Decodes ELF region descriptors from the trailing magic block ("ELF0" signature).
// Layout immediately after the file data:
//   [0..3]  'E','L','F','0'
//   [4..7]  nbRegions  (uint32_t)
//   [8..]   array of sRegionInfo
bool cFlasherStorage::GetElfRegionsInformation(const char*   pFileName,
                                               uint8_t*&     FilePtr,
                                               sRegionInfo*& pRegions,
                                               uint32_t&     nbRegions)
{
    if (!pFileName) {
        return false;
    }

    int16_t Index = findFileIndex(pFileName);
    if (Index == -1) {
        return false;
    }

    uint32_t Size = Dir[Index].Size;
    if (Size < 8) {
        return false;   // Not large enough to contain the magic block
    }

    FilePtr = reinterpret_cast<uint8_t*>(Dir[Index].DataAddress);

    // The magic block starts right after the raw file data.
    const uint8_t* pMagic = FilePtr + Size;

    if ((pMagic[0] != 'E') || (pMagic[1] != 'L') ||
        (pMagic[2] != 'F') || (pMagic[3] != '0')) {
        return false;   // Missing or corrupt signature
    }

    nbRegions = *reinterpret_cast<const uint32_t*>(&pMagic[4]);
    pRegions  =  reinterpret_cast<sRegionInfo*>   (const_cast<uint8_t*>(&pMagic[8]));

    return true;
}

} // namespace DadPersistentStorage

//***End of file**************************************************************
