#pragma once
//==================================================================================
//==================================================================================
// File: GUI_Event.h
// Description: Event management system for GUI components with real-time processing
//
// Copyright (c) 2026 Dad Design.
//==================================================================================
//==================================================================================

#include "main.h"
#include "AudioManager.h"
#include "EventManager.h"
#include "Serialize.h"

namespace DadGUI {

//**********************************************************************************
// Class: iGUI_EventListener
// Description: Abstract interface for objects that require real-time processing
//**********************************************************************************
class iGUI_EventListener {
public:
	virtual ~iGUI_EventListener() {};
    // -----------------------------------------------------------------------------
    // Interface
    // -----------------------------------------------------------------------------

    // audio thread frequency RT_RATE in Hz
    // audio thread period RT_TIME in Second
    // Real-time processing method called in audio thread
    virtual void on_GUI_RT_Process(){};

    // Process audio buffer through GUI object before audio process
    virtual void on_GUI_RT_ProcessIn(AudioBuffer *pIn){};

    // Process audio buffer through GUI object after audio process
    virtual void on_GUI_RT_ProcessOut(AudioBuffer *pOut){};

    // Update GUI Object time GUI_UPDATE_MS in __ms__
    virtual void on_GUI_Update(){};

    // Fast update GUI Object time GUI_FAST_UPDATE_MS in __ms__
    virtual void on_GUI_FastUpdate(){};

};

//**********************************************************************************
// Class: GUI_EventManager
// Description: Manages all GUI event subscriptions and dispatching
//**********************************************************************************
class GUI_EventManager {
public:
    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    GUI_EventManager() = default;

    // -----------------------------------------------------------------------------
    // Destructor
    // -----------------------------------------------------------------------------
    ~GUI_EventManager() = default;

    // -----------------------------------------------------------------------------
    // Delete copy constructor and assignment operator
    // -----------------------------------------------------------------------------
    GUI_EventManager(const GUI_EventManager&) = delete;
    GUI_EventManager& operator=(const GUI_EventManager&) = delete;

    // -----------------------------------------------------------------------------
    // GUI Event Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: Subscribe_RT_Process
    // Description: Subscribe to real-time processing events
    // Parameters:
    //   - listener: Pointer to the listener object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_RT_Process(iGUI_EventListener* listener, uint32_t family = 0) {
        m_rtProcessManager.Subscribe(listener, &iGUI_EventListener::on_GUI_RT_Process, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_RT_ProcessIn
    // Description: Subscribe to pre-audio processing events
    // Parameters:
    //   - listener: Pointer to the listener object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_RT_ProcessIn(iGUI_EventListener* listener, uint32_t family = 0) {
        m_rtProcessInManager.Subscribe(listener, &iGUI_EventListener::on_GUI_RT_ProcessIn, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_RT_ProcessOut
    // Description: Subscribe to post-audio processing events
    // Parameters:
    //   - listener: Pointer to the listener object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_RT_ProcessOut(iGUI_EventListener* listener, uint32_t family = 0) {
        m_rtProcessOutManager.Subscribe(listener, &iGUI_EventListener::on_GUI_RT_ProcessOut, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_Update
    // Description: Subscribe to regular GUI update events
    // Parameters:
    //   - listener: Pointer to the listener object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_Update(iGUI_EventListener* listener, uint32_t family = 0) {
        m_updateManager.Subscribe(listener, &iGUI_EventListener::on_GUI_Update, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_FastUpdate
    // Description: Subscribe to fast GUI update events
    // Parameters:
    //   - listener: Pointer to the listener object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_FastUpdate(iGUI_EventListener* listener, uint32_t family = 0) {
        m_fastUpdateManager.Subscribe(listener, &iGUI_EventListener::on_GUI_FastUpdate, family);
    }

    // -----------------------------------------------------------------------------
    // Serialization Event Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: Subscribe_SerializeSave
    // Description: Subscribe to serialization save events
    // Parameters:
    //   - serializable: Pointer to serializable object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_SerializeSave(DadPersistentStorage::cSerializedObject* serializable, uint32_t family = 0) {
        m_SerializeSaveManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::Save, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_SerializeRestore
    // Description: Subscribe to serialization restore events
    // Parameters:
    //   - serializable: Pointer to serializable object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_SerializeRestore(DadPersistentStorage::cSerializedObject* serializable, uint32_t family = 0) {
        m_SerializeRestoreManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::Restore, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_SerializeIsDirty
    // Description: Subscribe to serialization dirty check events
    // Parameters:
    //   - serializable: Pointer to serializable object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_SerializeIsDirty(DadPersistentStorage::cSerializedObject* serializable, uint32_t family = 0) {
        m_SerializeisDirtyManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::isDirty, family);
    }

    // -----------------------------------------------------------------------------
    // Method: Subscribe_AllSerializeEvents
    // Description: Subscribe to all serialization events (Save, Restore, IsDirty)
    // Parameters:
    //   - serializable: Pointer to serializable object
    //   - family: Family ID (0 = receives all events, default)
    // -----------------------------------------------------------------------------
    inline void Subscribe_AllSerializeEvents(DadPersistentStorage::cSerializedObject* serializable, uint32_t family = 0) {
        m_SerializeSaveManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::Save, family);
        m_SerializeRestoreManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::Restore, family);
        m_SerializeisDirtyManager.Subscribe(serializable, &DadPersistentStorage::cSerializedObject::isDirty, family);
    }

