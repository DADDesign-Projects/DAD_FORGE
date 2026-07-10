//==================================================================================
//==================================================================================
// File: AS4C16M32SB.cpp
// Description: SDRAM driver for AS4C16M32SB-6BCN Daisy Seed memory chip
// 
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "AS4C16M32SB.h"

//**********************************************************************************
// External variables
//**********************************************************************************
extern SDRAM_HandleTypeDef hsdram1;

// -----------------------------------------------------------------------------
// SDRAM_Initialize
// -----------------------------------------------------------------------------

void AS4C16M32SB_Initialize()
{
	FMC_SDRAM_CommandTypeDef cmd = {0};

	// Step 1 : Clock Configuration Enable
	cmd.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	cmd.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	cmd.AutoRefreshNumber      = 1;
	cmd.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF);

	// Attente 200 µs minimum (datasheet : 200 µs stable clock avant CKE HIGH) */
	HAL_Delay(1);  // 1 ms >> 200 µs

	// Step 2 : Precharge All
	cmd.CommandMode = FMC_SDRAM_CMD_PALL;
	HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF);

	// Etape 3 : 2 cycles Auto-Refresh (minimum requis par le datasheet)
	cmd.CommandMode       = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	cmd.AutoRefreshNumber = 8;   // 8 pour marge (2 minimum)
	HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF);

	// Step 4 : Mode Register Set
	//  CAS Latency = 3     		→ A6:A4 = 011
	//  Burst Type  = Sequential 	→ A3 = 0
	//  Burst Length = 1    		→ A2:A0 = 000  (ou 4 = 010 selon usage)
	//  Test Mode   = Normal 		→ A8:A7 = 00
	//  Write Burst = Burst  		→ A9 = 0
	//
	//  ModeRegister = 0b0000_0000_0011_0000 = 0x0030
	//  (CL=3, BL=1, Sequential, normal mode, burst write)
	//
	#define SDRAM_MODEREG_BURST_LENGTH_1        ((uint16_t)0x0000)
	#define SDRAM_MODEREG_BURST_LENGTH_4        ((uint16_t)0x0002)
	#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((uint16_t)0x0000)
	#define SDRAM_MODEREG_CAS_LATENCY_3         ((uint16_t)0x0030)
	#define SDRAM_MODEREG_OPERATING_MODE_NORMAL ((uint16_t)0x0000)
	#define SDRAM_MODEREG_WRITEBURST_MODE_BURST ((uint16_t)0x0000)

	cmd.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
	cmd.AutoRefreshNumber = 1;
	cmd.ModeRegisterDefinition = SDRAM_MODEREG_BURST_LENGTH_1        |
								  SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
								  SDRAM_MODEREG_CAS_LATENCY_3         |
								  SDRAM_MODEREG_OPERATING_MODE_NORMAL |
								  SDRAM_MODEREG_WRITEBURST_MODE_BURST;
	HAL_SDRAM_SendCommand(&hsdram1, &cmd, 0xFFFF);

	// Step 5 : Refresh rate
	//  64 ms / 8192 rows = 7.8 µs
	//  7.8 µs × 120 MHz = 936 cycles → marge : 920
	//
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, 920);
}

//***End of file**************************************************************
