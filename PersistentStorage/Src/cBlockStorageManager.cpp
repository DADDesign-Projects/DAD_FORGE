//==================================================================================
//==================================================================================
// File: cBlockStorageManager.cpp
// Description: Flash Memory Persistent Storage Manager
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cBlockStorageManager.h"
#include "ID.h"
#include <cstring>
#pragma GCC optimize ("O0")

extern RNG_HandleTypeDef hrng;

namespace DadPersistentStorage {

//**********************************************************************************
// Constants
//**********************************************************************************

//**********************************************************************************
// Main build information structure
//**********************************************************************************

struct sMainBlock {
    uint32_t MaGicBuild;
    uint32_t NumBuild;
};

constexpr uint32_t kIDMain     = BUILD_ID('B', 'S', 'M', 'A');  // Identifier for MainBlock of cBlockStorageManager
constexpr uint32_t kMaGicBuild = BUILD_ID('M', 'A', 'B', 'a');  // Magic number for build identifier

//**********************************************************************************
// class cBlockStorageManager
//**********************************************************************************

// Magic number used to mark valid blocks
// Pattern 0xDADDBA55 chosen for its distinctive bit pattern
constexpr uint32_t HEADER_MAGIC = 0xDADDBA55;

// Value indicating an invalid or erased block
// In flash memory, erased state is all bits set to 1
constexpr uint32_t INVALID_MARKER = 0xFFFFFFFF;

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initializes the persistent storage system
bool cBlockStorageManager::Init(uint32_t NumBuild) {
    sMainBlock MainBlock;
    uint32_t   ReadSize;

    // Generate random starting index for wear leveling
    uint32_t randomValue;
    HAL_RNG_GenerateRandomNumber(&hrng, &randomValue);
    constexpr uint32_t totalBlocks = NUM_BLOCKS;
    m_LastFreeIndex = randomValue % totalBlocks;

    // Load main block and verify integrity
    Load(kIDMain, &MainBlock, sizeof(MainBlock), ReadSize);
    if ((ReadSize != sizeof(MainBlock)) || (MainBlock.MaGicBuild != kMaGicBuild) || (MainBlock.NumBuild != NumBuild)) {
        return true;  // Initialization required
    }
    return false;     // Already initialized
}

// -----------------------------------------------------------------------------
// Initializes the memory by erasing all blocks and setting up main structure
void cBlockStorageManager::InitializeMemory(uint32_t NumBuild) {
    // Erase all blocks in storage area
    InitializeBlock();

    // Initialize main block with build information
    sMainBlock MainBlock;
    MainBlock.MaGicBuild = kMaGicBuild;
    MainBlock.NumBuild   = NumBuild;
    Save(kIDMain, &MainBlock, sizeof(MainBlock));
}

// -----------------------------------------------------------------------------
// Saves data to flash memory by splitting it into blocks if necessary
bool cBlockStorageManager::Save(uint32_t saveNumber, const void* pDataSource, uint32_t Size) {
    // Delete existing save with same number
    Delete(saveNumber);
    
    // Find first available block for data storage
    sSaveBlock* pCurrentBlock = findFreeBlock(nullptr);
    
    // Initialize save block header
    sSaveBlock SaveBlock;
    SaveBlock.m_saveNumber = saveNumber;
    SaveBlock.m_dataSize   = Size;
    SaveBlock.m_isValid    = HEADER_MAGIC;
    
    // Track remaining data to be written
    uint32_t remainingSize = Size;
    const uint8_t* pData = static_cast<const uint8_t*>(pDataSource);
    
    // Write data across multiple blocks if needed
    while (pCurrentBlock != nullptr) {
        // Calculate data size for current block
        uint32_t blockDataSize;
        if (remainingSize > DATA_SIZE) {
            blockDataSize = DATA_SIZE;
            SaveBlock.m_pNextBlock = findFreeBlock(pCurrentBlock);  // Link to next block
        } else {
            blockDataSize = remainingSize;
            SaveBlock.m_pNextBlock = nullptr;  // Last block in chain
        }

        // Copy data to save block structure
        memcpy(&SaveBlock.m_Data[0], pData, blockDataSize);
        
        // Write complete block to flash memory
        __Flash.Write(reinterpret_cast<uint8_t*>(&SaveBlock), reinterpret_cast<uint32_t>(pCurrentBlock), BLOCK_SIZE);
                    
        // Update pointers and counters
        pData += blockDataSize;
        remainingSize -= blockDataSize;
        pCurrentBlock = SaveBlock.m_pNextBlock;
    }

    // Verify all data was written successfully
    if (remainingSize != 0) {
        Delete(saveNumber);  // Clean up partial save
        return false;        // Not enough space
    } else {
        return true;         // Save successful
    }
}

// -----------------------------------------------------------------------------
// Loads data from flash memory using save number as identifier
void cBlockStorageManager::Load(uint32_t saveNumber, void* pData, uint32_t DataSize, uint32_t& Size) {
    Size = 0;                                           // Initialize output size
    uint8_t* pBuffer = static_cast<uint8_t*>(pData);    // Cast buffer to byte pointer
    
    // Find first block with matching save number
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);
    if ((pSaveBlock == nullptr) || (pSaveBlock->m_dataSize > DataSize)) {
        return;  // Save not found or buffer too small
    }

