//==================================================================================
//==================================================================================
// File: EventManager.h
// Description: Template class for managing events with callback subscriptions
//              and family-based filtering
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "main.h"

namespace DadUtilities {

//**********************************************************************************
// Structure: SubscriberNode
// Description: Linked list node for storing event subscribers with family support
//**********************************************************************************
template<typename Interface, typename ReturnType, typename... Args>
struct SubscriberNode {
    Interface* subscriber;
    ReturnType (Interface::*callback)(Args...);
    uint32_t family;
    SubscriberNode* next;

    SubscriberNode(Interface* sub, ReturnType (Interface::*cb)(Args...), uint32_t fam = 0)
        : subscriber(sub), callback(cb), family(fam), next(nullptr) {}
};

//**********************************************************************************
// Class: EventManager (Generic version)
// Description: Template class for managing event subscriptions and triggering
//              with family-based filtering
//**********************************************************************************
template<typename Interface, typename ReturnType, typename... Args>
class EventManager {
public:
    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    EventManager() : head(nullptr), tail(nullptr), subscriberCount(0) {}

    // -----------------------------------------------------------------------------
    // Destructor
    // -----------------------------------------------------------------------------
    ~EventManager() {
        Clear();
    }

    // -----------------------------------------------------------------------------
    // Delete copy constructor and assignment operator
    // -----------------------------------------------------------------------------
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // -----------------------------------------------------------------------------
    // Method: Subscribe
    // Description: Subscribe an object with a callback method to the event
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - callback: Member function pointer to call
    //   - family: Family ID (0 = receives all events, other values = specific family)
    // -----------------------------------------------------------------------------
    void Subscribe(Interface* subscriber, ReturnType (Interface::*callback)(Args...), uint32_t family = 0) {
        if (!subscriber || !callback) {
            return;
        }

        // Create a new node for the subscriber
        SubscriberNode<Interface, ReturnType, Args...>* newNode =
            new SubscriberNode<Interface, ReturnType, Args...>(subscriber, callback, family);

        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }

