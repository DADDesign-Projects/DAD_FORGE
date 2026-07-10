//==================================================================================
//==================================================================================
// File: IS25LP064A.h
// Description: ISSI IS25LP064A QSPI Flash Memory Management Header
//
// Copyright (c) 2025-26 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "main.h"
#include "iQSPI_FLashMemory.h"

namespace DadDrivers {

//**********************************************************************************
// Constants and Definitions
//**********************************************************************************

// =============================================================================
// IS25LP064A Command Set
// =============================================================================

enum IS25LP064A_Commands {
    // -----------------------------------------------------------------------------
    // BASIC SPI COMMANDS
    // -----------------------------------------------------------------------------
    CMD_WRITE_ENABLE                = 0x06,    // *Write Enable (WREN)
    CMD_WRITE_DISABLE               = 0x04,    // *Write Disable (WRDI)

    // -----------------------------------------------------------------------------
    // STATUS REGISTER COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_STATUS_REG             = 0x05,    // *Read Status Register-1 (RDSR1)
	CMD_WRITE_STATUS_REG            = 0x01,    // *Write Status Register-1 (WRSR1)

    CMD_READ_FUNCTION_REG     		= 0x48,    // *Read Function Register (RDFR)
    CMD_WRITE_FUNCTION_REG     		= 0x42,    // *Write Function Register (WRFR)

    CMD_SET_READ_PARAMETERS  		= 0xC0,    // *Set Read Parameters (SRP)
    // -----------------------------------------------------------------------------
    // QPI Mode
    // -----------------------------------------------------------------------------
	CMD_ENTER_QPI_MODE      		= 0x35,    // Enter Quad Peripheral Interface mode (QPIEN)
    CMD_EXIT_QPI_MODE       		= 0xF5,    // Exit QPI mode (QPIDI)

	// -----------------------------------------------------------------------------
    // DEVICE IDENTIFICATION COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_ID                     = 0xAB,    // *Read ID / Release Power Down (RDID/RDPD)
    CMD_READ_JEDEC_ID               = 0x9F,    // *Read JEDEC ID (RDJDID)
	CMD_READ_JEDEC_ID_QPI 			= 0xAF,    // *Read JEDEC ID in QPI mode (RDJDIDQ)
    CMD_READ_MANUFACTURER_ID        = 0x90,    // *Read Manufacturer & Device ID (RDMDID)
    CMD_READ_UNIQUE_ID              = 0x4B,    // *Read Unique ID (RDUID)
    CMD_READ_SFDP                   = 0x5A,    // *Read SFDP Register (RDSFDP)

    // -----------------------------------------------------------------------------
    // READ COMMANDS
    // -----------------------------------------------------------------------------
    CMD_READ_NORMAL                 = 0x03,    // *Normal Read (READ)
    CMD_READ_FAST                   = 0x0B,    // *Fast Read (FAST_READ)
    CMD_READ_FAST_DUAL_OUT          = 0x3B,    // *Fast Read Dual Output (DREAD)
    CMD_READ_FAST_DUAL_IO           = 0xBB,    // *Fast Read Dual I/O (2READ)
    CMD_READ_FAST_QUAD_OUT          = 0x6B,    // *Fast Read Quad Output (QREAD)
    CMD_READ_FAST_QUAD_IO           = 0xEB,    // *Fast Read Quad I/O (4READ)
    CMD_FAST_READ_DTR        		= 0x0D,    // *Fast Read DTR Mode (FRDTR)
    CMD_FAST_READ_DUAL_IO_DTR 		= 0xBD,    // *Fast Read Dual I/O DTR Mode (FRDDTR)
    CMD_FAST_READ_QUAD_IO_DTR 		= 0xED,    // *Fast Read Quad I/O DTR Mode (FRQDTR)


    // -----------------------------------------------------------------------------
    // PROGRAM COMMANDS
    // -----------------------------------------------------------------------------
    CMD_PAGE_PROGRAM                = 0x02,    // *Page Program (PP)
    CMD_QUAD_PAGE_PROGRAM           = 0x32,    // *Quad Input Page Program (QPP)

    // -----------------------------------------------------------------------------
    // ERASE COMMANDS
    // -----------------------------------------------------------------------------
    CMD_SECTOR_ERASE                = 0x20,    // *Sector Erase 4KB (SE)
    CMD_BLOCK_ERASE_32K             = 0x52,    // *Block Erase 32KB (BE32)
    CMD_BLOCK_ERASE_64K             = 0xD8,    // *Block Erase 64KB (BE64)
    CMD_CHIP_ERASE                  = 0xC7,    // *Chip Erase (CE) - Primary
    CMD_CHIP_ERASE_ALT              = 0x60,    // *Chip Erase (CE) - Alternative

