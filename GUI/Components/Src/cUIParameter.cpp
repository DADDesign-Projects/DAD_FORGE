//==================================================================================
//==================================================================================
// File: cUIParameter.cpp
// Description: Implementation of GUI-integrated parameter system
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUIParameter.h"
#include "HardwareAndCo.h"
#include "GUI_Define.h"

namespace DadGUI {

//**********************************************************************************
// Class: cUIParameter
// Description: Extends base parameter system with GUI integration capabilities
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Public Methods
// ---------------------------------------------------------------------------------

//***********************************************************************************
// Method: Init
// Description:
// Initializes parameter with configuration and registers with GUI systems
//***********************************************************************************
void cUIParameter::Init(uint32_t SerializeID,
                        float InitValue, float Min, float Max,
                        float RapidIncrement, float SlowIncrement,
                        DadDSP::CallbackType Callback,
                        uint32_t CallbackUserData,
                        float SlopeTime,
                        uint8_t Control, bool RTProcess)
{
    // Initialize base parameter functionality
	m_DrawInfoView = true;
    m_pParentView = nullptr;

	float Slope;
	if(RTProcess){
		Slope = SlopeTime * SAMPLING_RATE;
	}else{
		Slope = SlopeTime * 1000.0f / (float) GUI_FAST_UPDATE_MS;
	}
    cParameter::Init(InitValue, Min, Max, RapidIncrement, SlowIncrement,
                     Callback, CallbackUserData, Slope, Control);

    m_MemUIParameterValue = m_TargetValue;

    // Register parameter with GUI serialization and real-time processing systems
    __GUI_EventManager.Subscribe_AllSerializeEvents(this, SerializeID);
    __GUI_EventManager.Subscribe_Update(this, SerializeID);
	if(RTProcess){
		__GUI_EventManager.Subscribe_RT_Process(this, SerializeID);
	}else{
		__GUI_EventManager.Subscribe_FastUpdate(this, SerializeID);
	}
}

//***********************************************************************************
// Method: Save
// Description:
// Serializes parameter target value to persistent storage
//***********************************************************************************
void cUIParameter::Save(DadPersistentStorage::cSerialize* pSerializer){
    m_Dirty = false;                    // Reset dirty flag after save
    pSerializer->Push(m_TargetValue);   // Write target value to storage
}

//***********************************************************************************
// Method: Restore
// Description:
// Deserializes parameter value from storage and applies it
//***********************************************************************************
void cUIParameter::Restore(DadPersistentStorage::cSerialize* pSerializer){
    m_Dirty = false;                    // Reset dirty flag after restore
    float Value;                        // Temporary storage for retrieved value
    pSerializer->Pull(Value);           // Read value from storage
    if(Value != m_TargetValue){         // If value changed since last restore)
    	setValue(Value);                // Apply the restored value

    	// Save the value to prevent the parameterInfoView from displaying upon restore.
    	m_MemUIParameterValue = m_TargetValue;
    }
}

//***********************************************************************************
// Method: isDirty
// Description:
// Checks if parameter has been modified since last save
//***********************************************************************************
bool cUIParameter::isDirty(){
    return m_Dirty;                     // Return modification status
}

//***********************************************************************************
// Method: on_GUI_Update
// Description:
// Update Parameter info view
//***********************************************************************************
void cUIParameter::on_GUI_Update(){
    // Detect changes (MIDI, memory restore)
    if ((m_MemUIParameterValue != m_TargetValue) && (m_pParentView != nullptr)) {
        m_MemUIParameterValue = m_TargetValue;
        // Redraw dynamic part to reflect new value
        if(true == m_DrawInfoView){
        	__GUI.NotifyParamChange(m_pParentView);
        }
    }

}

//***********************************************************************************
// Method: on_GUI_FastUpdate
// Description:
// Performs processing including value interpolation and callback
//***********************************************************************************
void cUIParameter::on_GUI_FastUpdate(){
	Process();
}

//***********************************************************************************
// Method: on_GUI_RT_Process
// Description:
// Performs real-time processing including value interpolation and callback
//***********************************************************************************
void cUIParameter::on_GUI_RT_Process(){
	Process();
}

} // namespace DadGUI

//***End of file**************************************************************