        subscriberCount++;
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe
    // Description: Remove all subscriptions for a specific subscriber
    // -----------------------------------------------------------------------------
    void Unsubscribe(Interface* subscriber) {
        if (!subscriber || !head) {
            return;
        }

        SubscriberNode<Interface, ReturnType, Args...>* current = head;
        SubscriberNode<Interface, ReturnType, Args...>* previous = nullptr;

        while (current) {
            if (current->subscriber == subscriber) {
                if (previous) {
                    previous->next = current->next;
                } else {
                    head = current->next;
                }

                if (current == tail) {
                    tail = previous;
                }

                SubscriberNode<Interface, ReturnType, Args...>* toDelete = current;
                current = current->next;
                delete toDelete;
                subscriberCount--;
            } else {
                previous = current;
                current = current->next;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Method: SetSubscriberFamily
    // Description: Change the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - newFamily: New family ID to assign
    // Returns: true if subscriber was found and updated, false otherwise
    // -----------------------------------------------------------------------------
    bool SetSubscriberFamily(Interface* subscriber, uint32_t newFamily) {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, ReturnType, Args...>* current = head;
        bool found = false;

        while (current) {
            if (current->subscriber == subscriber) {
                current->family = newFamily;
                found = true;
            }
            current = current->next;
        }

        return found;
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberFamily
    // Description: Get the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - outFamily: Reference to store the family ID
    // Returns: true if subscriber was found, false otherwise
    // -----------------------------------------------------------------------------
    bool GetSubscriberFamily(Interface* subscriber, uint32_t& outFamily) const {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, ReturnType, Args...>* current = head;

        while (current) {
            if (current->subscriber == subscriber) {
                outFamily = current->family;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent
    // Description: Call all subscribed callbacks matching the specified family
    // Parameters:
    //   - family: Family ID to send to (subscribers with family=0 always receive)
    //   - args: Arguments to pass to the callbacks
    // -----------------------------------------------------------------------------
    inline void sendEvent(uint32_t family, Args... args) {
        SubscriberNode<Interface, ReturnType, Args...>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                // Send to subscribers with family=0 (receive all) or matching family
                if (current->family == 0 || current->family == family) {
                    (current->subscriber->*(current->callback))(args...);
                }
            }
            current = current->next;
        }
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToAll
    // Description: Call all subscribed callbacks
    // Parameters:
    //   - args: Arguments to pass to the callbacks
    // -----------------------------------------------------------------------------
    inline void sendEventToAll(Args... args) {
        SubscriberNode<Interface, ReturnType, Args...>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                (current->subscriber->*(current->callback))(args...);
            }
            current = current->next;
        }
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount
    // Description: Get the current number of subscribers
    // Parameters:
    //   - family: Optional family filter (if provided, count only that family)
    //   - includeUniversal: If true and family is specified, include family=0 subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        if (family == UINT32_MAX) {
            return subscriberCount;
        }

        int count = 0;
        SubscriberNode<Interface, ReturnType, Args...>* current = head;

        while (current) {
            if (current->family == family || (includeUniversal && current->family == 0)) {
                count++;
            }
            current = current->next;
        }

        return count;
    }

    // -----------------------------------------------------------------------------
    // Method: IsEmpty
    // Description: Check if there are no subscribers
    // -----------------------------------------------------------------------------
    bool IsEmpty() const {
        return head == nullptr;
    }

    // -----------------------------------------------------------------------------
    // Method: Clear
    // Description: Remove all subscribers and clean up memory
    // -----------------------------------------------------------------------------
    void Clear() {
        SubscriberNode<Interface, ReturnType, Args...>* current = head;

        while (current) {
            SubscriberNode<Interface, ReturnType, Args...>* next = current->next;
            delete current;
            current = next;
        }

        head = tail = nullptr;
        subscriberCount = 0;
    }

private:
    // =============================================================================
    // Private member variables
    // =============================================================================
    SubscriberNode<Interface, ReturnType, Args...>* head; // Pointer to first subscriber in list
    SubscriberNode<Interface, ReturnType, Args...>* tail; // Pointer to last subscriber in list
    int subscriberCount;                                  // Current number of subscribers
};

//**********************************************************************************
// Class: EventManager - Partial specialization for void return type
// Description: Specialized version for void callbacks, includes special sendEvent
//              that returns bool for isDirty-like functionality
//**********************************************************************************
template<typename Interface, typename... Args>
class EventManager<Interface, void, Args...> {
public:
    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    EventManager() : head(nullptr), tail(nullptr), subscriberCount(0) {}

    // -----------------------------------------------------------------------------
    // Destructor
    // -----------------------------------------------------------------------------
    ~EventManager() {
        Clear();
    }

    // -----------------------------------------------------------------------------
    // Delete copy constructor and assignment operator
    // -----------------------------------------------------------------------------
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // -----------------------------------------------------------------------------
    // Method: Subscribe
    // Description: Subscribe an object with a callback method to the event
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - callback: Member function pointer to call
    //   - family: Family ID (0 = receives all events, other values = specific family)
    // -----------------------------------------------------------------------------
    void Subscribe(Interface* subscriber, void (Interface::*callback)(Args...), uint32_t family = 0) {
        if(family == 0xFFFFFFFF) return;
        if (!subscriber || !callback) {
            return;
        }

        // Create a new node for the subscriber
        SubscriberNode<Interface, void, Args...>* newNode =
            new SubscriberNode<Interface, void, Args...>(subscriber, callback, family);

        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }

        subscriberCount++;
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe
    // Description: Remove all subscriptions for a specific subscriber
    // -----------------------------------------------------------------------------
    void Unsubscribe(Interface* subscriber) {
        if (!subscriber || !head) {
            return;
        }

        SubscriberNode<Interface, void, Args...>* current = head;
        SubscriberNode<Interface, void, Args...>* previous = nullptr;

        while (current) {
            if (current->subscriber == subscriber) {
                if (previous) {
                    previous->next = current->next;
                } else {
                    head = current->next;
                }

                if (current == tail) {
                    tail = previous;
                }

                SubscriberNode<Interface, void, Args...>* toDelete = current;
                current = current->next;
                delete toDelete;
                subscriberCount--;
            } else {
                previous = current;
                current = current->next;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Method: SetSubscriberFamily
    // Description: Change the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - newFamily: New family ID to assign
    // Returns: true if subscriber was found and updated, false otherwise
    // -----------------------------------------------------------------------------
    bool SetSubscriberFamily(Interface* subscriber, uint32_t newFamily) {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, void, Args...>* current = head;
        bool found = false;

        while (current) {
            if (current->subscriber == subscriber) {
                current->family = newFamily;
                found = true;
            }
            current = current->next;
        }

        return found;
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberFamily
    // Description: Get the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - outFamily: Reference to store the family ID
    // Returns: true if subscriber was found, false otherwise
    // -----------------------------------------------------------------------------
    bool GetSubscriberFamily(Interface* subscriber, uint32_t& outFamily) const {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, void, Args...>* current = head;

        while (current) {
            if (current->subscriber == subscriber) {
                outFamily = current->family;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent
    // Description: Call all subscribed callbacks matching the specified family
    // Parameters:
    //   - family: Family ID to send to (subscribers with family=0 always receive)
    //   - args: Arguments to pass to the callbacks
    // -----------------------------------------------------------------------------
    inline void sendEvent(uint32_t family, Args... args) {
        SubscriberNode<Interface, void, Args...>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                // Send to subscribers with family=0 (receive all) or matching family
                if (current->family == 0 || current->family == family) {
                    (current->subscriber->*(current->callback))(args...);
                }
            }
            current = current->next;
        }
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToAll
    // Description: Call all subscribed callbacks
    // Parameters:
    //   - args: Arguments to pass to the callbacks
    // -----------------------------------------------------------------------------
    inline void sendEventToAll(Args... args) {
        SubscriberNode<Interface, void, Args...>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                (current->subscriber->*(current->callback))(args...);
            }
            current = current->next;
        }
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount
    // Description: Get the current number of subscribers
    // Parameters:
    //   - family: Optional family filter (if provided, count only that family)
    //   - includeUniversal: If true and family is specified, include family=0 subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        if (family == UINT32_MAX) {
            return subscriberCount;
        }

        int count = 0;
        SubscriberNode<Interface, void, Args...>* current = head;

        while (current) {
            if (current->family == family || (includeUniversal && current->family == 0)) {
                count++;
            }
            current = current->next;
        }

        return count;
    }

    // -----------------------------------------------------------------------------
    // Method: IsEmpty
    // Description: Check if there are no subscribers
    // -----------------------------------------------------------------------------
    bool IsEmpty() const {
        return head == nullptr;
    }

    // -----------------------------------------------------------------------------
    // Method: Clear
    // Description: Remove all subscribers and clean up memory
    // -----------------------------------------------------------------------------
    void Clear() {
        SubscriberNode<Interface, void, Args...>* current = head;

        while (current) {
            SubscriberNode<Interface, void, Args...>* next = current->next;
            delete current;
            current = next;
        }

        head = tail = nullptr;
        subscriberCount = 0;
    }

private:
    // =============================================================================
    // Private member variables
    // =============================================================================
    SubscriberNode<Interface, void, Args...>* head; // Pointer to first subscriber in list
    SubscriberNode<Interface, void, Args...>* tail; // Pointer to last subscriber in list
    int subscriberCount;                            // Current number of subscribers
};

//**********************************************************************************
// Class: EventManager - Specialization for bool return type with void parameter
// Description: Special version for isDirty-like callbacks
//**********************************************************************************
template<typename Interface>
class EventManager<Interface, bool, void> {
public:
    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    EventManager() : head(nullptr), tail(nullptr), subscriberCount(0) {}

    // -----------------------------------------------------------------------------
    // Destructor
    // -----------------------------------------------------------------------------
    ~EventManager() {
        Clear();
    }

    // -----------------------------------------------------------------------------
    // Delete copy constructor and assignment operator
    // -----------------------------------------------------------------------------
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // -----------------------------------------------------------------------------
    // Method: Subscribe
    // Description: Subscribe an object with a callback method to the event
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - callback: Member function pointer to call
    //   - family: Family ID (0 = receives all events, other values = specific family)
    // -----------------------------------------------------------------------------
    void Subscribe(Interface* subscriber, bool (Interface::*callback)(), uint32_t family = 0) {
        if (!subscriber || !callback) {
            return;
        }

        // Create a new node for the subscriber
        SubscriberNode<Interface, bool>* newNode =
            new SubscriberNode<Interface, bool>(subscriber, callback, family);

        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }

        subscriberCount++;
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe
    // Description: Remove all subscriptions for a specific subscriber
    // -----------------------------------------------------------------------------
    void Unsubscribe(Interface* subscriber) {
        if (!subscriber || !head) {
            return;
        }

        SubscriberNode<Interface, bool>* current = head;
        SubscriberNode<Interface, bool>* previous = nullptr;

        while (current) {
            if (current->subscriber == subscriber) {
                if (previous) {
                    previous->next = current->next;
                } else {
                    head = current->next;
                }

                if (current == tail) {
                    tail = previous;
                }

                SubscriberNode<Interface, bool>* toDelete = current;
                current = current->next;
                delete toDelete;
                subscriberCount--;
            } else {
                previous = current;
                current = current->next;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Method: SetSubscriberFamily
    // Description: Change the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - newFamily: New family ID to assign
    // Returns: true if subscriber was found and updated, false otherwise
    // -----------------------------------------------------------------------------
    bool SetSubscriberFamily(Interface* subscriber, uint32_t newFamily) {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, bool>* current = head;
        bool found = false;

        while (current) {
            if (current->subscriber == subscriber) {
                current->family = newFamily;
                found = true;
            }
            current = current->next;
        }

        return found;
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberFamily
    // Description: Get the family of a specific subscriber
    // Parameters:
    //   - subscriber: Pointer to the subscriber object
    //   - outFamily: Reference to store the family ID
    // Returns: true if subscriber was found, false otherwise
    // -----------------------------------------------------------------------------
    bool GetSubscriberFamily(Interface* subscriber, uint32_t& outFamily) const {
        if (!subscriber || !head) {
            return false;
        }

        SubscriberNode<Interface, bool>* current = head;

        while (current) {
            if (current->subscriber == subscriber) {
                outFamily = current->family;
                return true;
            }
            current = current->next;
        }

        return false;
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent
    // Description: Call all subscribed callbacks matching the specified family
    //              and return true if any callback returns true (OR logic)
    // Parameters:
    //   - family: Family ID to send to (subscribers with family=0 always receive)
    // Returns: true if any subscribed callback in the matching family returned true
    // -----------------------------------------------------------------------------
    inline bool sendEvent(uint32_t family) {
        bool result = false;
        SubscriberNode<Interface, bool>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                // Send to subscribers with family=0 (receive all) or matching family
                if (current->family == 0 || current->family == family) {
                    bool callbackResult = (current->subscriber->*(current->callback))();
                    result = result || callbackResult;
                }
            }
            current = current->next;
        }

        return result;
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToAll
    // Description: Call all subscribed callbacks and return true if any returns true
    // Returns: true if any subscribed callback returned true
    // -----------------------------------------------------------------------------
    inline bool sendEventToAll() {
        bool result = false;
        SubscriberNode<Interface, bool>* current = head;

        while (current) {
            if (current->subscriber && current->callback) {
                bool callbackResult = (current->subscriber->*(current->callback))();
                result = result || callbackResult;
            }
            current = current->next;
        }

        return result;
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount
    // Description: Get the current number of subscribers
    // Parameters:
    //   - family: Optional family filter (if provided, count only that family)
    //   - includeUniversal: If true and family is specified, include family=0 subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        if (family == UINT32_MAX) {
            return subscriberCount;
        }

        int count = 0;
        SubscriberNode<Interface, bool>* current = head;

        while (current) {
            if (current->family == family || (includeUniversal && current->family == 0)) {
                count++;
            }
            current = current->next;
        }

        return count;
    }

    // -----------------------------------------------------------------------------
    // Method: IsEmpty
    // Description: Check if there are no subscribers
    // -----------------------------------------------------------------------------
    bool IsEmpty() const {
        return head == nullptr;
    }

    // -----------------------------------------------------------------------------
    // Method: Clear
    // Description: Remove all subscribers and clean up memory
    // -----------------------------------------------------------------------------
    void Clear() {
        SubscriberNode<Interface, bool>* current = head;

        while (current) {
            SubscriberNode<Interface, bool>* next = current->next;
            delete current;
            current = next;
        }

        head = tail = nullptr;
        subscriberCount = 0;
    }

private:
    // =============================================================================
    // Private member variables
    // =============================================================================
    SubscriberNode<Interface, bool>* head; // Pointer to first subscriber in list
    SubscriberNode<Interface, bool>* tail; // Pointer to last subscriber in list
    int subscriberCount;                   // Current number of subscribers
};

} // namespace DadUtilities

//***End of file**************************************************************
