//==================================================================================
//==================================================================================
// File: IS25LP064A.cpp
// Description: ISSI IS25LP064A QSPI Flash Memory Management
//
// Copyright (c) 2025-26 Dad Design.
//==================================================================================
//==================================================================================
#include "IS25LP064A.h"

namespace DadDrivers {

//**********************************************************************************
// Macros
//**********************************************************************************

// Macro to validate memory mapped address and convert to flash address
#define VALID_ADDRESS(AddrFlash, AddrMemoryMap)\
    if((AddrMemoryMap < m_MemoryMappedBaseAddress) || (AddrMemoryMap > (m_MemoryMappedBaseAddress + getSize()))){\
        return HAL_ERROR;\
    }else{\
        AddrFlash = AddrMemoryMap - m_MemoryMappedBaseAddress;\
    }

// Macro to enter command mode (switch from memory mapped to indirect mode if needed)
#define ENTER_CMD\
    bool InitialMapState = m_MappedMode;\
    if(m_MappedMode){\
        if((Result = ModeIndirect()) != HAL_OK){\
            return Result;\
        }\
    }

// Macro to exit command mode (restore memory mapped mode if it was initially active)
#define EXIT_CMD\
    if(InitialMapState == true){\
        Result = ModeMemoryMap();\
    }

// Macro to check HAL function results and return on error
#define CHECK_RESULT(Cmd)\
    if((Result = Cmd) != HAL_OK){\
        return Result;\
    }

// =============================================================================
// Constructor/Destructor
// =============================================================================

// -----------------------------------------------------------------------------
// Constructor - Initialize member variables
cIS25LP064A::cIS25LP064A(){
    m_pQSPI = nullptr;                    // QSPI handle pointer
    m_MemoryMappedBaseAddress = 0;        // Memory mapped base address
    m_MappedMode = false;                 // Memory mapped mode flag
    m_DualMode = false;                   // Dual flash mode flag
}

// =============================================================================
// Public Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Initializes the QSPI memory: performs a software reset and enables Quad mode
// Parameters:
//   phqspi: QSPI handle pointer
//   DualMode: Enable dual flash mode (two flash chips)
//   MemoryMappedBaseAddress: Base address for memory mapping
HAL_StatusTypeDef cIS25LP064A::Init(QSPI_HandleTypeDef* phqspi, bool DualMode, uint32_t MemoryMappedBaseAddress){
    HAL_StatusTypeDef Result = HAL_OK;    // Operation result
    QSPI_CommandTypeDef cmd;              // QSPI command structure

    //==================================================================
    // Store configuration parameters
    m_pQSPI = phqspi;                     // Store QSPI handle
    m_DualMode = DualMode;                // Store dual mode setting
    m_MemoryMappedBaseAddress = MemoryMappedBaseAddress; // Store memory mapped base address
    m_MappedMode = false;                 // Start in indirect mode

    //==================================================================
    // Perform software reset to ensure clean state
    cmd = {0};
    cmd.Instruction = CMD_ENABLE_RESET;   // Enable Reset command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    cmd.Instruction = CMD_RESET_DEVICE;   // Reset Device command
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    HAL_Delay(100);                       // Wait for reset to complete

    //==================================================================
    // Configure Read Register
    IS25LP064A_ReadReg ReadReg;
    ReadReg.octet = 0;
    ReadReg.ods = 0b111;    // Driver strength: 50%
    ReadReg.dc = 0b10;      // 6 Dummy cycles and 2 Mode bit cycles
    ReadReg.wrap = 0;       // No wrap
    ReadReg.Brush = 0;

    // Enable write operations
    cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Write register
    cmd = {0};
    cmd.Instruction = CMD_SET_READ_PARAMETERS;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;                       // Write one byte
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, (uint8_t *)&ReadReg, DEFAULT_TIMEOUT));

    CHECK_RESULT(WaitWhileBusy());        // Wait for write operation to complete

    //==================================================================
    // Configure Status Register
    IS25LP064A_StatusReg StatusReg;
    StatusReg.octet = 0;
    StatusReg.qe = 1; // Set Quad mode enable bit

    // Enable write operation
    cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;   // Write Enable command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Write register
    cmd = {0};
    cmd.Instruction = CMD_WRITE_STATUS_REG;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;                       // Write one byte
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, (uint8_t *)&StatusReg, DEFAULT_TIMEOUT));

    CHECK_RESULT(WaitWhileBusy());        // Wait for write operation to complete

    //==================================================================
    // Switch to memory mapped mode
    CHECK_RESULT(ModeMemoryMap());

    return Result;
}

