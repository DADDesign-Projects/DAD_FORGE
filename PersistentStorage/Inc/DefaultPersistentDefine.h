//==================================================================================
//==================================================================================
// File: DefaultPersistentDefine.h
// Description: Default persistent storage configuration and memory layout definitions
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once
#if __has_include("PersistentDefine.h")
    #include "PersistentDefine.h"
#else
	#define DUMMY_INCLUDE_PERSISTENT

	//**********************************************************************************
	// FLASH - W25Q128 in double mode
	//**********************************************************************************

	#include "W25Q128.h"

// *****************************************************************************
	// Global variables declarations
	// *****************************************************************************
	// External flash memory instance
	extern DadDrivers::cW25Q128 __Flash;

	// Flash memory configuration constants
	constexpr uint32_t QFLASH_SIZE       = 16 * 2 * 1024 * 1034;   // 16MB * 2 (dual mode)
	constexpr uint32_t QFLAH_SECTOR_SIZE = 8 * 1024;               // 8KB per sector
	constexpr bool     DOUBLE_MODE       = true;                   // Enable double mode

	// Flash memory base address
	#define FLASH_ADDRESS 0x90000000

	//**********************************************************************************
	// Flasher Storage Configuration
	//**********************************************************************************

	// Base address of external QSPI flash (16 MB total)
	constexpr uint32_t FLASHER_ADDRESS  = FLASH_ADDRESS;      // Base QSPI Flash address
	constexpr uint32_t FLASHER_MEM_SIZE = 16 * 1024 * 1024;   // 16 MB total size

	//**********************************************************************************
	// Block Storage Manager Configuration
	//**********************************************************************************

	// Block Storage Manager region (upper 8 MB of flash)
	constexpr uint32_t BLOCK_STORAGE_ADDRESS  = FLASH_ADDRESS + FLASHER_MEM_SIZE;   // After Flasher Storage
	constexpr uint32_t BLOCK_STORAGE_MEM_SIZE = QFLAH_SECTOR_SIZE * 1024;           // 1024 blocs = 8 M

	//**********************************************************************************
	// Loader storage zone
	//**********************************************************************************
	constexpr uint32_t LOADER_STORAGE_ADDRESS  = BLOCK_STORAGE_ADDRESS + BLOCK_STORAGE_MEM_SIZE;  // After Bloc Storage
	constexpr uint32_t LOADER_MEM_SIZE  = QFLAH_SECTOR_SIZE;  									  // One sector

#endif

//***End of file**************************************************************
