#include "main-window.h"

#include <dwmapi.h>
#include <uxtheme.h>
#include <objbase.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#define DWMSBT_MAINWINDOW 2
#endif

// UXTheme dark mode undocumented helper functions
typedef enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max } PreferredAppMode;

typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode appMode);
typedef BOOL(WINAPI* fnAllowDarkModeForWindow)(HWND hWnd, BOOL allow);

bool MainWindow::RegisterClass(HINSTANCE instance) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindow::StaticWndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = kClassName;

    return RegisterClassExW(&wc) != 0;
}

HACCEL MainWindow::CreateAppAccelerators() {
    ACCEL accels[] = {
        {FCONTROL | FVIRTKEY, 'N', Command::FileNew},
        {FCONTROL | FVIRTKEY, 'O', Command::FileOpen},
        {FCONTROL | FVIRTKEY, 'S', Command::FileSave},
        {FCONTROL | FSHIFT | FVIRTKEY, 'S', Command::FileSaveAs},
        {FCONTROL | FVIRTKEY, 'P', Command::FilePrint},
        {FCONTROL | FVIRTKEY, 'W', Command::ViewWordWrap},
        {FCONTROL | FVIRTKEY, 'Z', Command::EditUndo},
        {FCONTROL | FVIRTKEY, 'A', Command::EditSelectAll},
        {FCONTROL | FVIRTKEY, VK_OEM_PLUS, Command::ViewZoomIn},
        {FCONTROL | FVIRTKEY, VK_ADD, Command::ViewZoomIn},
        {FCONTROL | FVIRTKEY, VK_OEM_MINUS, Command::ViewZoomOut},
        {FCONTROL | FVIRTKEY, VK_SUBTRACT, Command::ViewZoomOut},
        {FCONTROL | FVIRTKEY, '0', Command::ViewZoomReset},
        {FCONTROL | FVIRTKEY, VK_NUMPAD0, Command::ViewZoomReset},
    };
    return CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));
}

HWND MainWindow::Create(HINSTANCE instance, int show_state) {
    hwnd_ = CreateWindowExW(0, kClassName, L"Untitled - Salt Text Editor", WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, 1000, 680, NULL, NULL, instance, this);

    if (hwnd_) {
        ShowWindow(hwnd_, show_state);
        UpdateWindow(hwnd_);
    }
    return hwnd_;
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM param_w, LPARAM param_l) {
    MainWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(param_l);
        self = reinterpret_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        if (self) {
            self->hwnd_ = hwnd;
        }
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->HandleMessage(msg, param_w, param_l);
    }

    return DefWindowProcW(hwnd, msg, param_w, param_l);
}

void MainWindow::EnableDarkMode(bool enable) {
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(enable));

    // Try Windows 11 backdrop (Mica effect)
    int backdrop = DWMSBT_MAINWINDOW;
    DwmSetWindowAttribute(hwnd_, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

    // Load uxtheme for dark context menus if supported
    HMODULE hUxTheme = GetModuleHandleW(L"uxtheme.dll");
    if (hUxTheme) {
        fnSetPreferredAppMode SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(
            GetProcAddress(hUxTheme, MAKEINTRESOURCEA(135)));
        if (SetPreferredAppMode) {
            SetPreferredAppMode(enable ? AllowDark : Default);
        }
        fnAllowDarkModeForWindow AllowDarkModeForWindow =
            reinterpret_cast<fnAllowDarkModeForWindow>(
                GetProcAddress(hUxTheme, MAKEINTRESOURCEA(133)));
        if (AllowDarkModeForWindow) {
            AllowDarkModeForWindow(hwnd_, enable);
        }
    }
}

void MainWindow::UpdateTitle() {
    std::wstring title = file_handler_.GetFileName();
    if (file_handler_.IsDirty()) {
        title += L"*";
    }
    title += L" - Salt Text Editor";
    SetWindowTextW(hwnd_, title.c_str());
}

void MainWindow::UpdateStatusBar() {
    if (!status_hwnd_ || !edit_hwnd_) return;

    DWORD start = 0, end = 0;
    SendMessageW(edit_hwnd_, EM_GETSEL, reinterpret_cast<WPARAM>(&start),
                 reinterpret_cast<LPARAM>(&end));
    LRESULT line_index = SendMessageW(edit_hwnd_, EM_LINEFROMCHAR, start, 0);
    LRESULT line_start_char = SendMessageW(edit_hwnd_, EM_LINEINDEX, line_index, 0);
    LRESULT col_index = start - line_start_char;
    int total_chars = GetWindowTextLengthW(edit_hwnd_);

    wchar_t pos_text[64];
    swprintf_s(pos_text, L"  Ln %ld, Col %ld", static_cast<long>(line_index + 1),
               static_cast<long>(col_index + 1));
    SendMessageW(status_hwnd_, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(pos_text));

    wchar_t count_text[64];
    swprintf_s(count_text, L"  %d characters", total_chars);
    SendMessageW(status_hwnd_, SB_SETTEXTW, 1, reinterpret_cast<LPARAM>(count_text));

    SendMessageW(status_hwnd_, SB_SETTEXTW, 2, reinterpret_cast<LPARAM>(L"  Windows (CRLF)"));
    SendMessageW(status_hwnd_, SB_SETTEXTW, 3, reinterpret_cast<LPARAM>(L"  UTF-8"));

    const wchar_t* state_text = file_handler_.IsDirty() ? L"  Modified" : L"  Saved";
    SendMessageW(status_hwnd_, SB_SETTEXTW, 4, reinterpret_cast<LPARAM>(state_text));
}

