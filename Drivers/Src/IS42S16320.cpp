//==================================================================================
//==================================================================================
// File: IS42S16320.cpp
// Description: SDRAM driver for IS42S16320 memory chip
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "IS42S16320.h"

//**********************************************************************************
// External variables
//**********************************************************************************
extern SDRAM_HandleTypeDef hsdram1;

// -----------------------------------------------------------------------------
// SDRAM_Initialize
// -----------------------------------------------------------------------------
void SDRAM_Initialize()
{
    FMC_SDRAM_CommandTypeDef Command = {};  // SDRAM command structure

    // Step 1: Configure a clock configuration enable command
    Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK)
    {
        Error_Handler();
    }

    // Step 2: Insert 100 µs delay
    HAL_Delay(1);  // 1ms delay for stabilization

    // Step 3: Configure a PALL (precharge all) command
    Command.CommandMode = FMC_SDRAM_CMD_PALL;

    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK)
    {
        Error_Handler();
    }

    // Step 4: Configure an Auto-Refresh command
    Command.CommandMode       = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    Command.AutoRefreshNumber = 2;  // Two auto-refresh cycles

    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK)
    {
        Error_Handler();
    }

    // Step 5: Program the mode register
    // - Burst Length: 0 (bits 2:0 = 000)
    // - Burst Type: Sequential (bit 3 = 0)
    // - CAS Latency: 2 (bits 6:4 = 010)
    // - Operating Mode: Standard (bits 8:7 = 00)
    // - Write Burst Mode: Programmed burst length (bit 9 = 0)
    Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0x0020;  // CAS=3, BL=0, Sequential

    if (HAL_SDRAM_SendCommand(&hsdram1, &Command, 1000) != HAL_OK)
    {
        Error_Handler();
    }

    // Step 6: Refresh rate configuration
    // IS42S16320: 8192 refresh cycles for 64ms
    // Refresh rate = 64ms / 8192 = 7.8125μs
    // for 100MHz SDRAM clock: 7.8125μs × 100MHz = 781.25
    if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, 781 - 10) != HAL_OK)
    {
        Error_Handler();
    }
}

//***End of file**************************************************************
