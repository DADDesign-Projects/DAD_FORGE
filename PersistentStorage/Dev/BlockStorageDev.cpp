//====================================================================================
// cBlockStorageManager.cpp
//
// Flash Memory Persistent Storage Manager
// Handles persistent data management in QSPI flash memory.
//
// Copyright(c) 2025 Dad Design.
//====================================================================================
#include "cBlockStorageManager.h"
#include "ID.h"
#include <cstring>

namespace DadPersistentStorage {

//***********************************************************************************
// Constants
//***********************************************************************************

//***********************************************************************************
// Main build information structure
//***********************************************************************************
struct sMainBlock {
    uint32_t MaGicBuild;
    uint32_t NumBuild;
};
constexpr uint32_t kIDMain     = BUILD_ID('B', 'S', 'M', 'A');  // Identifier for MainBlock of cBlockStorageManager
constexpr uint32_t kMaGicBuild = BUILD_ID('M', 'A', 'B', 'a');  // Magic number for build identifier

//***********************************************************************************
// class cBlockStorageManager
// This class manages persistent storage in QSPI flash memory.
// It handles saving, loading, and deleting data in flash memory blocks.
// Each block contains a header and data area.
//***********************************************************************************

// Magic number used to mark valid blocks
// Pattern 0xDADDBA55 chosen for its distinctive bit pattern
constexpr uint32_t HEADER_MAGIC = 0xDADDBA55;

// Value indicating an invalid or erased block
// In flash memory, erased state is all bits set to 1
constexpr uint32_t INVALID_MARKER = 0xFFFFFFFF;

// Marqueur pour un bloc supprimé (mais pas encore effacé physiquement)
constexpr uint32_t DELETED_MARKER = 0x00000000;

// --------------------------------------------------------------------------
// Initializes the persistent storage system
bool cBlockStorageManager::Init(uint32_t NumBuild) {
    sMainBlock MainBlock;
    uint32_t   ReadSize;

    Load(kIDMain, &MainBlock, sizeof(MainBlock), ReadSize);
    if ((ReadSize != sizeof(MainBlock)) || (MainBlock.MaGicBuild != kMaGicBuild) || (MainBlock.NumBuild != NumBuild)) {
        return true;
    }

    // Initialiser l'état des secteurs
    for (uint32_t i = 0; i < m_numSectors; i++) {
        updateSectorState(i);
    }

    return false;
}

// --------------------------------------------------------------------------
// Initializes the memory
void cBlockStorageManager::InitializeMemory(uint32_t NumBuild) {
    sMainBlock MainBlock;

    InitializeBlock();
    MainBlock.MaGicBuild = kMaGicBuild;
    MainBlock.NumBuild   = NumBuild;
    Save(kIDMain, &MainBlock, sizeof(MainBlock));
}

// --------------------------------------------------------------------------
// Saves data to flash memory by splitting it into blocks if necessary.
// Each block contains a header (sSaveBlock) and data area.
// If data is larger than one block, it creates a linked list of blocks.
// @param saveNumber Unique identifier for the save
// @param pDataSource Pointer to the data to be saved
// @param Size Size of the data in bytes
// @return true if save successful, false if not enough space or error
//
bool cBlockStorageManager::Save(uint32_t saveNumber, const void* pDataSource, uint32_t Size) {
    // Delete existing save if it already exists
    Delete(saveNumber);

    // Find first available block
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
        // Vérifier si le secteur doit être effacé avant d'écrire
        uint32_t sectorIdx = getSectorIndex(pCurrentBlock);
        eraseSectorIfNeeded(sectorIdx);

        // Calculate how much data can fit in current block
        uint32_t blockDataSize;
        if (remainingSize > DATA_SIZE) {
            blockDataSize = DATA_SIZE;
            SaveBlock.m_pNextBlock = findFreeBlock(pCurrentBlock);  // Link to next block
        } else {
            blockDataSize = remainingSize;
            SaveBlock.m_pNextBlock = nullptr;  // Last block in chain
        }

        // Copy data to the save block structure
        memcpy(&SaveBlock.m_Data[0], pData, blockDataSize);

        // Write the entire block (header + data) to flash
        __Flash.Write(reinterpret_cast<uint8_t*>(&SaveBlock), reinterpret_cast<uint32_t>(pCurrentBlock), BLOCK_SIZE);

        // Mettre à jour l'état du secteur
        updateSectorState(sectorIdx);

        // Update pointers and remaining size for next iteration
        pData += blockDataSize;
        remainingSize -= blockDataSize;
        pCurrentBlock = SaveBlock.m_pNextBlock;
    }

    // Check if all data was written
    if (remainingSize != 0) {
        // Not enough space available, delete partial save
        Delete(saveNumber);
        return false;
    } else {
        return true;
    }
}

