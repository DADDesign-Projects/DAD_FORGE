//==================================================================================
//==================================================================================
// File: ParameterViews.cpp
// Description: Implementation of graphical parameter view system for audio interface
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "ParameterViews.h"
#include "HardwareAndCo.h"

namespace DadGUI {

//**********************************************************************************
// Constants and utility functions
//**********************************************************************************
constexpr float PI = 3.14159265358979f;
constexpr float Deg2Rad(float a) { return a * PI / 180.0f; }

// Parameters for potentiometer graphical representation
#define PARAM_POT_RADIUS 28
#define PARAM_POT_ALPHA_MIN 30
#define PARAM_POT_ALPHA_MAX 360 - PARAM_POT_ALPHA_MIN
#define PARAM_POT_ALPHA PARAM_POT_ALPHA_MAX - PARAM_POT_ALPHA_MIN

// Parameters for potentiometer discrete graphical representation
#define PARAM_DISCRET_RADIUS 5
#define PARAM_DISCRET_POT_RADIUS 26

//**********************************************************************************
// cParameterView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the parameter view with parameter pointer and names
// ---------------------------------------------------------------------------------
void cParameterView::Init(DadDSP::cParameter* pParameter, const std::string& ShortName, const std::string& LongName) {
    m_pParameter = pParameter;     // Backend parameter object
    m_ShortName = ShortName;       // Short display name
    m_LongName  = LongName;        // Long descriptive name

    // Cache initial target value for change detection
    m_MemParameterValue = m_pParameter->getTargetValue();
}

// ---------------------------------------------------------------------------------
// Function: Update
// Description: Read encoder input, update parameter if needed, and redraw dynamic view
// ---------------------------------------------------------------------------------
bool cParameterView::Update(uint8_t NumParameterArea, DadGFX::cLayer* pDynamicLayer) {
    // Select appropriate encoder based on parameter area
    DadDrivers::cEncoder* pEncoder = (NumParameterArea == 1) ? &__Encoder1 :
                                     (NumParameterArea == 2) ? &__Encoder2 : &__Encoder3;

    uint8_t SwitchState = pEncoder->getSwitchState(); // Get encoder switch state
    int8_t Increment = pEncoder->getIncrement();      // Get encoder rotation

    // Process encoder rotation if any
    if (Increment != 0) {
        // User turned encoder -> increment parameter (respecting switch state)
        m_pParameter->Increment(Increment, SwitchState);
    }

    // Detect external changes (e.g., MIDI) or local change
    if (m_MemParameterValue != m_pParameter->getTargetValue()) {
        m_MemParameterValue = m_pParameter->getTargetValue();

        // Redraw dynamic part to reflect new value
        DrawDynView(NumParameterArea, pDynamicLayer);

        // If GUI restore is in progress, do not notify as changed
        if (__GUI.IsRestoreInProcess()) {
            return false;
        }
        return true; // Parameter value changed
    }

    return false; // No change detected
}

//**********************************************************************************
// cParameterNumView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initialize numeric parameter attributes (names, units, precision)
// ---------------------------------------------------------------------------------
void cParameterNumView::Init(DadDSP::cParameter* pParameter, const std::string& ShortName, const std::string& LongName,
                             const std::string& ShortUnit, const std::string& LongUnit, uint8_t StringPrecision) {
    cParameterView::Init(pParameter, ShortName, LongName);
    m_ShortUnit = ShortUnit;        // Short unit display
    m_LongUnit  = LongUnit;         // Long unit description
    m_StringPrecision = StringPrecision; // Decimal precision for display
}

// ---------------------------------------------------------------------------------
// Function: ValueToString
// Description: Converts the parameter target value to a formatted string
// ---------------------------------------------------------------------------------
std::string cParameterNumView::ValueToString() const {
    const int MAX_SIZE = 32; // Buffer safety limit
    char buffer[MAX_SIZE];
    int index = 0;

    float value = m_pParameter->getTargetValue();

    // Clamp precision to sensible bounds
    int precision = static_cast<int>(m_StringPrecision);
    if (precision < 0) precision = 0;
    if (precision > MAX_SIZE - 5) precision = MAX_SIZE - 5;

    // Handle negative values
    if (value < 0.0f) {
        buffer[index++] = '-';
        value = -value;
    }

    // Handle zero or extremely small values explicitly
    if (value < 0.0000001f) {
        buffer[index++] = '0';
        int decimalDigits = precision - 1;
        if (decimalDigits > 0) {
            buffer[index++] = '.';
            for (int i = 0; i < decimalDigits && index < MAX_SIZE - 1; ++i) {
                buffer[index++] = '0';
            }
        }
        buffer[index] = '\0';
        return std::string(buffer);
    }

    // Extract integer and fractional parts
    int integerPart = static_cast<int>(value);
    float fractionalPart = value - static_cast<float>(integerPart);

    // Integer digits counting and conversion
    if (integerPart == 0) {
        buffer[index++] = '0';
    } else {
        int startIndex = index;
        int temp = integerPart;
        while (temp > 0 && index < MAX_SIZE - precision - 2) {
            buffer[index++] = static_cast<char>((temp % 10) + '0');
            temp /= 10;
        }
        // Reverse the integer digits
        int endIndex = index - 1;
        while (startIndex < endIndex) {
            char swap = buffer[startIndex];
            buffer[startIndex] = buffer[endIndex];
            buffer[endIndex] = swap;
            startIndex++;
            endIndex--;
        }
    }

    // Compute how many decimal digits to show
    int integerDigits = (integerPart == 0) ? 1 : 0;
    if (integerPart != 0) {
        int tmp = integerPart;
        while (tmp > 0) {
            integerDigits++;
            tmp /= 10;
        }
    }

    int decimalDigits = precision - integerDigits;
    if (decimalDigits < 0) decimalDigits = 0;
    if (decimalDigits > precision) decimalDigits = precision;

    // Add fractional digits if requested
    if (decimalDigits > 0 && index < MAX_SIZE - 1) {
        buffer[index++] = '.';
        for (int i = 0; i < decimalDigits && index < MAX_SIZE - 1; ++i) {
            fractionalPart *= 10.0f;
            int digit = static_cast<int>(fractionalPart);
            if (digit < 0) digit = 0;
            if (digit > 9) digit = 9;
            buffer[index++] = static_cast<char>(digit + '0');
            fractionalPart -= static_cast<float>(digit);
        }
    }

    buffer[index] = '\0';
    return std::string(buffer);
}

// ---------------------------------------------------------------------------------
// Function: getInfoValue
// Description: Return formatted value for temporary info banner
// ---------------------------------------------------------------------------------
const std::string cParameterNumView::getInfoValue() {
    return ValueToString() + " " + m_LongUnit;
}

//**********************************************************************************
// cParameterNumNormalView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: Draw
// Description: Draw static layout and initial static decoration
// ---------------------------------------------------------------------------------
void cParameterNumNormalView::Draw(uint8_t NumParameterArea, DadGFX::cLayer* pStaticLayer, DadGFX::cLayer* pDynamicLayer) {
    const uint16_t xCenterView = pStaticLayer->getWith() / 2;
    const uint16_t yCenterView = pStaticLayer->getHeight() / 2;

    m_MemParameterValue = m_pParameter->getTargetValue();
    pStaticLayer->changeZOrder(21);

    // Clear the static layer to prepare for new content
    pStaticLayer->eraseLayer();

    // Draw the parameter name centered at the top of the layer
    pStaticLayer->setFont(__GUI.GetFontS());
    uint16_t NameWidth = pStaticLayer->getTextWidth(m_ShortName.c_str());
    pStaticLayer->setCursor(xCenterView - (NameWidth / 2), (PARAM_NAME_HEIGHT - pStaticLayer->getTextHeight()) / 2);
    pStaticLayer->setTextFrontColor(__pActivePalette->ParameterName);
    pStaticLayer->drawText(m_ShortName.c_str());

    // Draw the static arcs representing the potentiometer boundaries
    pStaticLayer->drawArc(xCenterView, yCenterView,
                          PARAM_POT_RADIUS + 1,
                          PARAM_POT_RADIUS + 1 + 180, (PARAM_POT_ALPHA_MAX + 180) % 360,
                          __pActivePalette->ParameterLines);

    pStaticLayer->drawArc(xCenterView, yCenterView,
                          PARAM_POT_RADIUS - 9,
                          PARAM_POT_ALPHA_MIN + 180, (PARAM_POT_ALPHA_MAX + 180) % 360,
                          __pActivePalette->ParameterLines);

    // Draw parameter area number (center)
    char Buffer[30];
    snprintf(Buffer, sizeof(Buffer), "%d", NumParameterArea);
    pStaticLayer->setFont(__GUI.GetFontXSB());
    uint16_t NumberWidth = pStaticLayer->getTextWidth(Buffer);
    pStaticLayer->setCursor(xCenterView - (NumberWidth / 2), yCenterView - (pStaticLayer->getTextHeight() / 2));
    pStaticLayer->setTextFrontColor(__pActivePalette->ParameterNum);
    pStaticLayer->drawText(Buffer);

    // Pre-calc trig values to draw boundary lines
    float CosAlpha = std::cos(Deg2Rad(90.0f - static_cast<float>(PARAM_POT_ALPHA_MIN)));
    float SinAlpha = std::sin(Deg2Rad(90.0f - static_cast<float>(PARAM_POT_ALPHA_MIN)));
    float R1 = static_cast<float>(PARAM_POT_RADIUS + 1);
    float R2 = static_cast<float>(PARAM_POT_RADIUS - 9);
    float DeltaX0 = R1 * CosAlpha;
    float DeltaY0 = R1 * SinAlpha;
    float DeltaX1 = R2 * CosAlpha;
    float DeltaY1 = R2 * SinAlpha;

    // Left boundary line (outer->inner)
    uint16_t x0 = xCenterView + static_cast<uint16_t>(DeltaX0);
    uint16_t y0 = yCenterView + static_cast<uint16_t>(DeltaY0);
    uint16_t x1 = xCenterView + static_cast<uint16_t>(DeltaX1);
    uint16_t y1 = yCenterView + static_cast<uint16_t>(DeltaY1);
    pStaticLayer->drawLine(x0, y0, x1, y1, __pActivePalette->ParameterLines);

    // Right boundary (mirrored horizontally)
    x0 = xCenterView - static_cast<uint16_t>(DeltaX0);
    x1 = xCenterView - static_cast<uint16_t>(DeltaX1);
    pStaticLayer->drawLine(x0, y0, x1, y1, __pActivePalette->ParameterLines);

    // Draw dynamic part (value)
    DrawDynView(NumParameterArea, pDynamicLayer);
}

// ---------------------------------------------------------------------------------
// Function: DrawDynView
// Description: Draw the dynamic portion (value text and potentiometer arcs)
// ---------------------------------------------------------------------------------
void cParameterNumNormalView::DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) {
    const uint16_t xCenterView = pLayer->getWith() / 2;
    const uint16_t yCenterView = pLayer->getHeight() / 2;

    m_MemParameterValue = m_pParameter->getTargetValue();

    // Initialize layer for dynamic drawing
    pLayer->changeZOrder(20);
    pLayer->eraseLayer(__pActivePalette->ParameterBack);

    // Render the parameter's current value as text
    char Buffer[30];
    snprintf(Buffer, sizeof(Buffer), "%s %s", ValueToString().c_str(), m_ShortUnit.c_str());
    pLayer->setFont(__GUI.GetFontSB());
    uint16_t TextWidth = pLayer->getTextWidth(Buffer);
    pLayer->setCursor(xCenterView - (TextWidth / 2),
                      pLayer->getHeight() - ((PARAM_VAL_HEIGHT + pLayer->getTextHeight()) / 2));
    pLayer->setTextFrontColor(__pActivePalette->ParameterValue);
    pLayer->drawText(Buffer);

    // Calculate angle extent for the pot cursor based on normalized value
    uint16_t AlphaMax = (static_cast<uint16_t>(m_pParameter->getNormalizedTargetValue() * static_cast<float>(PARAM_POT_ALPHA))
                         + 180 + PARAM_POT_ALPHA_MIN) % 360;

    // Draw multiple concentric arcs for a filled gradient-like cursor
    for (uint8_t i = 0; i < 11; i++) {
        pLayer->drawArc(xCenterView,
                        yCenterView,
                        PARAM_POT_RADIUS - i + 1,
                        PARAM_POT_ALPHA_MIN + 180,
                        AlphaMax,
                        __pActivePalette->ParameterCursor);
    }
}

