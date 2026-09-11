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
    HMENU file_menu = CreatePopupMenu();
    AppendMenuW(file_menu, MF_STRING, Context::Command::FileNew, L"&New\tCtrl+N");
    AppendMenuW(file_menu, MF_STRING, Context::Command::FileOpen, L"&Open...\tCtrl+O");
    AppendMenuW(file_menu, MF_STRING, Context::Command::FileSave, L"&Save\tCtrl+S");
    AppendMenuW(file_menu, MF_STRING, Context::Command::FileSaveAs, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(file_menu, MF_STRING, Context::Command::FilePrint, L"&Print...\tCtrl+P");
    AppendMenuW(file_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(file_menu, MF_STRING, Context::Command::FileExit, L"E&xit\tAlt+F4");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(file_menu), L"&File");

    // Edit Menu
    HMENU edit_menu = CreatePopupMenu();
    AppendMenuW(edit_menu, MF_STRING, Context::Command::EditUndo, L"&Undo\tCtrl+Z");
    AppendMenuW(edit_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(edit_menu, MF_STRING, Context::Command::EditCut, L"Cu&t\tCtrl+X");
    AppendMenuW(edit_menu, MF_STRING, Context::Command::EditCopy, L"&Copy\tCtrl+C");
    AppendMenuW(edit_menu, MF_STRING, Context::Command::EditPaste, L"&Paste\tCtrl+V");
    AppendMenuW(edit_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(edit_menu, MF_STRING, Context::Command::EditSelectAll, L"Select &All\tCtrl+A");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(edit_menu), L"&Edit");

    // View Menu
    HMENU view_menu = CreatePopupMenu();
    UINT wrap_flags = MF_STRING | (options_handler.IsWordWrap() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(view_menu, wrap_flags, Context::Command::ViewWordWrap, L"&Word Wrap\tCtrl+W");
    AppendMenuW(view_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(view_menu, MF_STRING, Context::Command::ViewFont, L"Choose &Font...");
    AppendMenuW(view_menu, MF_STRING, Context::Command::ViewZoomIn, L"Zoom &In\tCtrl++");
    AppendMenuW(view_menu, MF_STRING, Context::Command::ViewZoomOut, L"Zoom &Out\tCtrl+-");
    AppendMenuW(view_menu, MF_STRING, Context::Command::ViewZoomReset, L"&Reset Zoom\tCtrl+0");
    AppendMenuW(view_menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(
        view_menu, MF_STRING, Context::Command::ViewThemeToggle,
        options_handler.IsDarkMode() ? L"Switch to &Light Theme" : L"Switch to &Dark Theme");
    UINT status_flags =
        MF_STRING | (options_handler.IsStatusBarVisible() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(view_menu, status_flags, Context::Command::ViewStatusBar, L"&Status Bar");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(view_menu), L"&View");

    // Help Menu
    HMENU help_menu = CreatePopupMenu();
    AppendMenuW(help_menu, MF_STRING, Context::Command::HelpAbout, L"&About Salt...");
    AppendMenuW(menu_bar_, MF_POPUP, reinterpret_cast<UINT_PTR>(help_menu), L"&Help");
}

HMENU MenuBar::GetMenuHandle() const { return menu_bar_; }
