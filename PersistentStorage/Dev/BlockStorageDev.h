#pragma once
//====================================================================================
// cBlockStorageManager.h
//
// Flash Memory Persistent Storage Manager
// Handles persistent data management in QSPI flash memory.
//
// Copyright(c) 2025 Dad Design.
//====================================================================================
#include "PersistentDefine.h"

// External reference to the Daisy hardware interface
namespace DadPersistentStorage {

//***********************************************************************************
// class cBlockStorageManager
// This class manages persistent storage in QSPI flash memory.
// It handles saving, loading, and deleting data in flash memory blocks.
// Each block contains a header and data area.
//***********************************************************************************

// Flash memory block and storage configuration
// BLOCK_SIZE peut maintenant être plus petit que QFLAH_SECTOR_SIZE
// Exemple: BLOCK_SIZE = 1024 alors que QFLAH_SECTOR_SIZE = 4096
constexpr uint32_t BLOCK_SIZE = 1024;  // Taille de bloc logique (ajustable)

// Vérification que BLOCK_SIZE est un diviseur de QFLAH_SECTOR_SIZE
static_assert(QFLAH_SECTOR_SIZE % BLOCK_SIZE == 0,
    "BLOCK_SIZE must be a divisor of QFLAH_SECTOR_SIZE");

// Nombre de blocs logiques par secteur flash
constexpr uint32_t BLOCKS_PER_SECTOR = QFLAH_SECTOR_SIZE / BLOCK_SIZE;

// Size of the save block header, consisting of:
// - 3 uint32_t (saveNumber, dataSize, isValid)
// - 1 pointer (pNextBlock)
constexpr uint32_t HEADER_SIZE  = (3 * sizeof(uint32_t)) + sizeof(void *);

// Available space for data in each block
// Calculated by subtracting header size from total block size
constexpr uint32_t DATA_SIZE = BLOCK_SIZE - HEADER_SIZE;

// Total number of available blocks for persistent storage
constexpr uint32_t NUM_BLOCKS = BLOCK_STORAGE_MEM_SIZE / BLOCK_SIZE;


//***********************************************************************************
// Structure defining the layout of a save block in flash memory
//***********************************************************************************
struct sSaveBlock {
    uint32_t    m_saveNumber;      // Save identification number
    uint32_t    m_dataSize;        // Size of the data contained in this block
    sSaveBlock* m_pNextBlock;      // Pointer to next block (for multi-block saves)
    uint32_t    m_isValid;         // Validity marker (0xFFFFFFFF = invalid)
    uint8_t     m_Data[DATA_SIZE]; // Data storage array
};


//***********************************************************************************
// Structure pour gérer l'état des secteurs flash
//***********************************************************************************
struct sSectorState {
    uint32_t usedBlockCount;       // Nombre de blocs utilisés dans ce secteur
    bool needsErase;               // Indique si le secteur doit être effacé
};


//***********************************************************************************
// Class Definition
//***********************************************************************************
class cBlockStorageManager {
public:
    // --------------------------------------------------------------------------
    // Constructor
    cBlockStorageManager(sSaveBlock *pTabSaveBlock){
        m_pTabSaveBlock = pTabSaveBlock;
        m_numSectors = NUM_BLOCKS / BLOCKS_PER_SECTOR;
        m_pSectorStates = new sSectorState[m_numSectors]();
    }

    // --------------------------------------------------------------------------
    // Destructor
    ~cBlockStorageManager() {
        delete[] m_pSectorStates;
    }

    // --------------------------------------------------------------------------
    // Initializes the persistent storage system
    bool Init(uint32_t NumBuild);

    // --------------------------------------------------------------------------
    // Initializes the memory
    void InitializeMemory(uint32_t NumBuild);

    // --------------------------------------------------------------------------
    // Initialize blocks
    void InitializeBlock();

    // --------------------------------------------------------------------------
    // Saves data to flash memory by splitting it into blocks if necessary.
    // Each block contains a header (sSaveBlock) and data area.
    // If data is larger than one block, it creates a linked list of blocks.
    // @param saveNumber Unique identifier for the save
    // @param pDataSource Pointer to the data to be saved
    // @param Size Size of the data in bytes
    // @return true if save successful, false if not enough space or error
    //
    bool Save(uint32_t saveNumber, const void* pDataSource, uint32_t Size);

    // --------------------------------------------------------------------------
    // Loads data from flash memory using save number as identifier.
    // Follows the linked list of blocks to reconstruct the complete data.
    // @param saveNumber Identifier of the save to load
    // @param pBuffer Buffer to store the loaded data
    // @param DataSize size of the buffer
    // @param Size Will contain the size of loaded data
    // @return true if load successful, false if save not found
    //
    void Load(uint32_t saveNumber, void* pData, uint32_t DataSize, uint32_t& Size);

    // --------------------------------------------------------------------------
    // Deletes a save from flash memory by marking blocks as invalid.
    // Physical erase is deferred until the entire sector needs to be erased.
    // @param saveNumber Identifier of the save to delete
    //
    void Delete(uint32_t saveNumber);

    // --------------------------------------------------------------------------
    // Gets the size of data from flash memory using save number as identifier.
    // @param saveNumber Identifier of the save
    // @return the size of data, or 0 if saveNumber does not exist
    //
    uint32_t getSize(uint32_t saveNumber);

protected:
    // --------------------------------------------------------------------------
    // Finds a free block in the storage area with round-robin allocation.
    // A block is considered free if its valid flag is all 1's (0xFFFFFFFF).
    // The specified blockExcl will be ignored during the search.
    // @return Pointer to free block or nullptr if none available
    //
    sSaveBlock* findFreeBlock(sSaveBlock* blockExcl);

    // --------------------------------------------------------------------------
    // Finds the first block matching a given saveNumber.
    // @return Pointer to first saveNumber block or nullptr if not found
    //
    sSaveBlock* FindFirstBlock(uint32_t saveNumber);

    // --------------------------------------------------------------------------
    // Calcule l'index du secteur à partir d'un pointeur de bloc
    // @param pBlock Pointeur vers le bloc
    // @return Index du secteur contenant ce bloc
    //
    uint32_t getSectorIndex(sSaveBlock* pBlock);

    // --------------------------------------------------------------------------
    // Marque un bloc comme invalide (suppression logique)
    // @param pBlock Pointeur vers le bloc à invalider
    //
    void invalidateBlock(sSaveBlock* pBlock);

    // --------------------------------------------------------------------------
    // Efface un secteur physique si nécessaire
    // @param sectorIndex Index du secteur à effacer
    //
    void eraseSectorIfNeeded(uint32_t sectorIndex);

    // --------------------------------------------------------------------------
    // Met à jour l'état d'un secteur après modification
    // @param sectorIndex Index du secteur
    //
    void updateSectorState(uint32_t sectorIndex);

    // --------------------------------------------------------------------------
    // Persistent storage blocks array
    //
    sSaveBlock* m_pTabSaveBlock;

    // --------------------------------------------------------------------------
    // Last index used for free block search (for round-robin allocation)
    //
    uint32_t m_LastFreeIndex = 0;

    // --------------------------------------------------------------------------
    // Sector state tracking
    //
    sSectorState* m_pSectorStates;
    uint32_t m_numSectors;
};

} // namespace DadPersistentStorage
