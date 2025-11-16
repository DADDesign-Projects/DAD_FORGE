//==================================================================================
//==================================================================================
// File: ParameterViews.h
// Description: GUI parameter visualization and control classes for audio interface
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include "cParameter.h"
#include <string>
#include <vector>

namespace DadGUI {

//**********************************************************************************
// Class: cParameterInfoView
// Description: Handles temporary display of parameter information (name and value)
//**********************************************************************************
class cParameterInfoView {
public:
    cParameterInfoView() = default;
    virtual ~cParameterInfoView() = default;

    // ---------------------------------------------------------------------------------
    // Function: ShowParamView
    // Description: Display the parameter information on a given layer
    // ---------------------------------------------------------------------------------
    void ShowParamView(DadGFX::cLayer* pLayer, const std::string Name, const std::string Value);

    // ---------------------------------------------------------------------------------
    // Function: HideParamView
    // Description: Hide the parameter information view (reset z-order or clear)
    // ---------------------------------------------------------------------------------
    void HideParamView(DadGFX::cLayer* pLayer);
};

//**********************************************************************************
// Class: cParameterView
// Description: Abstract base class for all parameter visualization components
//**********************************************************************************
class cParameterView {
public:
    virtual ~cParameterView() {}

    // ---------------------------------------------------------------------------------
    // Function: Init
    // Description: Initialize parameter with given attributes
    // ---------------------------------------------------------------------------------
    void Init(DadDSP::cParameter* pParameter, const std::string& ShortName, const std::string& LongName);

    // Return associated parameter pointer
    DadDSP::cParameter* getParameter() { return m_pParameter; }

    // ---------------------------------------------------------------------------------
    // Function: Draw
    // Description: Draw static and dynamic parts of the view (pure virtual)
    // ---------------------------------------------------------------------------------
    virtual void Draw(uint8_t NumParameterArea, DadGFX::cLayer* pStaticLayer, DadGFX::cLayer* pDynamicLayer) = 0;

    // ---------------------------------------------------------------------------------
    // Function: Update
    // Description: Update the view - returns true if parameter value has changed
    // ---------------------------------------------------------------------------------
    virtual bool Update(uint8_t NumParameterArea, DadGFX::cLayer* pDynamicLayer);

    // Return long name for temporary info display
    virtual const std::string getInfoName() { return m_LongName; }

    // Return current value string for temporary info display (pure virtual)
    virtual const std::string getInfoValue() = 0;

protected:
    // Draw the dynamic (value-dependent) part of the view (pure virtual)
    virtual void DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) = 0;

    // Member variables
    std::string            m_ShortName;           // Short parameter name (compact label)
    std::string            m_LongName;            // Long parameter name (info banner)
    DadDSP::cParameter*    m_pParameter = nullptr; // Pointer to the associated parameter
    float                  m_MemParameterValue = 0.0f; // Cached last value for change detection
};

//**********************************************************************************
// Class: cParameterNumView
// Description: Base class for numeric parameter views with formatted value display
//**********************************************************************************
class cParameterNumView : public cParameterView {
public:
    // ---------------------------------------------------------------------------------
    // Function: Init
    // Description: Initialize numeric parameter attributes (names, units, precision)
    // ---------------------------------------------------------------------------------
    void Init(DadDSP::cParameter* pParameter, const std::string& ShortName, const std::string& LongName,
              const std::string& ShortUnit, const std::string& LongUnit, uint8_t StringPrecision = 3);

    // Return formatted value for temporary info banner
    const std::string getInfoValue() override;

protected:
    // Convert internal value to formatted string
    std::string ValueToString() const;

    // Member variables
    std::string    m_ShortUnit;        // Unit short form (shown beside value)
    std::string    m_LongUnit;         // Unit long form (used in info banner)
    uint8_t        m_StringPrecision;  // Decimal precision for textual formatting
};

//**********************************************************************************
// Class: cParameterNumNormalView
// Description: Displays a numeric parameter with a standard centered layout
//**********************************************************************************
class cParameterNumNormalView : public cParameterNumView {
public:
    // Draw static and dynamic layers
    void Draw(uint8_t NumParameterArea, DadGFX::cLayer* pStaticLayer, DadGFX::cLayer* pDynamicLayer) override;

protected:
    // Draw dynamic (value) part of the numeric view
    void DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) override;
};

//**********************************************************************************
// Class: cParameterNumLeftRightView
// Description: Displays a numeric parameter with left/right layout (e.g., stereo balance)
//**********************************************************************************
class cParameterNumLeftRightView : public cParameterNumNormalView {
protected:
    // Draw dynamic left/right style
    void DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) override;
};

//**********************************************************************************
// Struct: sDiscretValues
// Description: Holds a short and long version of a discrete parameter value label
//**********************************************************************************
struct sDiscretValues {
    std::string m_ShortValue;  // Compact display label
    std::string m_LongValue;   // Full descriptive label
};

//**********************************************************************************
// Class: cParameterDiscretView
// Description: Displays a parameter with a set of discrete (enumerated) values
//**********************************************************************************
class cParameterDiscretView : public cParameterView {
public:
    // Add a discrete value option to the table
    void AddDiscreteValue(const std::string& ShortDiscretValue, const std::string& LongDiscretValue);

    // Draw static and dynamic layers
    void Draw(uint8_t NumParameterArea, DadGFX::cLayer* pStaticLayer, DadGFX::cLayer* pDynamicLayer) override;

    // Return current value for temporary info banner
    const std::string getInfoValue() override;

protected:
    // Draw dynamic (value) portion of the view
    void DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pDynamicLayer) override;

    // Member variables
    std::vector<sDiscretValues> m_TabDiscretValues;  // List of discrete values
};

//**********************************************************************************
// Class: cPanelOfParameterView
// Description: Manages a panel composed of three parameter views and their rendering layers
//**********************************************************************************
class cPanelOfParameterView : public iUIComponent {
public:
    virtual ~cPanelOfParameterView() {}

    // ---------------------------------------------------------------------------------
    // Function: Init
    // Description: Initialize the three parameter views and allocate their layers
    // ---------------------------------------------------------------------------------
    void Init(cParameterView* pParameter1, cParameterView* pParameter2, cParameterView* pParameter3);

    // Called when the component becomes active
    void Activate() override;

    // Called when the component becomes inactive
    void Deactivate() override;

    // Called periodically to refresh the panel
    void Update() override;

    // Force to redraw UI component
    void Redraw() override;

protected:
    // Parameter view pointers
    cParameterView*     m_pParameter1 = nullptr;
    cParameterView*     m_pParameter2 = nullptr;
    cParameterView*     m_pParameter3 = nullptr;

    // Info view for displaying current parameter name/value
    cParameterInfoView  m_ParameterInfoView;

    // Layers for each parameter (static and dynamic)
    DadGFX::cLayer*     m_pParameter1LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter1LayerStat = nullptr;
    DadGFX::cLayer*     m_pParameter2LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter2LayerStat = nullptr;
    DadGFX::cLayer*     m_pParameter3LayerDyn = nullptr;
    DadGFX::cLayer*     m_pParameter3LayerStat = nullptr;

    // Info layer for temporary display
    DadGFX::cLayer*     m_pParamInfoLayer = nullptr;

    // Internal counter for info view timing
    uint16_t            m_InfoViewCounter = 0;

    // Memorize active state
    bool                m_isActive = false;
};

} // namespace DadGUI
//***End of file**************************************************************
