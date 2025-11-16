//==================================================================================
//==================================================================================
// File: cFlasherStorage.cpp
// Description: Implementation for QSPI flash memory file system operations
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "HardwareAndCo.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Class cFlasherStorage
// This class manages a simple file system in QSPI flash memory
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Retrieves pointer to file data in flash memory
// ---------------------------------------------------------------------------------
uint8_t* cFlasherStorage::GetFilePtr(const char *pFileName) const
{
    // Search through directory entries for matching filename
    for(int8_t Index = 0; Index < DIR_FILE_COUNT; Index++)
    {
        if(0 == strcmp(Dir[Index].Name, pFileName))
        {
            return (uint8_t*) Dir[Index].DataAddress;  // Return pointer to file data
        }
    }
    return nullptr;  // File not found
}

// ---------------------------------------------------------------------------------
// Gets the size of a file in bytes
// ---------------------------------------------------------------------------------
uint32_t cFlasherStorage::GetFileSize(const char* pFileName) const
{
    // Iterate through directory to find file
    for(int8_t Index = 0; Index < DIR_FILE_COUNT; Index++)
    {
        if(0 == strcmp(Dir[Index].Name, pFileName))
        {
            return Dir[Index].Size;  // Return file size
        }
    }
    return 0;  // File not found
}

// ---------------------------------------------------------------------------------
// Retrieves image information from a file stored in directory structure
// ---------------------------------------------------------------------------------
bool cFlasherStorage::GetImgInformation(const char* pFileName, uint8_t* &ImgPtr, uint8_t &NbFrame, uint16_t &Width, uint16_t &Height)
{
    // Input parameter validation
    if(!pFileName)
    {
        return false;
    }

    // Search through directory entries
    for(int8_t Index = 0; Index < DIR_FILE_COUNT; Index++)
    {
        if(0 == strcmp(Dir[Index].Name, pFileName))
        {
            // Get image data pointer and size
            ImgPtr = (uint8_t*) Dir[Index].DataAddress;
            uint32_t Size = Dir[Index].Size;

            // Check minimum size to prevent buffer overflow
            if(Size < 16)
            {
                return false;
            }

            // Point to the last 16 bytes where magic signature should be
            uint8_t *pMagic = ImgPtr + Size - 16;

            // Check for "IMAG" magic signature
            if((pMagic[0] == 'I') && (pMagic[1] == 'M') && (pMagic[2] == 'A') && (pMagic[3] == 'G'))
            {
                // Cast to uint32_t pointer to read metadata
                uint32_t *pData = (uint32_t*) &pMagic[4];

                // Read metadata values
                NbFrame = (uint8_t) pData[0];
                Width = (uint16_t) pData[1];
                Height = (uint16_t) pData[2];

                return true;
            }

            // Magic signature not found - invalid image format
            return false;
        }
    }

    // File not found in directory
    return false;
}

} // namespace DadPersistentStorage

//***End of file**************************************************************