//**********************************************************************************
// cParameterNumLeftRightView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: DrawDynView
// Description: Draw the dynamic portion for left/right style (handles wrap-around)
// ---------------------------------------------------------------------------------
void cParameterNumLeftRightView::DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) {
    const uint16_t xCenterView = pLayer->getWith() / 2;
    const uint16_t yCenterView = pLayer->getHeight() / 2;

    m_MemParameterValue = m_pParameter->getTargetValue();

    // Initialize layer
    pLayer->changeZOrder(20);
    pLayer->eraseLayer(__pActivePalette->ParameterBack);

    // Render value text
    char Buffer[30];
    snprintf(Buffer, sizeof(Buffer), "%s %s", ValueToString().c_str(), m_ShortUnit.c_str());
    pLayer->setFont(__GUI.GetFontSB());
    uint16_t TextWidth = pLayer->getTextWidth(Buffer);
    pLayer->setCursor(xCenterView - (TextWidth / 2),
                      pLayer->getHeight() - ((PARAM_VAL_HEIGHT + pLayer->getTextHeight()) / 2));
    pLayer->setTextFrontColor(__pActivePalette->ParameterValue);
    pLayer->drawText(Buffer);

    // Compute angle and draw arcs with special-case for <180 / >=180
    uint16_t AlphaMax = (static_cast<uint16_t>(m_pParameter->getNormalizedTargetValue() * static_cast<float>(PARAM_POT_ALPHA))
                         + 180 + PARAM_POT_ALPHA_MIN) % 360;

    // Draw concentric arcs based on angle position
    for (uint8_t i = 0; i < 11; i++) {
        if (AlphaMax < 180) {
            pLayer->drawArc(xCenterView,
                            yCenterView,
                            PARAM_POT_RADIUS - i + 1,
                            0,
                            AlphaMax,
                            __pActivePalette->ParameterCursor);
        } else {
            pLayer->drawArc(xCenterView,
                            yCenterView,
                            PARAM_POT_RADIUS - i + 1,
                            AlphaMax,
                            0,
                            __pActivePalette->ParameterCursor);
        }
    }
}

