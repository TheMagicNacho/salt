#include <gtest/gtest.h>
#include <windows.h>

#include "lib/context.h"
#include "lib/menu-bar.h"
#include "lib/options-handler.h"

namespace {

TEST(MenuBarTest, InitialStateHasNullMenuHandle) {
    // Arrange: Create an uninitialized MenuBar instance.
    MenuBar menu_bar;

    // Act: Query the menu handle.
    HMENU handle = menu_bar.GetMenuHandle();

    // Assert: Handle must initially be null before Create is called.
    EXPECT_EQ(handle, nullptr);
}

TEST(MenuBarTest, CreateInitializesMenuBarWithFourSubmenus) {
    // Arrange: Instantiate MenuBar and default OptionsHandler.
    MenuBar menu_bar;
    OptionsHandler options;

    // Act: Create the menu bar structure.
    menu_bar.Create(options);
    HMENU root = menu_bar.GetMenuHandle();

    // Assert: Root menu must be valid and contain 4 top-level popup menus (File, Edit, View, Help).
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(GetMenuItemCount(root), 4);

    HMENU file_menu = GetSubMenu(root, 0);
    HMENU edit_menu = GetSubMenu(root, 1);
    HMENU view_menu = GetSubMenu(root, 2);
    HMENU help_menu = GetSubMenu(root, 3);

    EXPECT_NE(file_menu, nullptr);
    EXPECT_NE(edit_menu, nullptr);
    EXPECT_NE(view_menu, nullptr);
    EXPECT_NE(help_menu, nullptr);
}

TEST(MenuBarTest, CreatePopulatesFileMenuCommands) {
    // Arrange: Create MenuBar and OptionsHandler.
    MenuBar menu_bar;
    OptionsHandler options;

    // Act: Build menu and fetch File submenu (index 0).
    menu_bar.Create(options);
    HMENU file_menu = GetSubMenu(menu_bar.GetMenuHandle(), 0);
    ASSERT_NE(file_menu, nullptr);

    // Assert: File menu items match Context::Command identifiers.
    EXPECT_EQ(GetMenuItemID(file_menu, 0), Context::Command::FileNew);
    EXPECT_EQ(GetMenuItemID(file_menu, 1), Context::Command::FileOpen);
    EXPECT_EQ(GetMenuItemID(file_menu, 2), Context::Command::FileSave);
    EXPECT_EQ(GetMenuItemID(file_menu, 3), Context::Command::FileSaveAs);
    // Index 4 is a separator (id 0)
    EXPECT_EQ(GetMenuItemID(file_menu, 5), Context::Command::FilePrint);
    // Index 6 is a separator (id 0)
    EXPECT_EQ(GetMenuItemID(file_menu, 7), Context::Command::FileExit);
}

TEST(MenuBarTest, CreatePopulatesEditMenuCommands) {
    // Arrange: Create MenuBar and OptionsHandler.
    MenuBar menu_bar;
    OptionsHandler options;

    // Act: Build menu and fetch Edit submenu (index 1).
    menu_bar.Create(options);
    HMENU edit_menu = GetSubMenu(menu_bar.GetMenuHandle(), 1);
    ASSERT_NE(edit_menu, nullptr);

    // Assert: Edit menu items match Context::Command identifiers.
    EXPECT_EQ(GetMenuItemID(edit_menu, 0), Context::Command::EditUndo);
    // Index 1 is a separator
    EXPECT_EQ(GetMenuItemID(edit_menu, 2), Context::Command::EditCut);
    EXPECT_EQ(GetMenuItemID(edit_menu, 3), Context::Command::EditCopy);
    EXPECT_EQ(GetMenuItemID(edit_menu, 4), Context::Command::EditPaste);
    // Index 5 is a separator
    EXPECT_EQ(GetMenuItemID(edit_menu, 6), Context::Command::EditSelectAll);
}

TEST(MenuBarTest, CreateReflectsWordWrapOptionCheckedState) {
    // Arrange: OptionsHandler with WordWrap enabled by default.
    MenuBar menu_bar;
    OptionsHandler options;
    ASSERT_TRUE(options.IsWordWrap());

    // Act 1: Build menu when WordWrap is active.
    menu_bar.Create(options);
    HMENU view_menu = GetSubMenu(menu_bar.GetMenuHandle(), 2);
    ASSERT_NE(view_menu, nullptr);
    UINT state1 = GetMenuState(view_menu, Context::Command::ViewWordWrap, MF_BYCOMMAND);

    // Assert 1: Menu item must have MF_CHECKED flag.
    EXPECT_TRUE(state1 & MF_CHECKED);

    // Act 2: Disable WordWrap and re-create menu.
    options.ToggleWordWrap();
    ASSERT_FALSE(options.IsWordWrap());
    menu_bar.Create(options);
    view_menu = GetSubMenu(menu_bar.GetMenuHandle(), 2);
    UINT state2 = GetMenuState(view_menu, Context::Command::ViewWordWrap, MF_BYCOMMAND);

    // Assert 2: Menu item must now be unchecked.
    EXPECT_FALSE(state2 & MF_CHECKED);
}

TEST(MenuBarTest, CreateReflectsStatusBarOptionCheckedState) {
    // Arrange: OptionsHandler with StatusBar disabled by default.
    MenuBar menu_bar;
    OptionsHandler options;
    ASSERT_FALSE(options.IsStatusBarVisible());

    // Act 1: Build menu when StatusBar is hidden.
    menu_bar.Create(options);
    HMENU view_menu = GetSubMenu(menu_bar.GetMenuHandle(), 2);
    ASSERT_NE(view_menu, nullptr);
    UINT state1 = GetMenuState(view_menu, Context::Command::ViewStatusBar, MF_BYCOMMAND);

    // Assert 1: Status Bar item must be unchecked.
    EXPECT_FALSE(state1 & MF_CHECKED);

    // Act 2: Enable StatusBar and re-create menu.
    options.ToggleStatusBar();
    ASSERT_TRUE(options.IsStatusBarVisible());
    menu_bar.Create(options);
    view_menu = GetSubMenu(menu_bar.GetMenuHandle(), 2);
    UINT state2 = GetMenuState(view_menu, Context::Command::ViewStatusBar, MF_BYCOMMAND);

    // Assert 2: Status Bar item must now be checked.
    EXPECT_TRUE(state2 & MF_CHECKED);
}

TEST(MenuBarTest, CreateCanBeCalledRepeatedlyWithoutLeakOrError) {
    // Arrange: Create MenuBar and OptionsHandler.
    MenuBar menu_bar;
    OptionsHandler options;

    // Act: Call Create multiple times in succession to verify safe recreation.
    for (int i = 0; i < 5; ++i) {
        menu_bar.Create(options);
    }
    HMENU root = menu_bar.GetMenuHandle();

    // Assert: Final handle is valid and still contains 4 submenus.
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(GetMenuItemCount(root), 4);
}

}  // namespace
