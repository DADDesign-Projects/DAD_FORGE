//==================================================================================
//==================================================================================
// File: cMemoryManager.h
// Description: Memory manager class for handling preset slots and MIDI navigation
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"
#include "GUI_Define.h"
#include "cCallBackIterator.h"

namespace DadGUI {

//**********************************************************************************
// class cMemoryManager
//**********************************************************************************

class cMemoryManager {
public:
    cMemoryManager() = default;
    ~cMemoryManager() = default;

    // -----------------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------------

    // Initializes memory manager and reads state from persistent storage
    void Init();

    // MIDI callback for preset up command
    static void MIDI_PresetUp_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // MIDI callback for preset down command
    static void MIDI_PresetDown_CallBack(uint8_t control, uint8_t value, uint32_t userData);

    // MIDI callback for program change messages
    static void MIDI_ProgramChange_CallBack(uint8_t program, uint32_t userData);

    // Restores system state from specified memory slot
    bool RestoreSlot(uint8_t Slot);

    // Saves current system state to specified slot with effect ID
    bool SaveSlot(uint8_t Slot, uint32_t EffectID);

    // Erases data from specified memory slot
    bool ErraseSlot(uint8_t Slot);

    // Checks if slot contains valid loadable data
    inline uint8_t isLoadable(uint8_t Slot){
        return m_MemoryHeader.m_SlotID[Slot] != 0;                 // Non-zero ID indicates valid data
    }

    // Determines if slot can be erased (not active and contains data)
    inline bool isErasable(uint8_t Slot){
        return (m_MemoryHeader.m_SlotID[Slot] != 0) && (m_MemoryHeader.m_ActiveSlot != Slot);
    }

    // Moves active slot selection by specified increment
    void IncrementSlot(int8_t Increment);

    // Returns index of currently active memory slot
    inline uint8_t getActiveSlot(){
        return m_MemoryHeader.m_ActiveSlot;                        // Current active slot index
    }

    // Read if the system is currently restoring a preset from memory
    inline bool IsInRestoreProcess(){
    	return m_RestoreInProcess;
    }

protected:
    // -----------------------------------------------------------------------------
    // Protected Structures
    // -----------------------------------------------------------------------------

    // Memory header structure storing slot metadata
    struct sMemoryHeader{
        uint8_t  m_ActiveSlot;                 // Currently active slot index
        uint32_t m_SlotID[MAX_SLOT];           // Unique identifiers for each slot
        uint32_t m_SlotSize[MAX_SLOT];         // Data size for each slot in bytes
    } m_MemoryHeader;                          // Instance of memory header

    // =============================================================================
    // Protected Member Variables
    // =============================================================================

    bool m_RestoreInProcess = false;		   // Indicates if a restore is in progress

};

} // namespace DadGUI
//***End of file**************************************************************