//**********************************************************************************
// cParameterDiscretView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: AddDiscreteValue
// Description: Add a discrete enumerated label pair and update parameter max value
// ---------------------------------------------------------------------------------
void cParameterDiscretView::AddDiscreteValue(const std::string& ShortDiscretValue, const std::string& LongDiscretValue) {
    sDiscretValues Values;
    Values.m_LongValue  = LongDiscretValue;   // Full descriptive label
    Values.m_ShortValue = ShortDiscretValue;  // Compact display label
    m_TabDiscretValues.push_back(Values);

    // Update the backend parameter max value to reflect discrete count - 1
    m_pParameter->setMaxValue(static_cast<float>(m_TabDiscretValues.size() - 1));
}

// ---------------------------------------------------------------------------------
// Function: Draw
// Description: Draw static decorations for discrete parameter and then dynamic view
// ---------------------------------------------------------------------------------
void cParameterDiscretView::Draw(uint8_t NumParameterArea, DadGFX::cLayer* pStaticLayer, DadGFX::cLayer* pDynamicLayer) {
    m_MemParameterValue = m_pParameter->getTargetValue();
    if (m_TabDiscretValues.size() == 0) return;

    const uint16_t xCenterView = pStaticLayer->getWith() / 2;
    const uint16_t yCenterView = pStaticLayer->getHeight() / 2;

    pStaticLayer->changeZOrder(21);

    // Clear static layer
    pStaticLayer->eraseLayer();

    // Draw parameter name
    pStaticLayer->setFont(__GUI.GetFontS());
    uint16_t NameWidth = pStaticLayer->getTextWidth(m_ShortName.c_str());
    pStaticLayer->setCursor(xCenterView - (NameWidth / 2), (PARAM_NAME_HEIGHT - pStaticLayer->getTextHeight()) / 2);
    pStaticLayer->setTextFrontColor(__pActivePalette->ParameterName);
    pStaticLayer->drawText(m_ShortName.c_str());

    // Draw static pot point and surrounding arc
    pStaticLayer->drawArc(xCenterView, yCenterView,
                          PARAM_DISCRET_POT_RADIUS,
                          PARAM_DISCRET_POT_RADIUS + 4 + 180, (PARAM_POT_ALPHA_MAX + 180) % 360,
                          __pActivePalette->ParameterLines);

    // Draw discrete markers around the pot
    uint8_t NbDiscretValues = static_cast<uint8_t>(m_TabDiscretValues.size());
    if (NbDiscretValues != 0) {
        float IncAlpha = Deg2Rad(static_cast<float>(PARAM_POT_ALPHA) / static_cast<float>(NbDiscretValues + 1));
        float Alpha = Deg2Rad(240.0f) - IncAlpha;
        for (uint8_t i = 1; i <= NbDiscretValues; i++) {
            float X = static_cast<float>(PARAM_DISCRET_POT_RADIUS) * std::cos(Alpha);
            float Y = static_cast<float>(PARAM_DISCRET_POT_RADIUS) * std::sin(Alpha);

            pStaticLayer->setMode(DadGFX::DRAW_MODE::Overwrite);
            pStaticLayer->drawFillCircle(xCenterView + X, yCenterView - Y, PARAM_DISCRET_RADIUS + 1, DadGFX::sColor(0, 0, 0, 0));
            pStaticLayer->setMode(DadGFX::DRAW_MODE::Blend);
            pStaticLayer->drawCircle(xCenterView + X, yCenterView - Y, PARAM_DISCRET_RADIUS + 1, __pActivePalette->ParameterLines);
            Alpha -= IncAlpha;
        }
    }

    // Draw parameter area number
    char Buffer[30];
    snprintf(Buffer, sizeof(Buffer), "%d", NumParameterArea);
    pStaticLayer->setFont(__GUI.GetFontXSB());
    uint16_t NumberWidth = pStaticLayer->getTextWidth(Buffer);
    pStaticLayer->setCursor(xCenterView - (NumberWidth / 2), yCenterView - (pStaticLayer->getTextHeight() / 2));
    pStaticLayer->setTextFrontColor(__pActivePalette->ParameterNum);
    pStaticLayer->drawText(Buffer);

    // Draw dynamic part
    DrawDynView(NumParameterArea, pDynamicLayer);
}