void MainWindow::CreateAppMenu() {
    HMENU hMenuBar = CreateMenu();

    // File Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, Command::FileNew, L"&New\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, Command::FileOpen, L"&Open...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, Command::FileSave, L"&Save\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, Command::FileSaveAs, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, Command::FilePrint, L"&Print...\tCtrl+P");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, Command::FileExit, L"E&xit\tAlt+F4");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"&File");

    // Edit Menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, Command::EditUndo, L"&Undo\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, Command::EditCut, L"Cu&t\tCtrl+X");
    AppendMenuW(hEditMenu, MF_STRING, Command::EditCopy, L"&Copy\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, Command::EditPaste, L"&Paste\tCtrl+V");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, Command::EditSelectAll, L"Select &All\tCtrl+A");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hEditMenu), L"&Edit");

    // View Menu
    HMENU hViewMenu = CreatePopupMenu();
    UINT wrapFlags = MF_STRING | (options_handler_.IsWordWrap() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, wrapFlags, Command::ViewWordWrap, L"&Word Wrap\tCtrl+W");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, Command::ViewFont, L"Choose &Font...");
    AppendMenuW(hViewMenu, MF_STRING, Command::ViewZoomIn, L"Zoom &In\tCtrl++");
    AppendMenuW(hViewMenu, MF_STRING, Command::ViewZoomOut, L"Zoom &Out\tCtrl+-");
    AppendMenuW(hViewMenu, MF_STRING, Command::ViewZoomReset, L"&Reset Zoom\tCtrl+0");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(
        hViewMenu, MF_STRING, Command::ViewThemeToggle,
        options_handler_.IsDarkMode() ? L"Switch to &Light Theme" : L"Switch to &Dark Theme");
    UINT statusFlags =
        MF_STRING | (options_handler_.IsStatusBarVisible() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, statusFlags, Command::ViewStatusBar, L"&Status Bar");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hViewMenu), L"&View");

    // Help Menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, Command::HelpAbout, L"&About Salt...");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), L"&Help");

    bool x = SetMenu(hwnd_, hMenuBar);
    if (!x) {
        MessageBoxW(hwnd_, L"Failed to set menu", L"Error", MB_OK | MB_ICONERROR);
    }
}

