//==================================================================================
//==================================================================================
// File: cUIParameter.h
// Description: GUI-integrated parameter system with serialization and real-time processing
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "GUI_Event.h"
#include "Serialize.h"
#include "cParameter.h"

namespace DadGUI {
class cParameterView;

//**********************************************************************************
// Class: cUIParameter
// Description:
// Extends base DSP parameter system with GUI integration, supporting serialization,
// deserialization, and real-time interpolation of parameter values
//**********************************************************************************
class cUIParameter
    : public iGUI_EventListener,
      public DadPersistentStorage::cSerializedObject,
      public DadDSP::cParameter
{
public:
    // -----------------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------------

    //***********************************************************************************
    // Method: Init
    // Description:
    // Initializes parameter with configuration and registers with GUI systems
    //***********************************************************************************
    void Init(uint32_t SerializeID,
              float InitValue,
              float Min,
              float Max,
              float RapidIncrement,
              float SlowIncrement,
              DadDSP::CallbackType Callback = nullptr,
              uint32_t CallbackUserData = 0,
              float SlopeTime = 0,
              uint8_t Control = 0xFF,
			  bool RTProcess = false);

    //***********************************************************************************
    // Method: Save
    // Description:
    // Serializes parameter target value to persistent storage
    //***********************************************************************************
    void Save(DadPersistentStorage::cSerialize* pSerializer) override;

    //***********************************************************************************
    // Method: Restore
    // Description:
    // Restores parameter value from storage and applies it
    //***********************************************************************************
    void Restore(DadPersistentStorage::cSerialize* pSerializer) override;

    //***********************************************************************************
    // Method: isDirty
    // Description:
    // Checks if parameter has been modified since last synchronization
    //***********************************************************************************
    bool isDirty() override;

    //***********************************************************************************
    // Method: setDrawInfoView
    // Description:
    // set DrawInfoView flag
    //***********************************************************************************
    inline void setDrawInfoView(){
    	m_DrawInfoView = true;              // reset restored flag
    }

    //***********************************************************************************
    // Method: resetDrawInfoView
    // Description:
    // reset DrawInfoView flag
    //***********************************************************************************
    inline void resetDrawInfoView(){
    	m_DrawInfoView = false;              // reset restored flag
    }

    //***********************************************************************************
    // Method: RtProcess
    // Description:
    // Performs fast value updates and smoothing between current and target values
    //***********************************************************************************
    void on_GUI_FastUpdate() override;

    //***********************************************************************************
    // Method: on_GUI_RT_Process
    // Description:
    // Performs real-time processing including value interpolation and callback
    //***********************************************************************************
    void on_GUI_RT_Process() override;

    //***********************************************************************************
    // Method: on_GUI_Update
    // Description:
    // Update Parameter info view
    //***********************************************************************************
    void on_GUI_Update();

    //***********************************************************************************
    // Method: setParentView
    // Description:
    // Define the view that contains the UIparameter.
    //***********************************************************************************
    inline void setParentView(cParameterView * pParentView){
    	m_pParentView = pParentView;
    }

protected :
    cParameterView*  m_pParentView;	      // Parent View
    float			 m_MemUIParameterValue;
    bool 		  	 m_DrawInfoView;	  // Restored flag
    bool		  	 m_IsRTProcess;		  // Process parameter in audio thread
};

} // namespace DadGUI

//***End of file**************************************************************