    // -----------------------------------------------------------------------------
    // GUI Unsubscribe Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_RT_Process
    // Description: Unsubscribe from real-time processing events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_RT_Process(iGUI_EventListener* listener) {
        m_rtProcessManager.Unsubscribe(listener);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_RT_ProcessIn
    // Description: Unsubscribe from pre-audio processing events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_RT_ProcessIn(iGUI_EventListener* listener) {
        m_rtProcessInManager.Unsubscribe(listener);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_RT_ProcessOut
    // Description: Unsubscribe from post-audio processing events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_RT_ProcessOut(iGUI_EventListener* listener) {
        m_rtProcessOutManager.Unsubscribe(listener);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_Update
    // Description: Unsubscribe from regular GUI update events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_Update(iGUI_EventListener* listener) {
        m_updateManager.Unsubscribe(listener);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_FastUpdate
    // Description: Unsubscribe from fast GUI update events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_FastUpdate(iGUI_EventListener* listener) {
        m_fastUpdateManager.Unsubscribe(listener);
    }

    // -----------------------------------------------------------------------------
    // Serialization Unsubscribe Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_SerializeSave
    // Description: Unsubscribe from serialization save events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_SerializeSave(DadPersistentStorage::cSerializedObject* serializable) {
        m_SerializeSaveManager.Unsubscribe(serializable);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_SerializeRestore
    // Description: Unsubscribe from serialization restore events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_SerializeRestore(DadPersistentStorage::cSerializedObject* serializable) {
        m_SerializeRestoreManager.Unsubscribe(serializable);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_SerializeIsDirty
    // Description: Unsubscribe from serialization dirty check events
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_SerializeIsDirty(DadPersistentStorage::cSerializedObject* serializable) {
        m_SerializeisDirtyManager.Unsubscribe(serializable);
    }

    // -----------------------------------------------------------------------------
    // Method: Unsubscribe_AllSerializeEvents
    // Description: Unsubscribe from all serialization events (Save, Restore, IsDirty)
    // Parameters:
    //   - serializable: Pointer to serializable object
    // -----------------------------------------------------------------------------
    inline void Unsubscribe_AllSerializeEvents(DadPersistentStorage::cSerializedObject* serializable) {
        m_SerializeSaveManager.Unsubscribe(serializable);
        m_SerializeRestoreManager.Unsubscribe(serializable);
        m_SerializeisDirtyManager.Unsubscribe(serializable);
    }
    // -----------------------------------------------------------------------------
    // GUI Family Change Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: SetFamily_RT_Process
    // Description: Change family for real-time processing events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_RT_Process(iGUI_EventListener* listener, uint32_t newFamily) {
        return m_rtProcessManager.SetSubscriberFamily(listener, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_RT_ProcessIn
    // Description: Change family for pre-audio processing events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_RT_ProcessIn(iGUI_EventListener* listener, uint32_t newFamily) {
        return m_rtProcessInManager.SetSubscriberFamily(listener, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_RT_ProcessOut
    // Description: Change family for post-audio processing events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_RT_ProcessOut(iGUI_EventListener* listener, uint32_t newFamily) {
        return m_rtProcessOutManager.SetSubscriberFamily(listener, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_Update
    // Description: Change family for regular GUI update events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_Update(iGUI_EventListener* listener, uint32_t newFamily) {
        return m_updateManager.SetSubscriberFamily(listener, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_FastUpdate
    // Description: Change family for fast GUI update events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_FastUpdate(iGUI_EventListener* listener, uint32_t newFamily) {
        return m_fastUpdateManager.SetSubscriberFamily(listener, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Serialization Family Change Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: SetFamily_SerializeSave
    // Description: Change family for serialization save events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_SerializeSave(DadPersistentStorage::cSerializedObject* serializable, uint32_t newFamily) {
        return m_SerializeSaveManager.SetSubscriberFamily(serializable, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_SerializeRestore
    // Description: Change family for serialization restore events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_SerializeRestore(DadPersistentStorage::cSerializedObject* serializable, uint32_t newFamily) {
        return m_SerializeRestoreManager.SetSubscriberFamily(serializable, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Method: SetFamily_SerializeIsDirty
    // Description: Change family for serialization dirty check events
    // -----------------------------------------------------------------------------
    inline bool SetFamily_SerializeIsDirty(DadPersistentStorage::cSerializedObject* serializable, uint32_t newFamily) {
        return m_SerializeisDirtyManager.SetSubscriberFamily(serializable, newFamily);
    }

    // -----------------------------------------------------------------------------
    // Active Family Setter Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_RT_Process
    // Description: Set active family for RT Process events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_RT_Process(uint32_t family) {
        m_activeFamilyRTProcess = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_RT_ProcessIn
    // Description: Set active family for RT Process In events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_RT_ProcessIn(uint32_t family) {
        m_activeFamilyRTProcessIn = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_RT_ProcessOut
    // Description: Set active family for RT Process Out events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_RT_ProcessOut(uint32_t family) {
        m_activeFamilyRTProcessOut = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_Update
    // Description: Set active family for Update events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_Update(uint32_t family) {
        m_activeFamilyUpdate = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_FastUpdate
    // Description: Set active family for Fast Update events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_FastUpdate(uint32_t family) {
        m_activeFamilyFastUpdate = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_SerializeSave
    // Description: Set active family for Serialize Save events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_SerializeSave(uint32_t family) {
        m_activeFamilySerializeSave = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_SerializeRestore
    // Description: Set active family for Serialize Restore events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_SerializeRestore(uint32_t family) {
        m_activeFamilySerializeRestore = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily_SerializeIsDirty
    // Description: Set active family for Serialize IsDirty events
    // Parameters:
    //   - family: Family ID to set as active
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily_SerializeIsDirty(uint32_t family) {
        m_activeFamilySerializeIsDirty = family;
    }

    // -----------------------------------------------------------------------------
    // Method: SetActiveFamily4AllEvents
    // Description: Set active family for ALL event types simultaneously
    // Parameters:
    //   - family: Family ID to set as active for all event types
    // -----------------------------------------------------------------------------
    inline void SetActiveFamily4AllEvents(uint32_t family) {
        m_activeFamilyRTProcess = family;
        m_activeFamilyRTProcessIn = family;
        m_activeFamilyRTProcessOut = family;
        m_activeFamilyUpdate = family;
        m_activeFamilyFastUpdate = family;
        m_activeFamilySerializeSave = family;
        m_activeFamilySerializeRestore = family;
        m_activeFamilySerializeIsDirty = family;
    }

    // -----------------------------------------------------------------------------
    // Active Family Getter Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_RT_Process
    // Description: Get active family for RT Process events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_RT_Process() const {
        return m_activeFamilyRTProcess;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_RT_ProcessIn
    // Description: Get active family for RT Process In events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_RT_ProcessIn() const {
        return m_activeFamilyRTProcessIn;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_RT_ProcessOut
    // Description: Get active family for RT Process Out events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_RT_ProcessOut() const {
        return m_activeFamilyRTProcessOut;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_Update
    // Description: Get active family for Update events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_Update() const {
        return m_activeFamilyUpdate;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_FastUpdate
    // Description: Get active family for Fast Update events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_FastUpdate() const {
        return m_activeFamilyFastUpdate;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_SerializeSave
    // Description: Get active family for Serialize Save events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_SerializeSave() const {
        return m_activeFamilySerializeSave;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_SerializeRestore
    // Description: Get active family for Serialize Restore events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_SerializeRestore() const {
        return m_activeFamilySerializeRestore;
    }

    // -----------------------------------------------------------------------------
    // Method: GetActiveFamily_SerializeIsDirty
    // Description: Get active family for Serialize IsDirty events
    // Returns: Current active family ID
    // -----------------------------------------------------------------------------
    inline uint32_t GetActiveFamily_SerializeIsDirty() const {
        return m_activeFamilySerializeIsDirty;
    }

    // -----------------------------------------------------------------------------
    // GUI Event Dispatch Methods (to specific family)
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: sendEvent_RT_Process
    // Description: send event real-time processing for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    // -----------------------------------------------------------------------------
    inline void sendEvent_RT_Process(uint32_t family = 0) {
        m_rtProcessManager.sendEvent(family);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_RT_ProcessIn
    // Description: send event pre-audio processing for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    //   - pIn: Pointer to input audio buffer
    // -----------------------------------------------------------------------------
    inline void sendEvent_RT_ProcessIn(uint32_t family, AudioBuffer* pIn) {
        m_rtProcessInManager.sendEvent(family, pIn);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_RT_ProcessOut
    // Description: send event post-audio processing for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    //   - pOut: Pointer to output audio buffer
    // -----------------------------------------------------------------------------
    inline void sendEvent_RT_ProcessOut(uint32_t family, AudioBuffer* pOut) {
        m_rtProcessOutManager.sendEvent(family, pOut);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_Update
    // Description: send event regular GUI update for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    // -----------------------------------------------------------------------------
    inline void sendEvent_Update(uint32_t family = 0) {
        m_updateManager.sendEvent(family);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_FastUpdate
    // Description: send event fast GUI update for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    // -----------------------------------------------------------------------------
    inline void sendEvent_FastUpdate(uint32_t family = 0) {
        m_fastUpdateManager.sendEvent(family);
    }

    // -----------------------------------------------------------------------------
    // Serialization Event Dispatch Methods (to specific family)
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: sendEvent_SerializeSave
    // Description: Send serialization save event for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    //   - pSerializer: Pointer to serializer object
    // -----------------------------------------------------------------------------
    inline void sendEvent_SerializeSave(uint32_t family, DadPersistentStorage::cSerialize* pSerializer) {
        m_SerializeSaveManager.sendEvent(family, pSerializer);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_SerializeRestore
    // Description: Send serialization restore event for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    //   - pSerializer: Pointer to serializer object
    // -----------------------------------------------------------------------------
    inline void sendEvent_SerializeRestore(uint32_t family, DadPersistentStorage::cSerialize* pSerializer) {
        m_SerializeRestoreManager.sendEvent(family, pSerializer);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEvent_SerializeIsDirty
    // Description: Send serialization dirty check event for specified family
    // Parameters:
    //   - family: Family ID to send event (0 send all)
    // Returns: Combined result (true if any subscribed object is dirty)
    // -----------------------------------------------------------------------------
    inline bool sendEvent_SerializeIsDirty(uint32_t family = 0) {
        return m_SerializeisDirtyManager.sendEvent(family);
    }

    // -----------------------------------------------------------------------------
    // Active Family Event Dispatch Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_RT_Process
    // Description: Send RT Process event to active family
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_RT_Process() {
        m_rtProcessManager.sendEvent(m_activeFamilyRTProcess);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_RT_ProcessIn
    // Description: Send RT Process In event to active family
    // Parameters:
    //   - pIn: Pointer to input audio buffer
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_RT_ProcessIn(AudioBuffer* pIn) {
        m_rtProcessInManager.sendEvent(m_activeFamilyRTProcessIn, pIn);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_RT_ProcessOut
    // Description: Send RT Process Out event to active family
    // Parameters:
    //   - pOut: Pointer to output audio buffer
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_RT_ProcessOut(AudioBuffer* pOut) {
        m_rtProcessOutManager.sendEvent(m_activeFamilyRTProcessOut, pOut);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_Update
    // Description: Send Update event to active family
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_Update() {
        m_updateManager.sendEvent(m_activeFamilyUpdate);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_FastUpdate
    // Description: Send Fast Update event to active family
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_FastUpdate() {
        m_fastUpdateManager.sendEvent(m_activeFamilyFastUpdate);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_SerializeSave
    // Description: Send Serialize Save event to active family
    // Parameters:
    //   - pSerializer: Pointer to serializer object
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_SerializeSave(DadPersistentStorage::cSerialize* pSerializer) {
        m_SerializeSaveManager.sendEvent(m_activeFamilySerializeSave, pSerializer);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_SerializeRestore
    // Description: Send Serialize Restore event to active family
    // Parameters:
    //   - pSerializer: Pointer to serializer object
    // -----------------------------------------------------------------------------
    inline void sendEventToActive_SerializeRestore(DadPersistentStorage::cSerialize* pSerializer) {
        m_SerializeRestoreManager.sendEvent(m_activeFamilySerializeRestore, pSerializer);
    }

    // -----------------------------------------------------------------------------
    // Method: sendEventToActive_SerializeIsDirty
    // Description: Send Serialize IsDirty event to active family
    // Returns: Combined result (true if any subscribed object in active family is dirty)
    // -----------------------------------------------------------------------------
    inline bool sendEventToActive_SerializeIsDirty() {
        return m_SerializeisDirtyManager.sendEvent(m_activeFamilySerializeIsDirty);
    }

    // -----------------------------------------------------------------------------
    // GUI Subscriber Count Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_RT_Process
    // Description: Get number of RT process subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_RT_Process(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_rtProcessManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_RT_ProcessIn
    // Description: Get number of RT process in subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_RT_ProcessIn(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_rtProcessInManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_RT_ProcessOut
    // Description: Get number of RT process out subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_RT_ProcessOut(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_rtProcessOutManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_Update
    // Description: Get number of update subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_Update(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_updateManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_FastUpdate
    // Description: Get number of fast update subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_FastUpdate(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_fastUpdateManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Serialization Subscriber Count Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_SerializeSave
    // Description: Get number of serialization save subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_SerializeSave(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_SerializeSaveManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_SerializeRestore
    // Description: Get number of serialization restore subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_SerializeRestore(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_SerializeRestoreManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: GetSubscriberCount_SerializeIsDirty
    // Description: Get number of serialization dirty check subscribers
    // -----------------------------------------------------------------------------
    int GetSubscriberCount_SerializeIsDirty(uint32_t family = UINT32_MAX, bool includeUniversal = false) const {
        return m_SerializeisDirtyManager.GetSubscriberCount(family, includeUniversal);
    }

    // -----------------------------------------------------------------------------
    // Method: Clear
    // Description: Clear all event subscriptions and reset active families
    // -----------------------------------------------------------------------------
    void Clear() {
        m_rtProcessManager.Clear();
        m_rtProcessInManager.Clear();
        m_rtProcessOutManager.Clear();
        m_updateManager.Clear();
        m_fastUpdateManager.Clear();
        m_SerializeSaveManager.Clear();
        m_SerializeRestoreManager.Clear();
        m_SerializeisDirtyManager.Clear();

        // Reset all active families to 0
        m_activeFamilyRTProcess = 0;
        m_activeFamilyRTProcessIn = 0;
        m_activeFamilyRTProcessOut = 0;
        m_activeFamilyUpdate = 0;
        m_activeFamilyFastUpdate = 0;
        m_activeFamilySerializeSave = 0;
        m_activeFamilySerializeRestore = 0;
        m_activeFamilySerializeIsDirty = 0;
    }

protected:
    // =============================================================================
    // Protected member variables - Event managers for each event type
    // =============================================================================

    DadUtilities::EventManager<iGUI_EventListener, void> 						m_rtProcessManager;
    DadUtilities::EventManager<iGUI_EventListener, void, AudioBuffer*> 			m_rtProcessInManager;
    DadUtilities::EventManager<iGUI_EventListener, void, AudioBuffer*> 			m_rtProcessOutManager;
    DadUtilities::EventManager<iGUI_EventListener, void> 						m_updateManager;
    DadUtilities::EventManager<iGUI_EventListener, void> 						m_fastUpdateManager;
    DadUtilities::EventManager<DadPersistentStorage::cSerializedObject, void, DadPersistentStorage::cSerialize*> m_SerializeSaveManager;
    DadUtilities::EventManager<DadPersistentStorage::cSerializedObject, void, DadPersistentStorage::cSerialize*> m_SerializeRestoreManager;
    DadUtilities::EventManager<DadPersistentStorage::cSerializedObject, bool, void> m_SerializeisDirtyManager;

    // =============================================================================
    // Active family variables for each event type
    // =============================================================================
    uint32_t m_activeFamilyRTProcess = 0;
    uint32_t m_activeFamilyRTProcessIn = 0;
    uint32_t m_activeFamilyRTProcessOut = 0;
    uint32_t m_activeFamilyUpdate = 0;
    uint32_t m_activeFamilyFastUpdate = 0;
    uint32_t m_activeFamilySerializeSave = 0;
    uint32_t m_activeFamilySerializeRestore = 0;
    uint32_t m_activeFamilySerializeIsDirty = 0;
};

} // namespace DadGUI

//***End of file**************************************************************
