//==================================================================================
//==================================================================================
// File: ParameterViews.h
// Description: GUI parameters views
//
// Copyright (c) 2025-2026 Dad Design.
//==================================================================================
//==================================================================================
#pragma once

#include "GUI_Include.h"
#include "cUIParameter.h"
#include <string>
#include <vector>

namespace DadGUI {

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
    void Init(cUIParameter* pParameter, const std::string& ShortName, const std::string& LongName);

    // Return associated parameter pointer for external access
    cUIParameter* getParameter() { return m_pParameter; }

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
    std::string            	m_ShortName;           			// Short parameter name (compact label)
    std::string            	m_LongName;            			// Long parameter name (info banner)
    cUIParameter*   		m_pParameter = nullptr; 		// Pointer to the associated parameter
    float                  	m_MemParameterValue = 0.0f; 	// Cached last value for change detection
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
    void Init(cUIParameter* pParameter, const std::string& ShortName, const std::string& LongName,
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

} // namespace DadGUI
//***End of file**************************************************************
