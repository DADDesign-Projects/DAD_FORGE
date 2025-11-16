//==================================================================================
//==================================================================================
// File: cBlockStorageManager.h
// Description: Flash Memory Persistent Storage Manager
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "PersistentDefine.h"

namespace DadPersistentStorage {

//**********************************************************************************
// Constants and Configuration
//**********************************************************************************

// -----------------------------------------------------------------------------
// Flash memory block and storage configuration
// Block size matches the QSPI flash page size for optimal performance
constexpr uint32_t BLOCK_SIZE = QFLAH_SECTOR_SIZE;

// -----------------------------------------------------------------------------
// Size of the save block header, consisting of:
// - 3 uint32_t (saveNumber, dataSize, isValid)
// - 1 pointer (pNextBlock)
constexpr uint32_t HEADER_SIZE  = (3 * sizeof(uint32_t)) + sizeof(void *);

// -----------------------------------------------------------------------------
// Available space for data in each block
// Calculated by subtracting header size from total block size
constexpr uint32_t DATA_SIZE = BLOCK_SIZE - HEADER_SIZE;

// -----------------------------------------------------------------------------
// Total number of available blocks for persistent storage
constexpr uint32_t NUM_BLOCKS = BLOCK_STORAGE_MEM_SIZE / BLOCK_SIZE;

//**********************************************************************************
// Structure Definitions
//**********************************************************************************

// -----------------------------------------------------------------------------
// Structure defining the layout of a save block in flash memory
struct sSaveBlock {
    uint32_t    m_saveNumber;      // Save identification number
    uint32_t    m_dataSize;        // Size of the data contained in this block
    sSaveBlock* m_pNextBlock;      // Pointer to next block (for multi-block saves)
    uint32_t    m_isValid;         // Validity marker (0xFFFFFFFF = invalid)
    uint8_t     m_Data[DATA_SIZE]; // Data storage array
};

//**********************************************************************************
// Class cBlockStorageManager
//**********************************************************************************

class cBlockStorageManager {
public:
    // =============================================================================
    // Public Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Constructor - initializes the storage manager with block array
    cBlockStorageManager(sSaveBlock *pTabSaveBlock) {
        m_pTabSaveBlock = pTabSaveBlock;
    }

    // -----------------------------------------------------------------------------
    // Initializes the persistent storage system
    bool Init(uint32_t NumBuild);

    // -----------------------------------------------------------------------------
    // Initializes the memory by erasing all blocks and setting up main structure
    void InitializeMemory(uint32_t NumBuild);

    // -----------------------------------------------------------------------------
    // Initializes blocks by erasing flash memory in optimal block sizes
    void InitializeBlock();

    // -----------------------------------------------------------------------------
    // Saves data to flash memory by splitting it into blocks if necessary
    bool Save(uint32_t saveNumber, const void* pDataSource, uint32_t Size);

    // -----------------------------------------------------------------------------
    // Loads data from flash memory using save number as identifier
    void Load(uint32_t saveNumber, void* pData, uint32_t DataSize, uint32_t& Size);

    // -----------------------------------------------------------------------------
    // Deletes a save from flash memory by erasing all blocks in its chain
    void Delete(uint32_t saveNumber);
    
    // -----------------------------------------------------------------------------
    // Gets the size of data from flash memory using save number as identifier
    uint32_t getSize(uint32_t saveNumber);

protected:
    // =============================================================================
    // Protected Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Finds a free block in the storage area with round-robin allocation
    sSaveBlock* findFreeBlock(sSaveBlock* blockExcl);

    // -----------------------------------------------------------------------------
    // Finds the first block matching a given saveNumber
    sSaveBlock* FindFirstBlock(uint32_t saveNumber);

    // =============================================================================
    // Member Variables
    // =============================================================================

    sSaveBlock* m_pTabSaveBlock;   // Persistent storage blocks array
    uint32_t    m_LastFreeIndex;   // Last index used for free block search (for round-robin allocation)
};

} // namespace DadPersistentStorage

//***End of file**************************************************************