    // -----------------------------------------------------------------------------
    // SUSPEND & RESUME COMMANDS
    // -----------------------------------------------------------------------------
    CMD_ERASE_PROGRAM_SUSPEND       = 0x75,    // *Erase/Program Suspend (EPS)
    CMD_ERASE_PROGRAM_RESUME        = 0x7A,    // *Erase/Program Resume (EPR)

    // -----------------------------------------------------------------------------
    // POWER MANAGEMENT COMMANDS
    // -----------------------------------------------------------------------------
    CMD_POWER_DOWN                  = 0xB9,    // *Deep Power Down (DP)
    CMD_RELEASE_POWER_DOWN          = 0xAB,    // *Release Power Down (RPD)

    // -----------------------------------------------------------------------------
    // RESET COMMANDS
    // -----------------------------------------------------------------------------
    CMD_ENABLE_RESET                = 0x66,    // *Enable Reset (RSTEN)
    CMD_RESET_DEVICE                = 0x99,    // *Reset Device (RST)

	// -----------------------------------------------------------------------------
	// Security & Protection
    // -----------------------------------------------------------------------------
    CMD_LOCK_SECTOR         		= 0x24,	   // *Sector Lock (SECLOCK)
    CMD_UNLOCK_SECTOR       		= 0x26,    // *Sector Unlock (SECUNLOCK)
    CMD_ERASE_INFO_ROW      		= 0x64,    // *Erase Information Row (IRER)
    CMD_PROGRAM_INFO_ROW    		= 0x62,    // *Program Information Row (IRP)
    CMD_READ_INFO_ROW       		= 0x68,    // *Read Information Row (IRRD)

	// -----------------------------------------------------------------------------
    // NO OPERATION
    // -----------------------------------------------------------------------------
    CMD_NOP                         = 0x00     // *No Operation (NOP)
};

// =============================================================================
// Register Structures
// =============================================================================

// -----------------------------------------------------------------------------
// Structure du Status Register pour IS25LP064A
// -----------------------------------------------------------------------------
union IS25LP064A_StatusReg {
	struct{
		uint8_t wip   : 1; // Bit 0: Write In Progress (1 si en écriture, 0 si prêt)
		uint8_t wel   : 1; // Bit 1: Write Enable Latch (1 si activé)
		uint8_t bp0   : 1; // Bit 2: Block Protection Bit 0
		uint8_t bp1   : 1; // Bit 3: Block Protection Bit 1
		uint8_t bp2   : 1; // Bit 4: Block Protection Bit 2
		uint8_t bp3   : 1; // Bit 5: Block Protection Bit 3
		uint8_t qe    : 1; // Bit 6: Quad Enable (1 si mode Quad activé)
		uint8_t srwd  : 1; // Bit 7: Status Register Write Disable
	};
	uint8_t octet;
};

// -----------------------------------------------------------------------------
// Structure du Function Register pour IS25LP064A
// -----------------------------------------------------------------------------
union IS25LP064A_FunctionReg{
	struct  {
		uint8_t reserved : 1; // Reserved
		uint8_t tbs      : 1; // Top/Bottom Selection (0 = Top, 1 = Bottom)
		uint8_t psus     : 1; // Program Suspend Status
		uint8_t esus     : 1; // Erase Suspend Status
		uint8_t irl      : 4; // Information Row Lock Bit 0
	};
	uint8_t octet;
};

// -----------------------------------------------------------------------------
// Structure du Read Register pour IS25LP064A
// -----------------------------------------------------------------------------
union IS25LP064A_ReadReg{
	struct {
		uint8_t Brush : 2; // Burst Length
		uint8_t wrap  : 1; // Wrap Enable
		uint8_t dc    : 2; // Dummy Cycle Bit
		uint8_t ods   : 3; // Output Driver Strength

	};
	uint8_t	octet;
};
//**********************************************************************************
// Device Configuration Constants
//**********************************************************************************

#define IS25LP064A_MANUFACTURER_ID     0x9D        // ISSI Manufacturer ID
#define IS25LP064A_DEVICE_ID           0x60        // Device ID for IS25LP064A (64Mb)
#define IS25LP064A_PAGE_SIZE           256         // Page size in bytes
#define IS25LP064A_SECTOR_SIZE         4096        // Sector size in bytes (4KB)
#define IS25LP064A_BLOCK_32K_SIZE      32768       // 32KB block size in bytes
#define IS25LP064A_BLOCK_64K_SIZE      65536       // 64KB block size in bytes
#define IS25LP064A_TOTAL_SIZE          8388608     // Total size in bytes (8MB)
#define IS25LP064A_TOTAL_PAGES         32768       // Total number of pages
#define IS25LP064A_TOTAL_SECTORS       2048        // Total number of 4KB sectors
#define IS25LP064A_TOTAL_BLOCKS_64K    128         // Total number of 64KB blocks

static constexpr uint32_t DEFAULT_TIMEOUT = 5000;       // Default operation timeout in ms
static constexpr uint32_t ERASE_4K_TIMEOUT = 10000;     // 4KB erase timeout in ms
static constexpr uint32_t ERASE_32K_TIMEOUT = 15000;    // 32KB erase timeout in ms
static constexpr uint32_t ERASE_64K_TIMEOUT = 20000;    // 64KB erase timeout in ms
static constexpr uint32_t CHIP_ERASE_TIMEOUT = 120000;  // Chip erase timeout in ms

//**********************************************************************************
// cIS25LP064A Class Declaration
//**********************************************************************************

// =============================================================================
// cIS25LP064A Class
//
// Implements the iQSPI_FlashMemory interface for the ISSI IS25LP064A chip.
//
// This class supports full 4-line read/write operations and includes:
// - Initialization with software reset
// - Quad mode configuration
// - Indirect and memory-mapped mode support
// - Sector/block/chip erase functionality
// =============================================================================
class cIS25LP064A : public iQSPI_FlashMemory {
public:
    // -----------------------------------------------------------------------------
    // Constructor/Destructor
    // -----------------------------------------------------------------------------
    cIS25LP064A();
    virtual ~cIS25LP064A() = default;