    uint32_t remainingSize = pSaveBlock->m_dataSize;    // Track remaining data to read

    // Read data from all linked blocks
    while (pSaveBlock != nullptr) {
        // Calculate data size to read from current block
        uint32_t blockDataSize = (remainingSize > DATA_SIZE) ? DATA_SIZE : remainingSize;

        // Copy data from flash to buffer
        memcpy(pBuffer, &(pSaveBlock->m_Data[0]), blockDataSize);
        
        // Update pointers and counters
        pBuffer += blockDataSize;
        Size += blockDataSize;
        remainingSize -= blockDataSize;
        pSaveBlock = pSaveBlock->m_pNextBlock;  // Move to next block
    }
}

// -----------------------------------------------------------------------------
// Deletes a save from flash memory by erasing all blocks in its chain
void cBlockStorageManager::Delete(uint32_t saveNumber) {
    // Find first block of the save to delete
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);

    // Erase all blocks in the chain
    while (pSaveBlock != nullptr) {
        sSaveBlock* pNextBlock = pSaveBlock->m_pNextBlock;  // Store next pointer before erase

        __Flash.EraseBlock4K(reinterpret_cast<uint32_t>(pSaveBlock));
        HAL_Delay(10);  // Ensure erase completion
        
        pSaveBlock = pNextBlock;  // Move to next block in chain
    }
}

// -----------------------------------------------------------------------------
// Returns the size of data for a given save number
uint32_t cBlockStorageManager::getSize(uint32_t saveNumber) {
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);
    return (pSaveBlock != nullptr) ? pSaveBlock->m_dataSize : 0;  // Return size or 0 if not found
}

// =============================================================================
// Private Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Finds a free block in the storage area with round-robin allocation
sSaveBlock* cBlockStorageManager::findFreeBlock(sSaveBlock* blockExcl) {
    constexpr uint32_t totalBlocks = NUM_BLOCKS;
    uint32_t index = m_LastFreeIndex % totalBlocks;  // Start from last used index

    // Search through all blocks
    for (uint32_t i = 0; i < totalBlocks; ++i) {
        uint32_t currentIndex = (index + i) % totalBlocks;
        sSaveBlock* pBlock = &m_pTabSaveBlock[currentIndex];

        // Skip excluded block
        if (pBlock == blockExcl)
            continue;

        // Check if block is free (erased state)
        if (pBlock->m_isValid == INVALID_MARKER) {
            m_LastFreeIndex = (currentIndex + 1) % totalBlocks;  // Update for next allocation
            return pBlock;  // Return free block
        }
    }

    return nullptr;  // No free blocks available
}

// -----------------------------------------------------------------------------
// Initializes blocks by erasing flash memory in optimal block sizes
void cBlockStorageManager::InitializeBlock() {
    constexpr uint32_t BLOCK_64K_SIZE = 16 * BLOCK_SIZE;  // 64K in terms of BLOCK_SIZE units
    constexpr uint32_t BLOCK_32K_SIZE = 8 * BLOCK_SIZE;   // 32K in terms of BLOCK_SIZE units

    uint8_t* pCurrentBlock = reinterpret_cast<uint8_t*>(m_pTabSaveBlock);
    uint32_t remainingSize = NUM_BLOCKS * BLOCK_SIZE;
    uint32_t currentAddress;

    // Erase memory in largest possible blocks for efficiency
    while (remainingSize > 0) {
        currentAddress = reinterpret_cast<uint32_t>(pCurrentBlock);

        // Use largest possible erase block size for better performance
        if (remainingSize >= BLOCK_64K_SIZE) {
            __Flash.EraseBlock64K(currentAddress);
            pCurrentBlock += BLOCK_64K_SIZE;
            remainingSize -= BLOCK_64K_SIZE;
        } else if (remainingSize >= BLOCK_32K_SIZE) {
            __Flash.EraseBlock32K(currentAddress);
            pCurrentBlock += BLOCK_32K_SIZE;
            remainingSize -= BLOCK_32K_SIZE;
        } else {
            // Default to 4K sector erase for small remaining areas
            __Flash.EraseBlock4K(currentAddress);
            pCurrentBlock += BLOCK_SIZE;
            remainingSize -= BLOCK_SIZE;
        }
    }
}

// -----------------------------------------------------------------------------
// Finds the first block matching a given saveNumber
sSaveBlock* cBlockStorageManager::FindFirstBlock(uint32_t saveNumber) {
    sSaveBlock* pSaveBlock = m_pTabSaveBlock;  // Start from first block

    // Search through all blocks
    for (uint32_t i = 0; i < NUM_BLOCKS; i++) {
        // Check if block is valid and matches save number
        if ((pSaveBlock->m_isValid == HEADER_MAGIC) && (pSaveBlock->m_saveNumber == saveNumber)) {
            return pSaveBlock;  // Found matching block
        }
        pSaveBlock++;  // Move to next block
    }
    return nullptr;  // No matching blocks found
}

} // namespace DadPersistentStorage

//***End of file**************************************************************
