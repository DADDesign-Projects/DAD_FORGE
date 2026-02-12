//==================================================================================
//==================================================================================
// File: cCallBackIterator.h
// Description:  Simple callback list / listener pattern using singly linked list
//
// Copyright (c) 2024-2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include <stdint.h>

namespace DadUtilities {

//==================================================================================
// Callback function prototype
//   parameter    : user pointer (often 'this' of the object that changed)
//   contextValue : user-defined 32-bit value (can be used as ID, priority, etc.)
//==================================================================================
using IteratorCallback_t = void (*)(void* parameter, uint32_t contextValue);


//==================================================================================
// Forward declaration of node structure
//==================================================================================
struct CallbackNode;


//==================================================================================
// cCallBackIterator – manages a list of callbacks (listeners)
//==================================================================================
class cCallBackIterator
{
public:

    // -------------------------------------------------------------------------
    // Constructor / Destructor
    // -------------------------------------------------------------------------
    cCallBackIterator() : head(nullptr) {}
    ~cCallBackIterator();


    // -------------------------------------------------------------------------
    // Register a new listener (callback + context value + priority)
    // priority: 0 = notified first, 255 = notified last, default = 127
    // Returns true if registration succeeded
    // -------------------------------------------------------------------------
    bool RegisterListener(IteratorCallback_t callback, uint32_t listenerContext, uint8_t priority = 127);


    // -------------------------------------------------------------------------
    // Remove a specific listener (identified by callback pointer ONLY)
    // Returns true if at least one entry was removed
    // -------------------------------------------------------------------------
    bool UnregisterListener(IteratorCallback_t callback);


    // -------------------------------------------------------------------------
    // Notify all registered listeners
    // -------------------------------------------------------------------------
    void NotifyListeners(void* parameter);


    // -------------------------------------------------------------------------
    // Remove ALL listeners
    // -------------------------------------------------------------------------
    void Clear();


private:

    // -------------------------------------------------------------------------
    // Internal node structure (singly linked list)
    // -------------------------------------------------------------------------
    struct CallbackNode
    {
        IteratorCallback_t  callback;
        uint32_t            context;
        uint8_t             priority;    // 0 = highest priority, 255 = lowest
        CallbackNode*       next;

        // Constructor
        CallbackNode(IteratorCallback_t cb, uint32_t ctx, uint8_t prio, CallbackNode* nxt = nullptr)
            : callback(cb), context(ctx), priority(prio), next(nxt)
        {}
    };
    // Head of the singly linked list
    CallbackNode* head;

};

} // namespace DadUtilities

//***End of file**************************************************************