    // =============================================================================
    // Public Interface Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Initializes the QSPI interface and the flash memory.
    // Performs a reset and enables Quad mode.
    // Parameters:
    //   phqspi: QSPI handle pointer
    //   DualMode: Dual mode operation flag
    //   MemoryAddress: Memory mapped address
    // Returns: HAL status code
    HAL_StatusTypeDef Init(QSPI_HandleTypeDef* phqspi, bool DualMode = false, uint32_t MemoryAddress = 0x90000000) override;

    // -----------------------------------------------------------------------------
    // Configures the QSPI memory in memory-mapped mode so that the CPU can access the flash
    // memory directly without issuing commands.
    HAL_StatusTypeDef ModeMemoryMap() override;

    // -----------------------------------------------------------------------------
    // Configures the QSPI memory in indirect access mode
    HAL_StatusTypeDef ModeIndirect() override;

    // -----------------------------------------------------------------------------
    // Reads data from the QSPI memory using Quad I/O (4-4-4)
    // Parameters:
    //   pData: Pointer to data buffer
    //   Address: Memory address to read from
    //   NbData: Number of bytes to read
    HAL_StatusTypeDef Read(uint8_t* pData, uint32_t Address, uint32_t NbData) override;

    // -----------------------------------------------------------------------------
    // Writes data to the QSPI memory using Quad Page Program
    // Parameters:
    //   pData: Pointer to data buffer
    //   Address: Memory address to write to
    //   NbData: Number of bytes to write
    HAL_StatusTypeDef Write(uint8_t* pData, uint32_t Address, uint32_t NbData) override;

    // -----------------------------------------------------------------------------
    // Erases a 4 KB sector of the QSPI memory
    HAL_StatusTypeDef EraseBlock4K(uint32_t Address) override;

    // -----------------------------------------------------------------------------
    // Erases a 32 KB block of the QSPI memory
    HAL_StatusTypeDef EraseBlock32K(uint32_t Address) override;

    // -----------------------------------------------------------------------------
    // Erases a 64 KB block of the QSPI memory
    HAL_StatusTypeDef EraseBlock64K(uint32_t Address) override;

    // -----------------------------------------------------------------------------
    // Erases the entire QSPI memory chip
    HAL_StatusTypeDef EraseChip() override;

    // -----------------------------------------------------------------------------
    // Returns the total size of the memory in bytes
    uint32_t getSize() const override;

    // -----------------------------------------------------------------------------
    // Returns the ID of flash memory
    HAL_StatusTypeDef getFlashID(FlashID* pID) override;

private:
    // =============================================================================
    // Private Member Variables
    // =============================================================================
    QSPI_HandleTypeDef* m_pQSPI;                  // Pointer to the HAL QSPI handle
    uint32_t m_MemoryMappedBaseAddress;           // Base address for memory-mapped access
    bool m_DualMode;                              // Flag for dual memory mode operation
    bool m_MappedMode;                            // Flag to indicate if the memory is in mapped mode

    // =============================================================================
    // Private Methods
    // =============================================================================

    // -----------------------------------------------------------------------------
    // Enables write operations on the memory.
    HAL_StatusTypeDef WriteEnable();

    // -----------------------------------------------------------------------------
    // Waits until the Write-In-Progress bit is cleared.
    HAL_StatusTypeDef WaitWhileBusy(uint32_t Timeout = DEFAULT_TIMEOUT);
};

} // namespace DadDrivers
