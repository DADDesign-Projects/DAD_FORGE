//==================================================================================
//==================================================================================
// File: ID.h
// Description: ID generation utilities for building 32-bit identifiers
// 
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

//**********************************************************************************
// ID Generation Macros
//
// Provides utilities for building 32-bit IDs from ASCII characters
//**********************************************************************************

// -----------------------------------------------------------------------------
// Macro: BUILD_ID
// Description: Builds a 32-bit ID from 4 ASCII characters (e.g. 'TEST')
// Parameters: Four ASCII characters (A, B, C, D)
// Returns: 32-bit unsigned integer combining the four characters
// -----------------------------------------------------------------------------
#define BUILD_ID(A,B,C,D) ((uint32_t)(A) + ((uint32_t)(B)<<8) + ((uint32_t)(C)<<16) + ((uint32_t)(D)<<24))

#define UNIVERSE_ID 0x00000000
#define NO_ID		0xFFFFFFFF

//***End of file**************************************************************
