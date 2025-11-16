//==================================================================================
//==================================================================================
// File: cUIMenu.h
// Description: Declaration of the cUIMenu class for graphical user interface
// menu management
//
// Copyright (c) 2025 Dad Design.
//==================================================================================
//==================================================================================

#pragma once

#include "GUI_Include.h"
#include <string>

namespace DadGUI {

//**********************************************************************************
// Struct: MenuItem
// Description: Represents an individual menu entry with display name
// and associated components
//**********************************************************************************
class cUIMenu;

struct MenuItem {
    std::string   Name;        // Display name of the menu item
    iUIComponent* pItem;       // Pointer to the associated GUI component
    cUIMenu*      pNextMenu;   // Optional pointer to a submenu
};

//**********************************************************************************
// Class: cUIMenu
// Description: Handles the graphical menu display and navigation logic
//**********************************************************************************
class cUIMenu : public iUIComponent {
public:
    virtual ~cUIMenu() {}

    // -----------------------------------------------------------------------------
    // Section: Public Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Function: Init
    // Description: Initializes the menu system by creating display layers and resetting states
    // -----------------------------------------------------------------------------
    void Init();

    // -----------------------------------------------------------------------------
    // Function: addMenuItem
    // Description: Adds a new menu entry
    // Parameters:
    //   - pItem: Pointer to the associated GUI component
    //   - Name: Display name of the menu entry
    //   - pNextMenu: Pointer to a submenu (optional)
    // -----------------------------------------------------------------------------
    void addMenuItem(iUIComponent* pItem, const std::string& Name,
                     cUIMenu* pNextMenu = nullptr);

    // -----------------------------------------------------------------------------
    // Function: Activate
    // Description: Activates the menu display and highlights the current item
    // -----------------------------------------------------------------------------
    void Activate() override;

    // -----------------------------------------------------------------------------
    // Function: Deactivate
    // Description: Deactivates the menu and hides the layers
    // -----------------------------------------------------------------------------
    void Deactivate() override;

    // -----------------------------------------------------------------------------
    // Function: Update
    // Description: Handles user input and navigation between menu items
    // -----------------------------------------------------------------------------
    void Update() override;

    // -----------------------------------------------------------------------------
    // Function: Redraw
    // Description: Forces redraw of UI component
    // -----------------------------------------------------------------------------
    void Redraw() override;

    // -----------------------------------------------------------------------------
    // Function: drawTab
    // Description: Draws the visible menu items and the active selection highlight
    // -----------------------------------------------------------------------------
    void drawTab();

    // -----------------------------------------------------------------------------
    // Function: setItem
    // Description: Sets the currently active menu item index
    // Parameters:
    //   - Item: Index of the item to activate
    // -----------------------------------------------------------------------------
    void setItem(uint8_t Item) {
        if (Item < m_TabMenuItem.size()) {
            m_ActiveItem = Item;                 // Set specified item as active
        } else {
            m_ActiveItem = m_TabMenuItem.size() - 1; // Clamp to last item
        }
        drawTab();
    }

    // -----------------------------------------------------------------------------
    // Function: GetItem
    // Description: Returns the index of the currently active menu item
    // -----------------------------------------------------------------------------
    uint8_t GetItem() const { return m_ActiveItem; }

protected:
    // -----------------------------------------------------------------------------
    // Section: Protected Methods
    // -----------------------------------------------------------------------------

    // -----------------------------------------------------------------------------
    // Function: drawArrowIndicator
    // Description: Draws an arrow indicator on the left or right side of the menu bar
    // Parameters:
    //   - isLeft: If true, draws a left arrow; otherwise, a right arrow
    // -----------------------------------------------------------------------------
    void drawArrowIndicator(bool isLeft);

    // -----------------------------------------------------------------------------
    // Section: Member Variables
    // -----------------------------------------------------------------------------

    DadGFX::cLayer* m_pDynMenuLayer;   // Pointer to the dynamic (active) menu layer
    DadGFX::cLayer* m_pStatMenuLayer;  // Pointer to the static (background) menu layer

    std::vector<MenuItem> m_TabMenuItem;  // List of menu items
    int8_t  m_ActiveItem;                 // Index of the currently selected item
    int8_t  m_MenuShift;                  // Horizontal scroll offset for tab display
    uint8_t m_isActive;                   // Indicates if the menu is currently active
};

} // namespace DadGUI
//***End of file**************************************************************
