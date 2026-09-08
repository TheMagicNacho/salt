#include "lib/menu-bar.h"

MenuBar::~MenuBar() {
    if (menu_bar_) {
        DestroyMenu(menu_bar_);
        menu_bar_ = nullptr;
    }
}

void MenuBar::Create(OptionsHandler& options_handler) {
    if (menu_bar_) {
        DestroyMenu(menu_bar_);
    }
    menu_bar_ = CreateMenu();

    // File Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FileNew, L"&New\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FileOpen, L"&Open...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FileSave, L"&Save\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FileSaveAs, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FilePrint, L"&Print...\tCtrl+P");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, Context::Command::FileExit, L"E&xit\tAlt+F4");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"&File");

    // Edit Menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, Context::Command::EditUndo, L"&Undo\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, Context::Command::EditCut, L"Cu&t\tCtrl+X");
    AppendMenuW(hEditMenu, MF_STRING, Context::Command::EditCopy, L"&Copy\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, Context::Command::EditPaste, L"&Paste\tCtrl+V");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, Context::Command::EditSelectAll, L"Select &All\tCtrl+A");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(hEditMenu), L"&Edit");

    // View Menu
    HMENU hViewMenu = CreatePopupMenu();
    UINT wrapFlags = MF_STRING | (options_handler.IsWordWrap() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, wrapFlags, Context::Command::ViewWordWrap, L"&Word Wrap\tCtrl+W");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, Context::Command::ViewFont, L"Choose &Font...");
    AppendMenuW(hViewMenu, MF_STRING, Context::Command::ViewZoomIn, L"Zoom &In\tCtrl++");
    AppendMenuW(hViewMenu, MF_STRING, Context::Command::ViewZoomOut, L"Zoom &Out\tCtrl+-");
    AppendMenuW(hViewMenu, MF_STRING, Context::Command::ViewZoomReset, L"&Reset Zoom\tCtrl+0");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(
        hViewMenu, MF_STRING, Context::Command::ViewThemeToggle,
        options_handler.IsDarkMode() ? L"Switch to &Light Theme" : L"Switch to &Dark Theme");
    UINT statusFlags =
        MF_STRING | (options_handler.IsStatusBarVisible() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, statusFlags, Context::Command::ViewStatusBar, L"&Status Bar");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(hViewMenu), L"&View");

    // Help Menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, Context::Command::HelpAbout, L"&About Salt...");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), L"&Help");
}

HMENU MenuBar::GetMenuHandle() const { return menu_bar_; }