// --------------------------------------------------------------------------
// Loads data from flash memory using save number as identifier.
// Follows the linked list of blocks to reconstruct the complete data.
// @param saveNumber Identifier of the save to load
// @param pBuffer Buffer to store the loaded data
// @param DataSize size of the buffer
// @param Size Will contain the size of loaded data (0 if load error)
//

// ???Corrects a memory fault during initialization of persistent memory blocks,
// but the root cause is unclear. Possibly due to uninitialized block pointer or
// flash timing alignment.
#pragma GCC push_options
#pragma GCC optimize ("O0")

void cBlockStorageManager::Load(uint32_t saveNumber, void* pData, uint32_t DataSize, uint32_t& Size) {
    Size = 0;
    uint8_t* pBuffer = static_cast<uint8_t*>(pData);

    // Search through all blocks for matching save number
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);
    if ((pSaveBlock == nullptr) || (pSaveBlock->m_dataSize > DataSize)) {
        return;
    }

    uint32_t remainingSize = pSaveBlock->m_dataSize;

    // Read data from all linked blocks
    while (pSaveBlock != nullptr) {
        uint32_t blockDataSize = (remainingSize > DATA_SIZE) ? DATA_SIZE : remainingSize;

        // Copy data from flash to buffer (flash is memory-mapped)
        memcpy(pBuffer, &(pSaveBlock->m_Data[0]), blockDataSize);

        // Update pointers and counters
        pBuffer += blockDataSize;
        Size += blockDataSize;
        remainingSize -= blockDataSize;
        pSaveBlock = pSaveBlock->m_pNextBlock;
    }
}
#pragma GCC pop_options

// --------------------------------------------------------------------------
// Deletes a save from flash memory by marking blocks as deleted.
// Physical erase is deferred until the entire sector needs to be erased.
// @param saveNumber Identifier of the save to delete
//
void cBlockStorageManager::Delete(uint32_t saveNumber) {
    // Search through all blocks for matching save number
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);

    // Invalidate all blocks in the chain
    while (pSaveBlock != nullptr) {
        sSaveBlock* pNextBlock = pSaveBlock->m_pNextBlock;

        // Marquer le bloc comme invalide (suppression logique)
        invalidateBlock(pSaveBlock);

        // Mettre à jour l'état du secteur
        uint32_t sectorIdx = getSectorIndex(pSaveBlock);
        updateSectorState(sectorIdx);

        pSaveBlock = pNextBlock;  // Move to next block in chain
    }
}

// --------------------------------------------------------------------------
// Returns the size of data from flash memory using save number as identifier.
// @param saveNumber Identifier of the save
// @return the size of data, or 0 if saveNumber does not exist
//
uint32_t cBlockStorageManager::getSize(uint32_t saveNumber) {
    sSaveBlock* pSaveBlock = FindFirstBlock(saveNumber);
    return (pSaveBlock != nullptr) ? pSaveBlock->m_dataSize : 0;
}

// --------------------------------------------------------------------------
// Finds a free block in the storage area with round-robin allocation.
// A block is considered free if its valid flag is all 1's (0xFFFFFFFF).
// The specified blockExcl is ignored to avoid reusing the same one.
// Starts searching from the last used index to distribute flash wear.
//
sSaveBlock* cBlockStorageManager::findFreeBlock(sSaveBlock* blockExcl)
{
    constexpr uint32_t totalBlocks = NUM_BLOCKS;
    uint32_t index = m_LastFreeIndex % totalBlocks;

    for (uint32_t i = 0; i < totalBlocks; ++i)
    {
        uint32_t currentIndex = (index + i) % totalBlocks;
        sSaveBlock* pBlock = &m_pTabSaveBlock[currentIndex];

        // Skip excluded block
        if (pBlock == blockExcl)
            continue;

        // Check if block is free (effacé) ou supprimé
        if (pBlock->m_isValid == INVALID_MARKER || pBlock->m_isValid == DELETED_MARKER)
        {
            // Update index for next allocation
            m_LastFreeIndex = (currentIndex + 1) % totalBlocks;
            return pBlock;
        }
    }

    // No free blocks found
    return nullptr;
}