void MainWindow::RecreateEditControl() {
    int len = edit_hwnd_ ? GetWindowTextLengthW(edit_hwnd_) : 0;
    std::wstring buffer;
    if (len > 0) {
        buffer.resize(len);
        GetWindowTextW(edit_hwnd_, &buffer[0], len + 1);
    }

    if (edit_hwnd_) {
        DestroyWindow(edit_hwnd_);
    }

    DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL;
    if (!options_handler_.IsWordWrap()) {
        style |= WS_HSCROLL | ES_AUTOHSCROLL;
    }

    RECT rcClient;
    GetClientRect(hwnd_, &rcClient);

    edit_hwnd_ = CreateWindowExW(
        0, L"EDIT", buffer.c_str(), style, 0, 0, rcClient.right, rcClient.bottom, hwnd_,
        reinterpret_cast<HMENU>(ControlId::MainEdit), GetModuleHandle(NULL), NULL);

    // Padding margins for modern comfortable reading
    SendMessageW(edit_hwnd_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

    // Tab size: 4 spaces equivalent
    int tabStops = 16;
    SendMessageW(edit_hwnd_, EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&tabStops));

    file_handler_.SetEditHandle(edit_hwnd_);
    options_handler_.SetEditHandle(edit_hwnd_);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM param_w, LPARAM param_l) {
    switch (msg) {
        case WM_CREATE: {
            EnableDarkMode(options_handler_.IsDarkMode());

            // Create Status Bar
            DWORD status_style = WS_CHILD | SBARS_SIZEGRIP;
            if (options_handler_.IsStatusBarVisible()) {
                status_style |= WS_VISIBLE;
            }
            status_hwnd_ = CreateWindowExW(
                0, STATUSCLASSNAMEW, NULL, status_style, 0, 0, 0, 0,
                hwnd_, reinterpret_cast<HMENU>(ControlId::MainStatus), GetModuleHandle(NULL), NULL);

            int statusParts[] = {160, 310, 440, 540, -1};
            SendMessageW(status_hwnd_, SB_SETPARTS, 5, reinterpret_cast<LPARAM>(statusParts));

            // Create Edit Control
            RecreateEditControl();
            options_handler_.Init(hwnd_, edit_hwnd_);

            CreateAppMenu();
            UpdateTitle();
            UpdateStatusBar();

            SetFocus(edit_hwnd_);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(param_l);
            int height = HIWORD(param_l);

            int statusHeight = 0;
            if (status_hwnd_ && options_handler_.IsStatusBarVisible()) {
                SendMessageW(status_hwnd_, WM_SIZE, param_w, param_l);
                RECT rcStatus;
                GetWindowRect(status_hwnd_, &rcStatus);
                statusHeight = rcStatus.bottom - rcStatus.top;
            }

            if (edit_hwnd_) {
                MoveWindow(edit_hwnd_, 0, 0, width, height - statusHeight, TRUE);
            }
            return 0;
        }

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            if (reinterpret_cast<HWND>(param_l) == edit_hwnd_) {
                HDC hdc = reinterpret_cast<HDC>(param_w);
                SetBkColor(hdc, options_handler_.GetBackgroundColor());
                SetTextColor(hdc, options_handler_.GetTextColor());
                return reinterpret_cast<LRESULT>(options_handler_.GetBackgroundBrush());
            }
            break;
        }

        case WM_COMMAND: {
            // Edit control notifications
            if (HIWORD(param_w) == EN_CHANGE && reinterpret_cast<HWND>(param_l) == edit_hwnd_) {
                if (!file_handler_.IsDirty()) {
                    file_handler_.SetDirty(true);
                    UpdateTitle();
                }
                UpdateStatusBar();
                return 0;
            }

            Command cmd = static_cast<Command>(LOWORD(param_w));
            switch (cmd) {
                case Command::FileNew:
                    if (file_handler_.New(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Command::FileOpen:
                    if (file_handler_.Open(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Command::FileSave:
                    if (file_handler_.Save(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Command::FileSaveAs:
                    if (file_handler_.SaveAs(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Command::FilePrint:
                    file_handler_.Print(hwnd_);
                    break;

                case Command::FileExit:
                    SendMessageW(hwnd_, WM_CLOSE, 0, 0);
                    break;

                case Command::EditUndo:
                    SendMessageW(edit_hwnd_, EM_UNDO, 0, 0);
                    break;

                case Command::EditCut:
                    SendMessageW(edit_hwnd_, WM_CUT, 0, 0);
                    break;

                case Command::EditCopy:
                    SendMessageW(edit_hwnd_, WM_COPY, 0, 0);
                    break;

                case Command::EditPaste:
                    SendMessageW(edit_hwnd_, WM_PASTE, 0, 0);
                    break;

                case Command::EditSelectAll:
                    SendMessageW(edit_hwnd_, EM_SETSEL, 0, -1);
                    break;

                case Command::ViewWordWrap: {
                    options_handler_.ToggleWordWrap();
                    RecreateEditControl();
                    CreateAppMenu();
                    RECT rc;
                    GetClientRect(hwnd_, &rc);
                    SendMessageW(hwnd_, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
                    SetFocus(edit_hwnd_);
                    break;
                }

                case Command::ViewFont:
                    options_handler_.ChooseFontDialog();
                    break;

                case Command::ViewZoomIn:
                    options_handler_.ZoomIn();
                    break;

                case Command::ViewZoomOut:
                    options_handler_.ZoomOut();
                    break;

                case Command::ViewZoomReset:
                    options_handler_.ZoomReset();
                    break;

                case Command::ViewThemeToggle:
                    options_handler_.ToggleTheme();
                    EnableDarkMode(options_handler_.IsDarkMode());
                    CreateAppMenu();
                    break;

                case Command::ViewStatusBar: {
                    bool visible = options_handler_.ToggleStatusBar();
                    ShowWindow(status_hwnd_, visible ? SW_SHOW : SW_HIDE);
                    CreateAppMenu();
                    RECT rc;
                    GetClientRect(hwnd_, &rc);
                    SendMessageW(hwnd_, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
                    break;
                }

                case Command::HelpAbout:
                    MessageBoxW(hwnd_,
                                L"Salt Text Editor v0.1.0\n\n"
                                L"A text editor for Windows.\n"
                                L"Made for those salty that notepad became bloated.\n\n",
                                L"About Salt", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_SETFOCUS:
            if (edit_hwnd_) {
                SetFocus(edit_hwnd_);
                return 0;
            }
            break;

        case WM_CLOSE:
            if (!file_handler_.PromptSaveIfDirty(hwnd_)) {
                return 0;  // Cancel close
            }
            DestroyWindow(hwnd_);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd_, msg, param_w, param_l);
}
