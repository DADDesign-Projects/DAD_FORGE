//==================================================================================
//==================================================================================
// File: cUIMenu.cpp
// Description: Implementation of the cUIMenu class for graphical user interface
// menu management
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#include "cUIMenu.h"
#include "HardwareAndCo.h"

//-----------------------------------------------------------------------------
// Layer declarations for dynamic and static parts of the menu
//-----------------------------------------------------------------------------
DECLARE_LAYER(MenuLayerDyn, SCREEN_WIDTH, MENU_HEIGHT);
DECLARE_LAYER(MenuLayerStat, SCREEN_WIDTH, MENU_HEIGHT);

namespace DadGUI {

//**********************************************************************************
// Class: cUIMenu
// Description: Manages the main menu system including display, navigation, and activation
//**********************************************************************************

// -----------------------------------------------------------------------------
// Section: Public Methods
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function: Init
// Description: Initializes the menu layers and resets internal variables
// -----------------------------------------------------------------------------
void cUIMenu::Init() {
    m_pDynMenuLayer  = __Display.addLayer(&__LayerMenuLayerDyn[0][0], 0, 0, SCREEN_WIDTH, MENU_HEIGHT, 0);
    m_pStatMenuLayer = __Display.addLayer(&__LayerMenuLayerStat[0][0], 0, 0, SCREEN_WIDTH, MENU_HEIGHT, 0);

    m_TabMenuItem.clear();   // Remove all menu items
    m_ActiveItem = 0;        // Reset current item index
    m_MenuShift  = 0;        // Reset scroll/shift index
    m_isActive   = false;    // Menu starts inactive

    Deactivate();            // Ensure the menu is deactivated
}

// -----------------------------------------------------------------------------
// Function: addMenuItem
// Description: Adds a new item to the menu
// Parameters:
//   - pItem: Pointer to the associated GUI component
//   - Name: Label of the menu item
//   - pNextMenu: Pointer to the next submenu (optional)
// -----------------------------------------------------------------------------
void cUIMenu::addMenuItem(iUIComponent* pItem, const std::string& Name, cUIMenu* pNextMenu) {
    MenuItem Item;
    Item.Name      = Name;      // Item display name
    Item.pItem     = pItem;     // Associated UI component
    Item.pNextMenu = pNextMenu; // Next submenu pointer

    m_TabMenuItem.push_back(Item);  // Store in the menu item list
}

// -----------------------------------------------------------------------------
// Function: Activate
// Description: Activates the menu display and the currently selected item
// -----------------------------------------------------------------------------
void cUIMenu::Activate() {
    m_pDynMenuLayer->changeZOrder(101);   // Dynamic layer above static
    m_pStatMenuLayer->changeZOrder(100);  // Static layer background

    m_isActive = true;                    // Mark as active
    m_ActiveItem = 0;
    drawTab();                            // Draw menu tabs

    // Activate the current item if available
    if (!m_TabMenuItem.empty()) {
        m_TabMenuItem[m_ActiveItem].pItem->Activate();
    }
}

// -----------------------------------------------------------------------------
// Function: Deactivate
// Description: Deactivates the menu and resets layer visibility
// -----------------------------------------------------------------------------
void cUIMenu::Deactivate() {
    m_pDynMenuLayer->changeZOrder(0);     // Hide dynamic layer
    m_pStatMenuLayer->changeZOrder(0);    // Hide static layer

    m_isActive = false;                   // Mark as inactive

    // Deactivate current item if valid
    if (!m_TabMenuItem.empty()) {
        m_TabMenuItem[m_ActiveItem].pItem->Deactivate();
    }
}

// -----------------------------------------------------------------------------
// Function: Update
// Description: Handles menu navigation and user input
// -----------------------------------------------------------------------------
void cUIMenu::Update() {
    // Skip update if inactive or no items
    if (!m_isActive || m_TabMenuItem.empty()) return;

    int8_t OldActiveItem = m_ActiveItem;  // Backup previous selection

    // Handle encoder rotation for navigation
    uint8_t Increment = __Encoder0.getIncrement();
    if (Increment != 0) {
        m_ActiveItem += Increment;  // Change active item index

        // Clamp to valid range
        if (m_ActiveItem >= (int8_t)m_TabMenuItem.size())
            m_ActiveItem = m_TabMenuItem.size() - 1;
        if (m_ActiveItem < 0)
            m_ActiveItem = 0;

        // Adjust scrolling for visibility
        while (m_ActiveItem >= (NB_MENU_ITEM + m_MenuShift)) m_MenuShift++;
        while (m_ActiveItem < m_MenuShift) m_MenuShift--;

        // Switch active menu item
        m_TabMenuItem[OldActiveItem].pItem->Deactivate();
        m_TabMenuItem[m_ActiveItem].pItem->Activate();

        drawTab();  // Redraw menu after change
    }

    // Update current item's logic
    m_TabMenuItem[m_ActiveItem].pItem->Update();
}

// -----------------------------------------------------------------------------
// Function: Redraw
// Description: Forces redraw of UI component
// -----------------------------------------------------------------------------
void cUIMenu::Redraw() {
    if (m_isActive) {
        drawTab();
        // Update current item's logic
        m_TabMenuItem[m_ActiveItem].pItem->Redraw();
    }
}

// -----------------------------------------------------------------------------
// Section: Private Methods
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Function: drawTab
// Description: Draws the visible portion of the menu, highlighting the active item
// -----------------------------------------------------------------------------
void cUIMenu::drawTab() {
    m_pDynMenuLayer->eraseLayer(__pActivePalette->MenuBack);  // Clear layer

    uint8_t  LastTab = m_MenuShift + NB_MENU_ITEM;            // Last visible item index
    uint16_t xTab    = MENU_EDGE;                             // Initial X position

    // Draw visible menu items
    for (uint8_t Index = m_MenuShift; Index < LastTab; Index++) {
        if (Index == m_ActiveItem) {
            // Highlight the active tab
            m_pDynMenuLayer->drawFillRect(
                xTab, 0, MENU_ITEM_WIDTH, MENU_HEIGHT, __pActivePalette->MenuActive);
        }

        // Draw text label if within item list
        if (Index < m_TabMenuItem.size()) {
            m_pDynMenuLayer->setFont(FONTXSB);
            uint16_t TextWidth = m_pDynMenuLayer->getTextWidth(m_TabMenuItem[Index].Name.c_str());
            m_pDynMenuLayer->setCursor(xTab + (MENU_ITEM_WIDTH - TextWidth) / 2, 2);

            if (Index == m_ActiveItem)
                m_pDynMenuLayer->setTextFrontColor(__pActivePalette->MenuActiveText);
            else
                m_pDynMenuLayer->setTextFrontColor(__pActivePalette->MenuText);

            m_pDynMenuLayer->drawText(m_TabMenuItem[Index].Name.c_str());
            xTab += MENU_ITEM_WIDTH;  // Advance to next tab position
        }
    }

    // Draw arrow indicators if needed
    if (m_MenuShift > 0) drawArrowIndicator(true);   // Left arrow
    if (m_MenuShift < (int8_t)(m_TabMenuItem.size() - NB_MENU_ITEM))
        drawArrowIndicator(false);                    // Right arrow
}

// -----------------------------------------------------------------------------
// Function: drawArrowIndicator
// Description: Draws an arrow indicator on either the left or right edge of the menu
// Parameters:
//   - isLeft: If true, draws a left arrow; otherwise, a right arrow
// -----------------------------------------------------------------------------
void cUIMenu::drawArrowIndicator(bool isLeft) {
    uint16_t YMin = MENU_EDGE;
    uint16_t YMax = MENU_EDGE + 1;
    uint16_t XStart = isLeft ? 0 : 319;  // Arrow edge position
    int16_t  XOffset = isLeft ? 1 : -1;  // Direction of arrow

    // Draw arrow lines symmetrically
    for (uint16_t Index = 0; Index < MENU_EDGE; Index++) {
        m_pDynMenuLayer->drawLine(
            XStart + Index * XOffset, YMin, XStart + Index * XOffset, YMax,
            __pActivePalette->MenuArrow);

        YMin--;  // Expand vertically
        YMax++;
    }
}

} // namespace DadGUI
//***End of file**************************************************************