// --------------------------------------------------------------------------
// Initializes blocks (optimized).
// Erases flash memory in the largest possible blocks (64K, 32K, or 4K)
// to optimize performance. Processes the entire persistent storage area.
//
void cBlockStorageManager::InitializeBlock() {
    constexpr uint32_t BLOCK_64K_SIZE = 16 * QFLAH_SECTOR_SIZE;  // 64K
    constexpr uint32_t BLOCK_32K_SIZE = 8 * QFLAH_SECTOR_SIZE;   // 32K

    uint8_t* pCurrentBlock = reinterpret_cast<uint8_t*>(m_pTabSaveBlock);
    uint32_t remainingSize = NUM_BLOCKS * BLOCK_SIZE;
    uint32_t currentAddress;

    while (remainingSize > 0) {
        currentAddress = reinterpret_cast<uint32_t>(pCurrentBlock);

        // Prefer largest possible block erase for better performance
        if (remainingSize >= BLOCK_64K_SIZE) {
            __Flash.EraseBlock64K(currentAddress);
            pCurrentBlock += BLOCK_64K_SIZE;
            remainingSize -= BLOCK_64K_SIZE;
        } else if (remainingSize >= BLOCK_32K_SIZE) {
            __Flash.EraseBlock32K(currentAddress);
            pCurrentBlock += BLOCK_32K_SIZE;
            remainingSize -= BLOCK_32K_SIZE;
        } else if (remainingSize >= QFLAH_SECTOR_SIZE) {
            // Default to 4K sector erase
            __Flash.EraseBlock4K(currentAddress);
            pCurrentBlock += QFLAH_SECTOR_SIZE;
            remainingSize -= QFLAH_SECTOR_SIZE;
        } else {
            break;  // Remaining size is less than a sector
        }
    }

    // Réinitialiser l'état des secteurs
    for (uint32_t i = 0; i < m_numSectors; i++) {
        m_pSectorStates[i].usedBlockCount = 0;
        m_pSectorStates[i].needsErase = false;
    }
}

// --------------------------------------------------------------------------
// Finds the first block matching a given saveNumber.
// @return Pointer to first saveNumber block or nullptr if not found
//
sSaveBlock* cBlockStorageManager::FindFirstBlock(uint32_t saveNumber) {
    sSaveBlock* pSaveBlock = m_pTabSaveBlock;
    for (uint32_t i = 0; i < NUM_BLOCKS; i++) {
        if ((pSaveBlock->m_isValid == HEADER_MAGIC) && (pSaveBlock->m_saveNumber == saveNumber)) {
            return pSaveBlock;
        }
        pSaveBlock++;
    }
    return nullptr;  // No matching blocks found
}

// --------------------------------------------------------------------------
// Calcule l'index du secteur à partir d'un pointeur de bloc
// @param pBlock Pointeur vers le bloc
// @return Index du secteur contenant ce bloc
//
uint32_t cBlockStorageManager::getSectorIndex(sSaveBlock* pBlock) {
    uint32_t blockIndex = pBlock - m_pTabSaveBlock;
    return blockIndex / BLOCKS_PER_SECTOR;
}

// --------------------------------------------------------------------------
// Marque un bloc comme invalide (suppression logique)
// @param pBlock Pointeur vers le bloc à invalider
//
void cBlockStorageManager::invalidateBlock(sSaveBlock* pBlock) {
    // En flash, on ne peut passer que de 1 à 0, donc on écrit 0x00000000
    uint32_t marker = DELETED_MARKER;
    uint32_t offset = reinterpret_cast<uint32_t>(&pBlock->m_isValid) -
                     reinterpret_cast<uint32_t>(pBlock);

    __Flash.Write(reinterpret_cast<uint8_t*>(&marker),
                  reinterpret_cast<uint32_t>(pBlock) + offset,
                  sizeof(uint32_t));
}

// --------------------------------------------------------------------------
// Efface un secteur physique si nécessaire
// @param sectorIndex Index du secteur à effacer
//
void cBlockStorageManager::eraseSectorIfNeeded(uint32_t sectorIndex) {
    if (m_pSectorStates[sectorIndex].needsErase) {
        // Calculer l'adresse du début du secteur
        uint32_t sectorAddress = reinterpret_cast<uint32_t>(m_pTabSaveBlock) +
                                (sectorIndex * QFLAH_SECTOR_SIZE);

        __Flash.EraseBlock4K(sectorAddress);
        HAL_Delay(10);  // Ensure erase completion

        // Réinitialiser l'état du secteur
        m_pSectorStates[sectorIndex].usedBlockCount = 0;
        m_pSectorStates[sectorIndex].needsErase = false;
    }
}

// --------------------------------------------------------------------------
// Met à jour l'état d'un secteur après modification
// @param sectorIndex Index du secteur
//
void cBlockStorageManager::updateSectorState(uint32_t sectorIndex) {
    uint32_t usedCount = 0;
    uint32_t deletedCount = 0;

    // Compter les blocs utilisés et supprimés dans ce secteur
    uint32_t startBlock = sectorIndex * BLOCKS_PER_SECTOR;
    uint32_t endBlock = startBlock + BLOCKS_PER_SECTOR;

    for (uint32_t i = startBlock; i < endBlock && i < NUM_BLOCKS; i++) {
        sSaveBlock* pBlock = &m_pTabSaveBlock[i];
        if (pBlock->m_isValid == HEADER_MAGIC) {
            usedCount++;
        } else if (pBlock->m_isValid == DELETED_MARKER) {
            deletedCount++;
        }
    }

    m_pSectorStates[sectorIndex].usedBlockCount = usedCount;

    // Si tous les blocs sont supprimés ou libres, marquer pour effacement
    if (usedCount == 0 && deletedCount > 0) {
        m_pSectorStates[sectorIndex].needsErase = true;
    }
}

} // namespace DadPersistentStorage