// ---------------------------------------------------------------------------------
// Function: DrawDynView
// Description: Draw the dynamic part for discrete parameter (selected label + point)
// ---------------------------------------------------------------------------------
void cParameterDiscretView::DrawDynView(uint8_t NumParameterArea, DadGFX::cLayer* pLayer) {
    uint8_t NbDiscretValues = static_cast<uint8_t>(m_TabDiscretValues.size());
    if (NbDiscretValues == 0) return;

    const uint16_t xCenterView = pLayer->getWith() / 2;
    const uint16_t yCenterView = pLayer->getHeight() / 2;

    m_MemParameterValue = m_pParameter->getTargetValue();

    // Initialize layer
    pLayer->changeZOrder(20);
    pLayer->eraseLayer(__pActivePalette->ParameterBack);

    // Render selected value short label
    uint8_t NumValue = static_cast<uint8_t>(m_pParameter->getTargetValue());
    pLayer->setFont(__GUI.GetFontSB());
    uint16_t TextWidth = pLayer->getTextWidth(m_TabDiscretValues[NumValue].m_ShortValue.c_str());
    pLayer->setCursor(xCenterView - (TextWidth / 2), pLayer->getHeight() - ((PARAM_VAL_HEIGHT + pLayer->getTextHeight()) / 2));
    pLayer->setTextFrontColor(__pActivePalette->ParameterValue);
    pLayer->drawText(m_TabDiscretValues[NumValue].m_ShortValue.c_str());

    // Render discrete points and highlight the selected one
    float IncAlpha = Deg2Rad(static_cast<float>(PARAM_POT_ALPHA) / static_cast<float>(NbDiscretValues + 1));
    float Alpha = Deg2Rad(240.0f) - IncAlpha;
    for (uint8_t i = 0; i < NbDiscretValues; i++) {
        float X = static_cast<float>(PARAM_DISCRET_POT_RADIUS) * std::cos(Alpha);
        float Y = static_cast<float>(PARAM_DISCRET_POT_RADIUS) * std::sin(Alpha);
        pLayer->drawFillCircle(xCenterView + X, yCenterView - Y, PARAM_DISCRET_RADIUS + 1,
                               (NumValue == i) ? __pActivePalette->ParameterCursor : __pActivePalette->ParameterBack);
        Alpha -= IncAlpha;
    }
}

