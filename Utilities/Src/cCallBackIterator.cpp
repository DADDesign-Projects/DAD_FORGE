//==================================================================================
//==================================================================================
// File: cCallBackIterator.h
// Description:  Simple callback list / listener pattern using singly linked list
//
// Copyright (c) 2024-2025 Dad Design.
//==================================================================================
//==================================================================================
#include "cCallBackIterator.h"

namespace DadUtilities {

//----------------------------------------------------------------------------------
// Destructor – free all remaining nodes
//----------------------------------------------------------------------------------
cCallBackIterator::~cCallBackIterator()
{
    Clear();
}


//----------------------------------------------------------------------------------
// RegisterListener - inserts in priority order (0 first, 255 last)
//----------------------------------------------------------------------------------
bool cCallBackIterator::RegisterListener(IteratorCallback_t callback, uint32_t listenerContext, uint8_t priority)
{
    if (callback == nullptr)
    {
        return false;
    }

    // Create new node
    CallbackNode* newNode = new CallbackNode(callback, listenerContext, priority);

    if (newNode == nullptr)
    {
        return false;   // allocation failed
    }

    // Insert in priority order (sorted by priority ascending: 0, 1, 2, ..., 255)
    if (head == nullptr || priority < head->priority)
    {
        // Insert at head
        newNode->next = head;
        head = newNode;
    }
    else
    {
        // Find insertion point
        CallbackNode* current = head;
        while (current->next != nullptr && current->next->priority <= priority)
        {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }

    return true;
}


//----------------------------------------------------------------------------------
// UnregisterListener – removes ALL occurrences of this callback pointer
//----------------------------------------------------------------------------------
bool cCallBackIterator::UnregisterListener(IteratorCallback_t callback)
{
    if (callback == nullptr || head == nullptr)
    {
        return false;
    }

    bool removed = false;
    CallbackNode* current  = head;
    CallbackNode* previous = nullptr;

    while (current != nullptr)
    {
        if (current->callback == callback)
        {
            // Remove this node
            if (previous == nullptr)
            {
                // Remove head
                head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            CallbackNode* toDelete = current;
            current = current->next;

            delete toDelete;           // ← replace with pool.Free() in real code
            removed = true;
        }
        else
        {
            previous = current;
            current  = current->next;
        }
    }

    return removed;
}


//----------------------------------------------------------------------------------
// NotifyListeners
//----------------------------------------------------------------------------------
void cCallBackIterator::NotifyListeners(void* parameter)
{
    CallbackNode* current = head;

    // We iterate forward → safe even if a callback unregisters itself
    // (but beware of double-free if badly managed in user code)
    while (current != nullptr)
    {
        if (current->callback != nullptr)
        {
            current->callback(parameter, current->context);
        }
        current = current->next;
    }
}


//----------------------------------------------------------------------------------
// Clear – remove and delete all nodes
//----------------------------------------------------------------------------------
void cCallBackIterator::Clear()
{
    CallbackNode* current = head;

    while (current != nullptr)
    {
        CallbackNode* next = current->next;
        delete current;          // ← replace with pool.Free() in production
        current = next;
    }

    head = nullptr;
}

} // namespace DadUtilities

//***End of file**************************************************************