// -----------------------------------------------------------------------------
// Configure the memory in memory-mapped mode for direct CPU access
HAL_StatusTypeDef cIS25LP064A::ModeMemoryMap(){
    HAL_StatusTypeDef Result = HAL_OK;

    if(m_MappedMode == true) return Result;

    // Send NOP command to prepare for mode change
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_NOP;            // No operation command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    // Configure memory mapped settings
    QSPI_MemoryMappedTypeDef mem_mapped_cfg = {0};
    mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    // Setup fast read quad I/O command (4-4-4 mode)
    cmd = {0};
    cmd.Instruction = CMD_READ_FAST_QUAD_IO; // Fast Read Quad I/O command
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.AlternateBytes = 0x000000A0;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 6;                  // Required dummy cycles for fast read
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    //cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;


    // Enable memory mapped mode
    CHECK_RESULT(HAL_QSPI_MemoryMapped(m_pQSPI, &cmd, &mem_mapped_cfg));
    m_MappedMode = true;

    // Invalidate CPU caches for memory mapped region
    if ((SCB->CCR & (1 << 17)) != 0) {
        SCB_InvalidateICache_by_Addr((uint32_t*)m_MemoryMappedBaseAddress, getSize());
    }
    if ((SCB->CCR & (1 << 16)) != 0) {
        SCB_InvalidateDCache_by_Addr((uint32_t*)m_MemoryMappedBaseAddress, getSize());
    }

    return Result;
}

// -----------------------------------------------------------------------------
// Reverts the flash memory to indirect mode for standard read/write commands
HAL_StatusTypeDef cIS25LP064A::ModeIndirect(){
    HAL_StatusTypeDef Result = HAL_OK;

    if(m_MappedMode == false) return Result;

    CHECK_RESULT(HAL_QSPI_Abort(m_pQSPI));
    m_MappedMode = false;

    // Send NOP command after mode change
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_NOP;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    CHECK_RESULT(WaitWhileBusy());        // Wait for write operation to complete

    return Result;
}

