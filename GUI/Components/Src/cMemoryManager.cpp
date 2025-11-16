//==================================================================================
//==================================================================================
// File: cMemoryManager.cpp
// Description: Implementation of memory management for preset slots with MIDI support
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cMemoryManager.h"
#include "HardwareAndCo.h"
#include "ID.h"

namespace DadGUI {

constexpr uint32_t MEM_HEADER_ID = BUILD_ID('M','E','M','A');   // Memory header identifier
constexpr uint32_t SLOT_ID       = BUILD_ID('S','L','O', 0);    // Base ID for memory slots

//**********************************************************************************
// class cMemoryManager
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------
// Function: Init
// Description:
//   Initializes memory system by loading header data. Resets all slots if header
//   is corrupted or invalid, otherwise restores the last active slot.
void cMemoryManager::Init(){
    uint32_t LoadSize = 0;
    __BlockStorageManager.Load(MEM_HEADER_ID, &m_MemoryHeader, sizeof(m_MemoryHeader), LoadSize);

    // Check if stored header size matches expected size
    if (LoadSize != sizeof(m_MemoryHeader)) {
        // Initialize all slots to empty state
        for (uint8_t Index = 0; Index < MAX_SLOT; Index++) {
            m_MemoryHeader.m_SlotID[Index] = 0;                    // Reset slot ID
            __BlockStorageManager.Delete(SLOT_ID + Index);         // Clear storage
        }
        m_MemoryHeader.m_ActiveSlot = 0;                           // Set default active slot
        __BlockStorageManager.Save(MEM_HEADER_ID, &m_MemoryHeader, sizeof(m_MemoryHeader));
    } else {
        // Restore previously active slot
        RestoreSlot(m_MemoryHeader.m_ActiveSlot);
    }
}

// ---------------------------------------------------------------------------------
// Function: MIDI_PresetUp_CallBack
// Description:
//   MIDI callback for preset up command - increments to next available slot
void cMemoryManager::MIDI_PresetUp_CallBack(uint8_t control, uint8_t value, uint32_t userData){
    cMemoryManager* pThis = reinterpret_cast<cMemoryManager*>(userData);
    pThis->IncrementSlot(+1);                                      // Move to next slot
}

// ---------------------------------------------------------------------------------
// Function: MIDI_PresetDown_CallBack
// Description:
//   MIDI callback for preset down command - decrements to previous available slot
void cMemoryManager::MIDI_PresetDown_CallBack(uint8_t control, uint8_t value, uint32_t userData){
    cMemoryManager* pThis = reinterpret_cast<cMemoryManager*>(userData);
    pThis->IncrementSlot(-1);                                      // Move to previous slot
}

// ---------------------------------------------------------------------------------
// Function: MIDI_ProgramChange_CallBack
// Description:
//   MIDI callback for program change - loads specified program slot
void cMemoryManager::MIDI_ProgramChange_CallBack(uint8_t program, uint32_t userData){
    cMemoryManager* pThis = reinterpret_cast<cMemoryManager*>(userData);
    if (pThis->isLoadable(program)){
        pThis->RestoreSlot(program);                               // Load valid program slot
    }
}

// ---------------------------------------------------------------------------------
// Function: RestoreSlot
// Description:
//   Restores GUI state from specified memory slot if data is valid
bool cMemoryManager::RestoreSlot(uint8_t Slot){
    uint32_t Size = __BlockStorageManager.getSize(SLOT_ID + Slot);
    if (Size != m_MemoryHeader.m_SlotSize[Slot]) {
        return false;                                              // Slot size mismatch
    }

    // Allocate buffer for slot data
    uint8_t* pBuffer = new uint8_t[Size];                          // Temporary data buffer
    if (pBuffer == nullptr) {
        return false;                                              // Allocation failed
    }

    uint32_t LoadSize = 0;
    __BlockStorageManager.Load((SLOT_ID + Slot), pBuffer, Size, LoadSize);
    if (LoadSize != Size) {
        delete[] pBuffer;
        return false;                                              // Data load incomplete
    }

    // Deserialize and restore GUI state
    DadPersistentStorage::cSerialize Serializer;
    Serializer.setBuffer(pBuffer, Size);                           // Set serialization buffer
    __GUI.Restore(&Serializer, m_MemoryHeader.m_SlotID[Slot]);     // Restore GUI from data

    // Update active slot information
    m_MemoryHeader.m_ActiveSlot = Slot;                            // Set new active slot
    __BlockStorageManager.Save(MEM_HEADER_ID, &m_MemoryHeader, sizeof(m_MemoryHeader));

    delete[] pBuffer;                                              // Clean up buffer
    return true;
}

// ---------------------------------------------------------------------------------
// Function: SaveSlot
// Description:
//   Saves current GUI state to specified memory slot with given ID
bool cMemoryManager::SaveSlot(uint8_t Slot, uint32_t SlotID){
    DadPersistentStorage::cSerialize Serializer;
    __GUI.Save(&Serializer, SlotID);                               // Serialize GUI state

    const uint8_t* pBuffer = nullptr;                              // Pointer to serialized data
    uint32_t Size = Serializer.getBuffer(&pBuffer);                // Get data size

    // Save to persistent storage
    bool result = __BlockStorageManager.Save((SLOT_ID + Slot), pBuffer, Size);

    if (result == true) {
        // Update slot metadata in header
        m_MemoryHeader.m_SlotID[Slot] = SlotID;                    // Store slot identifier
        m_MemoryHeader.m_SlotSize[Slot] = Size;                    // Store data size
        m_MemoryHeader.m_ActiveSlot = Slot;                        // Set as active slot
        __BlockStorageManager.Save(MEM_HEADER_ID, &m_MemoryHeader, sizeof(m_MemoryHeader));
    }
    return result;
}

// ---------------------------------------------------------------------------------
// Function: ErraseSlot
// Description:
//   Erases specified slot if it's not the currently active slot
bool cMemoryManager::ErraseSlot(uint8_t Slot){
    if (isErasable(Slot)) {
        __BlockStorageManager.Delete(SLOT_ID + Slot);              // Remove from storage
        m_MemoryHeader.m_SlotID[Slot] = 0;                         // Clear slot ID
        m_MemoryHeader.m_SlotSize[Slot] = 0;                       // Clear slot size
        __BlockStorageManager.Save(MEM_HEADER_ID, &m_MemoryHeader, sizeof(m_MemoryHeader));
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------------
// Function: IncrementSlot
// Description:
//   Cycles through slots in specified direction and loads next available slot
void cMemoryManager::IncrementSlot(int8_t Increment){
    uint8_t activeSlot = m_MemoryHeader.m_ActiveSlot;              // Current active slot
    uint8_t targetSlot = activeSlot;                               // Starting search point

    do {
        // Calculate next slot with wrap-around
        targetSlot = (targetSlot + Increment) % MAX_SLOT;
        if (targetSlot >= MAX_SLOT) {
            targetSlot += MAX_SLOT;                                // Handle negative wrap
        }

        // Load slot if valid and available
        if (isLoadable(targetSlot)) {
            RestoreSlot(targetSlot);                               // Restore found slot
            break;                                                 // Exit search loop
        }
    } while (targetSlot != activeSlot);                            // Prevent infinite loop
}

} // namespace DadGUI
//***End of file**************************************************************
