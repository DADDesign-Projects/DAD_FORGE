#pragma once
//==================================================================================
//==================================================================================
// File: cObjectIterator.h
// Description: Object family management and serialization helper
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "main.h"
#include <cstdint>
#include <array>
#include <functional>

namespace DadUtilities {

//**********************************************************************************
// Constants
//**********************************************************************************
constexpr size_t MAX_FAMILIES           = 8;     // Maximum number of families
constexpr size_t MAX_OBJECTS_PER_FAMILY = 32;    // Maximum objects per family

//**********************************************************************************
// Template class: cObjectIterator
// Description: Provides object grouping and iteration by family ID
//**********************************************************************************
template<typename ObjectType>
class cObjectIterator {
public:
    using ObjectCallback = std::function<void(ObjectType)>;    // Callback type for object iteration

    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    cObjectIterator() : familyCount_(0), activeFamilyIndex_(0) {}

    // -----------------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------------

    // Add an object to a given family (creates the family if it does not exist)
    bool addObject(uint32_t familyId, ObjectType object);

    // Set the active family by its ID
    bool setActiveFamily(uint32_t familyId);

    // Get the ID of the currently active family
    uint32_t getActiveFamilyId() const;

    // Iterate over all objects in the active family
    void forEachInActiveFamily(ObjectCallback callback);

    // Iterate over all objects in the specified family
    void forEachInFamily(uint32_t FamilyID, ObjectCallback callback);

    // Get the number of objects in the active family
    size_t getActiveFamilySize() const;

    // Get the number of objects in a given family
    size_t getFamilySize(uint32_t familyId) const;

    // Clear all objects in a given family
    bool clearFamily(uint32_t familyId);

    // Clear all families
    void clearAll();

    // Get the number of existing families
    size_t getFamilyCount() const;

    // Check if a given family exists
    bool familyExists(uint32_t familyId) const;

private:
    //******************************************************************************
    // Internal structure: Family
    // Description: Represents a group of objects with common family ID
    //******************************************************************************
    struct Family {
        uint32_t id;                                            // Family identifier
        std::array<ObjectType, MAX_OBJECTS_PER_FAMILY> objects; // Object storage array
        size_t count;                                           // Current object count
    };

    // -----------------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------------

    // Find the internal index of a family by its ID
    int8_t findFamilyIndex(uint32_t familyId) const;

    //******************************************************************************
    // Member Variables
    //******************************************************************************
    std::array<Family, MAX_FAMILIES> families_;    // Family storage array
    size_t familyCount_;                           // Number of currently used families
    size_t activeFamilyIndex_;                     // Index of the active family
};

} // namespace DadUtilities

// Include template implementation
#include "cObjectIterator.hpp"

//***End of file**************************************************************