// ---------------------------------------------------------------------------------
// Function: getInfoValue
// Description: Return the long descriptive value for the info banner
// ---------------------------------------------------------------------------------
const std::string cParameterDiscretView::getInfoValue() {
    return m_TabDiscretValues[static_cast<uint8_t>(m_pParameter->getTargetValue())].m_LongValue;
}

//**********************************************************************************
// cParameterInfoView implementation
//**********************************************************************************

// ---------------------------------------------------------------------------------
// Function: ShowParamView
// Description: Show a temporary banner with parameter name and value
// ---------------------------------------------------------------------------------
#define NAME_OFFSET 1
void cParameterInfoView::ShowParamView(DadGFX::cLayer* pLayer, const std::string Name, const std::string Value) {
    const uint16_t xCenterView = pLayer->getWith() / 2;

    pLayer->changeZOrder(41); // Bring to foreground
    pLayer->eraseLayer(__pActivePalette->ParamInfoBack);

    // Parameter name (small font)
    pLayer->setFont(FONTL);
    uint16_t NameWidth = pLayer->getTextWidth(Name.c_str());
    pLayer->setCursor(xCenterView - (NameWidth / 2), NAME_OFFSET);
    pLayer->setTextFrontColor(__pActivePalette->ParamInfoName);
    pLayer->drawText(Name.c_str());

    // Parameter value (large font)
    uint16_t NameHeight = pLayer->getTextHeight() + NAME_OFFSET;
    pLayer->setFont(FONTXL);
    uint16_t ValueWidth = pLayer->getTextWidth(Value.c_str());
    pLayer->setCursor(xCenterView - (ValueWidth / 2), NameHeight + 1);
    pLayer->setTextFrontColor(__pActivePalette->ParamInfoValue);
    pLayer->drawText(Value.c_str());
}

