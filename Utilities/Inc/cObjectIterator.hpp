#pragma once
//==================================================================================
//==================================================================================
// File: cObjectIterator.hpp
// Description: Template implementation for object iterator with family grouping
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

namespace DadUtilities {

//**********************************************************************************
// Add an object to a given family (creates the family if it does not exist)
//**********************************************************************************
template<typename ObjectType>
bool cObjectIterator<ObjectType>::addObject(uint32_t familyId, ObjectType object) {
    // Find existing family index
    int8_t index = findFamilyIndex(familyId);

    // Create new family if it doesn't exist
    if (index == -1) {
        if (familyCount_ >= MAX_FAMILIES) {
            return false;    // Maximum family limit reached
        }
        index = familyCount_++;
        families_[index].id    = familyId;    // Set family ID
        families_[index].count = 0;           // Initialize object count
    }

    Family& family = families_[index];
    if (family.count >= MAX_OBJECTS_PER_FAMILY) {
        return false;    // Family object limit reached
    }

    // Add object to family
    family.objects[family.count++] = object;
    return true;
}

//**********************************************************************************
// Set the active family by its ID
//**********************************************************************************
template<typename ObjectType>
bool cObjectIterator<ObjectType>::setActiveFamily(uint32_t familyId) {
    int8_t index = findFamilyIndex(familyId);
    if (index == -1) {
        return false;    // Family not found
    }
    activeFamilyIndex_ = index;    // Set active family index
    return true;
}

//**********************************************************************************
// Get the ID of the currently active family
//**********************************************************************************
template<typename ObjectType>
uint32_t cObjectIterator<ObjectType>::getActiveFamilyId() const {
    if (familyCount_ == 0) {
        return 0;    // No families exist
    }
    return families_[activeFamilyIndex_].id;    // Return active family ID
}

//**********************************************************************************
// Iterate over all objects in the active family
//**********************************************************************************
template<typename ObjectType>
void cObjectIterator<ObjectType>::forEachInActiveFamily(ObjectCallback callback) {
    if (familyCount_ == 0) {
        return;    // No families to iterate
    }

    Family& family = families_[activeFamilyIndex_];
    // Execute callback for each object in active family
    for (size_t i = 0; i < family.count; ++i) {
        callback(family.objects[i]);
    }
}

//**********************************************************************************
// Iterate over all objects in a specific family
//**********************************************************************************
template<typename ObjectType>
void cObjectIterator<ObjectType>::forEachInFamily(uint32_t FamilyID, ObjectCallback callback) {
    int8_t FamilyIndex = findFamilyIndex(FamilyID);
    if (-1 == FamilyIndex) {
        return;    // Family not found
    }

    Family& family = families_[FamilyIndex];
    // Execute callback for each object in specified family
    for (size_t i = 0; i < family.count; ++i) {
        callback(family.objects[i]);
    }
}

//**********************************************************************************
// Get the number of objects in the active family
//**********************************************************************************
template<typename ObjectType>
size_t cObjectIterator<ObjectType>::getActiveFamilySize() const {
    if (familyCount_ == 0) {
        return 0;    // No families exist
    }
    return families_[activeFamilyIndex_].count;    // Return object count
}

//**********************************************************************************
// Get the number of objects in a given family
//**********************************************************************************
template<typename ObjectType>
size_t cObjectIterator<ObjectType>::getFamilySize(uint32_t familyId) const {
    int8_t index = findFamilyIndex(familyId);
    if (index == -1) {
        return 0;    // Family not found
    }
    return families_[index].count;    // Return object count for specified family
}

//**********************************************************************************
// Clear all objects in a given family
//**********************************************************************************
template<typename ObjectType>
bool cObjectIterator<ObjectType>::clearFamily(uint32_t familyId) {
    int8_t index = findFamilyIndex(familyId);
    if (index == -1) {
        return false;    // Family not found
    }
    families_[index].count = 0;    // Reset object count
    return true;
}

//**********************************************************************************
// Clear all families
//**********************************************************************************
template<typename ObjectType>
void cObjectIterator<ObjectType>::clearAll() {
    familyCount_       = 0;    // Reset family count
    activeFamilyIndex_ = 0;    // Reset active family index
}

//**********************************************************************************
// Get the number of existing families
//**********************************************************************************
template<typename ObjectType>
size_t cObjectIterator<ObjectType>::getFamilyCount() const {
    return familyCount_;    // Return total family count
}

//**********************************************************************************
// Check if a given family exists
//**********************************************************************************
template<typename ObjectType>
bool cObjectIterator<ObjectType>::familyExists(uint32_t familyId) const {
    return findFamilyIndex(familyId) != -1;    // Check if family index exists
}

//**********************************************************************************
// Find the internal index of a family by its ID
//**********************************************************************************
template<typename ObjectType>
int8_t cObjectIterator<ObjectType>::findFamilyIndex(uint32_t familyId) const {
    // Search through all families for matching ID
    for (size_t i = 0; i < familyCount_; ++i) {
        if (families_[i].id == familyId) {
            return static_cast<int8_t>(i);    // Return found index
        }
    }
    return -1;    // Family not found
}

} // namespace DadUtilities

//***End of file**************************************************************