// -----------------------------------------------------------------------------
// Reads data from the QSPI memory using Quad I/O mode (4-4-4)
// Parameters:
//   pData: Pointer to buffer to store read data
//   MappedAddress: Memory mapped address to read from
//   NbData: Number of bytes to read
HAL_StatusTypeDef cIS25LP064A::Read(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    uint32_t Address;
    VALID_ADDRESS(Address, MappedAddress);

    // Setup fast read quad I/O command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_READ_FAST_QUAD_IO;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_4_LINES;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    cmd.AlternateBytes = 0x000000A0;
    cmd.DataMode = QSPI_DATA_4_LINES;
    cmd.DummyCycles = 6;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    cmd.NbData = NbData;
    cmd.Address = Address;


    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, pData, DEFAULT_TIMEOUT));

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Writes data to the flash memory using Quad Page Program
HAL_StatusTypeDef cIS25LP064A::Write(uint8_t* pData, uint32_t MappedAddress, uint32_t NbData){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    uint32_t Address;
    VALID_ADDRESS(Address, MappedAddress);
    if ((Address + NbData) > getSize()) return HAL_ERROR;

    // Setup quad page program command
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_QUAD_PAGE_PROGRAM;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DdrMode = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    cmd.DataMode = QSPI_DATA_4_LINES;


    // Write data in page-aligned chunks
    while (NbData > 0)
    {
        uint32_t page_size = IS25LP064A_PAGE_SIZE - (Address % IS25LP064A_PAGE_SIZE);
        uint32_t write_size = (NbData < page_size) ? NbData : page_size;

        CHECK_RESULT(WriteEnable());

        cmd.Address = Address;
        cmd.NbData = write_size;

        CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
        CHECK_RESULT(HAL_QSPI_Transmit(m_pQSPI, pData, DEFAULT_TIMEOUT));

        CHECK_RESULT(WaitWhileBusy());

        Address += write_size;
        pData += write_size;
        NbData -= write_size;
    }

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Erases a 4 KB sector of the QSPI memory
HAL_StatusTypeDef cIS25LP064A::EraseBlock4K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    uint32_t Address;
    VALID_ADDRESS(Address, MappedAddress);

    CHECK_RESULT(WriteEnable());

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_SECTOR_ERASE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(WaitWhileBusy(ERASE_4K_TIMEOUT));

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Erases a 32 KB block of the QSPI memory
HAL_StatusTypeDef cIS25LP064A::EraseBlock32K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    uint32_t Address;
    VALID_ADDRESS(Address, MappedAddress);

    CHECK_RESULT(WriteEnable());

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_BLOCK_ERASE_32K;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;
    cmd.DataMode = QSPI_DATA_NONE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(WaitWhileBusy(ERASE_32K_TIMEOUT));

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Erases a 64 KB block of the QSPI memory
HAL_StatusTypeDef cIS25LP064A::EraseBlock64K(uint32_t MappedAddress){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    uint32_t Address;
    VALID_ADDRESS(Address, MappedAddress);

    CHECK_RESULT(WriteEnable());

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_BLOCK_ERASE_64K;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.AddressMode = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize = QSPI_ADDRESS_24_BITS;
    cmd.Address = Address;
    cmd.DataMode = QSPI_DATA_NONE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(WaitWhileBusy(ERASE_64K_TIMEOUT));

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Erases the entire QSPI memory chip
HAL_StatusTypeDef cIS25LP064A::EraseChip(){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    CHECK_RESULT(WriteEnable());

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_CHIP_ERASE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(WaitWhileBusy(CHIP_ERASE_TIMEOUT));

    EXIT_CMD;
    return Result;
}

// -----------------------------------------------------------------------------
// Returns the total size of the memory in bytes
uint32_t cIS25LP064A::getSize() const
{
    if(m_DualMode){
        return IS25LP064A_TOTAL_SIZE * 2;
    }else{
        return IS25LP064A_TOTAL_SIZE;
    }
}

// -----------------------------------------------------------------------------
// Returns the ID of flash memory
HAL_StatusTypeDef cIS25LP064A::getFlashID(FlashID* pID){
    HAL_StatusTypeDef Result = HAL_OK;
    ENTER_CMD;

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_READ_JEDEC_ID;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;

    if(m_DualMode){
        cmd.NbData = 6;
    }else{
        cmd.NbData = 3;
    }

    uint8_t Data[6] = {};

    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));
    CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, Data, DEFAULT_TIMEOUT));

    if(m_DualMode){
        if((Data[4] != Data[5]) || (Data[2] != Data[3]) || (Data[0] != Data[1])){
            return HAL_ERROR;
        }
        pID->ManufactuerID = Data[0];
        pID->MemoryType    = Data[2];
        pID->Capacity      = Data[4];
    }else{
        pID->ManufactuerID = Data[0];
        pID->MemoryType    = Data[1];
        pID->Capacity      = Data[2];
    }

    EXIT_CMD;
    return Result;
}

// =============================================================================
// Private Methods
// =============================================================================

// -----------------------------------------------------------------------------
// Enables write operations on the memory.
HAL_StatusTypeDef cIS25LP064A::WriteEnable(){
    HAL_StatusTypeDef Result = HAL_OK;

    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_WRITE_ENABLE;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, DEFAULT_TIMEOUT));

    return Result;
}

// -----------------------------------------------------------------------------
// Waits until the Write-In-Progress bit is cleared.
HAL_StatusTypeDef cIS25LP064A::WaitWhileBusy(uint32_t Timeout){
    HAL_StatusTypeDef Result = HAL_OK;
    uint32_t startTick = HAL_GetTick();
    
    QSPI_CommandTypeDef cmd = {0};
    cmd.Instruction = CMD_READ_STATUS_REG;
    cmd.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    cmd.DataMode = QSPI_DATA_1_LINE;
    cmd.NbData = 1;

    while (HAL_GetTick() - startTick < Timeout) {
        CHECK_RESULT(HAL_QSPI_Command(m_pQSPI, &cmd, 100));
        uint8_t status = 0;
        CHECK_RESULT(HAL_QSPI_Receive(m_pQSPI, &status, 100));
        if (!(status & 0x01)) { // BUSY bit is bit 0
        	return HAL_OK;
        }
    }
    return HAL_TIMEOUT;
}

} // namespace DadDrivers