// ---------------------------------------------------------------------------------
// Function: HideParamView
// Description: Hide the temporary info banner (reset z-order)
// ---------------------------------------------------------------------------------
void cParameterInfoView::HideParamView(DadGFX::cLayer* pLayer) {
    pLayer->changeZOrder(0); // Send to background
}

//**********************************************************************************
// cPanelOfParameterView implementation
//**********************************************************************************

// Layers declaration (SDRAM allocation)
DECLARE_LAYER(Parameter1LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter1LayerStat, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter2LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter2LayerStat, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter3LayerDyn, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(Parameter3LayerStat, PARAM_WIDTH, PARAM_HEIGHT);
DECLARE_LAYER(ParamInfoLayer, SCREEN_WIDTH, INFO_HEIGHT);

// Time to show information of the parameter currently being edited (ms)
#define PARAM_INFO_TIME_MS 2000

// ---------------------------------------------------------------------------------
// Function: Init
// Description: Initialize the three parameter views and allocate their layers
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Init(cParameterView* pParameter1, cParameterView* pParameter2, cParameterView* pParameter3) {
    // Initialize internal state
    m_isActive = false;

    // Parameter view pointers
    m_pParameter1 = pParameter1;
    m_pParameter2 = pParameter2;
    m_pParameter3 = pParameter3;

    // Allocate and initialize parameter layers
    m_pParameter1LayerDyn  = ADD_LAYER(__Display, Parameter1LayerDyn, 0, MENU_HEIGHT, 0);
    m_pParameter1LayerDyn->changeZOrder(0);
    m_pParameter1LayerDyn->eraseLayer();
    m_pParameter1LayerStat = ADD_LAYER(__Display, Parameter1LayerStat, 0, MENU_HEIGHT, 0);
    m_pParameter1LayerStat->changeZOrder(0);
    m_pParameter1LayerStat->eraseLayer();

    m_pParameter2LayerDyn  = ADD_LAYER(__Display, Parameter2LayerDyn, PARAM_WIDTH, MENU_HEIGHT, 0);
    m_pParameter2LayerDyn->changeZOrder(0);
    m_pParameter2LayerDyn->eraseLayer();
    m_pParameter2LayerStat = ADD_LAYER(__Display, Parameter2LayerStat, PARAM_WIDTH, MENU_HEIGHT, 0);
    m_pParameter2LayerStat->changeZOrder(0);
    m_pParameter2LayerStat->eraseLayer();

    m_pParameter3LayerDyn  = ADD_LAYER(__Display, Parameter3LayerDyn, PARAM_WIDTH * 2, MENU_HEIGHT, 0);
    m_pParameter3LayerDyn->changeZOrder(0);
    m_pParameter3LayerDyn->eraseLayer();
    m_pParameter3LayerStat = ADD_LAYER(__Display, Parameter3LayerStat, PARAM_WIDTH * 2, MENU_HEIGHT, 0);
    m_pParameter3LayerStat->changeZOrder(0);
    m_pParameter3LayerStat->eraseLayer();

    // Info layer for temporary parameter display
    m_pParamInfoLayer  = ADD_LAYER(__Display, ParamInfoLayer, 0, MENU_HEIGHT + PARAM_HEIGHT, 0);
    m_pParamInfoLayer->changeZOrder(0);
    m_pParamInfoLayer->eraseLayer();
}

// ---------------------------------------------------------------------------------
// Function: Activate
// Description: Called when the panel component becomes active
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Activate() {
    m_isActive = true;
    m_InfoViewCounter = (PARAM_INFO_TIME_MS / GUI_UPDATE_MS) + 1;
    m_ParameterInfoView.HideParamView(m_pParamInfoLayer);

    // Draw parameter 1 if exists
    if (m_pParameter1) {
        m_pParameter1->Draw(1, m_pParameter1LayerStat, m_pParameter1LayerDyn);
    } else {
        m_pParameter1LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter1LayerStat->changeZOrder(0);
    }

    // Draw parameter 2 if exists
    if (m_pParameter2) {
        m_pParameter2->Draw(2, m_pParameter2LayerStat, m_pParameter2LayerDyn);
    } else {
        m_pParameter2LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter2LayerStat->changeZOrder(0);
    }

    // Draw parameter 3 if exists
    if (m_pParameter3) {
        m_pParameter3->Draw(3, m_pParameter3LayerStat, m_pParameter3LayerDyn);
    } else {
        m_pParameter3LayerStat->eraseLayer(__pActivePalette->ParameterBack);
        m_pParameter3LayerStat->changeZOrder(0);
    }
}

// ---------------------------------------------------------------------------------
// Function: Deactivate
// Description: Called when the component becomes inactive
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Deactivate() {
    m_isActive = false;
    m_InfoViewCounter = (PARAM_INFO_TIME_MS / GUI_UPDATE_MS) + 1;
    m_ParameterInfoView.HideParamView(m_pParamInfoLayer);
}

// ---------------------------------------------------------------------------------
// Function: Update
// Description: Periodic update; checks parameter updates and shows info banner
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Update() {
    if (!m_isActive) return;

    // Update parameter 1 and show info if changed
    if (m_pParameter1) {
        if (true == m_pParameter1->Update(1, m_pParameter1LayerDyn)) {
            m_InfoViewCounter = 0;
            m_ParameterInfoView.ShowParamView(m_pParamInfoLayer, m_pParameter1->getInfoName(), m_pParameter1->getInfoValue());
        }
    }

    // Update parameter 2 and show info if changed
    if (m_pParameter2) {
        if (true == m_pParameter2->Update(2, m_pParameter2LayerDyn)) {
            m_InfoViewCounter = 0;
            m_ParameterInfoView.ShowParamView(m_pParamInfoLayer, m_pParameter2->getInfoName(), m_pParameter2->getInfoValue());
        }
    }

    // Update parameter 3 and show info if changed
    if (m_pParameter3) {
        if (true == m_pParameter3->Update(3, m_pParameter3LayerDyn)) {
            m_InfoViewCounter = 0;
            m_ParameterInfoView.ShowParamView(m_pParamInfoLayer, m_pParameter3->getInfoName(), m_pParameter3->getInfoValue());
        }
    }

    // Manage info view timeout counter
    if (m_InfoViewCounter < (PARAM_INFO_TIME_MS / GUI_UPDATE_MS)) {
        m_InfoViewCounter++;
    }

    // Hide info view when timeout reached
    if (m_InfoViewCounter == (PARAM_INFO_TIME_MS / GUI_UPDATE_MS)) {
        m_ParameterInfoView.HideParamView(m_pParamInfoLayer);
    }

    // Reset restore flag if set
    if (__GUI.IsRestoreInProcess()) {
        __GUI.resetRestoreInProcess();
    }
}

// ---------------------------------------------------------------------------------
// Function: Redraw
// Description: Force redraw of all parameter views (static+dynamic)
// ---------------------------------------------------------------------------------
void cPanelOfParameterView::Redraw() {
    if (!m_isActive) return;

    m_ParameterInfoView.HideParamView(m_pParamInfoLayer);

    // Redraw all parameters
    if (m_pParameter1) {
        m_pParameter1->Draw(1, m_pParameter1LayerStat, m_pParameter1LayerDyn);
    }
    if (m_pParameter2) {
        m_pParameter2->Draw(2, m_pParameter2LayerStat, m_pParameter2LayerDyn);
    }
    if (m_pParameter3) {
        m_pParameter3->Draw(3, m_pParameter3LayerStat, m_pParameter3LayerDyn);
    }
}

} // namespace DadGUI
//***End of file**************************************************************
