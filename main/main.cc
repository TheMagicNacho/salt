#ifndef WINVER
#define WINVER 0x0A00
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <objbase.h>
#include <string>

#include "lib/defaults.h"
#include "lib/file-handler.h"
#include "lib/options-handler.h"

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Command IDs
#define ID_FILE_NEW 1001
#define ID_FILE_OPEN 1002
#define ID_FILE_SAVE 1003
#define ID_FILE_SAVE_AS 1004
#define ID_FILE_PRINT 1005
#define ID_FILE_EXIT 1006

#define ID_EDIT_UNDO 1011
#define ID_EDIT_CUT 1012
#define ID_EDIT_COPY 1013
#define ID_EDIT_PASTE 1014
#define ID_EDIT_SELECT_ALL 1015

#define ID_VIEW_WORD_WRAP 1021
#define ID_VIEW_FONT 1022
#define ID_VIEW_ZOOM_IN 1023
#define ID_VIEW_ZOOM_OUT 1024
#define ID_VIEW_ZOOM_RESET 1025
#define ID_VIEW_THEME_TOGGLE 1026
#define ID_VIEW_STATUS_BAR 1027

#define ID_HELP_ABOUT 1031

#define IDC_MAIN_EDIT 2001
#define IDC_MAIN_STATUS 2002

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#define DWMSBT_MAINWINDOW 2
#endif

// UXTheme dark mode undocumented helper functions
typedef enum PreferredAppMode {
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
} PreferredAppMode;

typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode appMode);
typedef BOOL(WINAPI* fnAllowDarkModeForWindow)(HWND hWnd, BOOL allow);

static void EnableWindowDarkMode(HWND hwnd, bool enable) {
    BOOL value = enable ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

    // Try Windows 11 backdrop (Mica effect)
    int backdrop = DWMSBT_MAINWINDOW;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

    // Load uxtheme for dark context menus if supported
    HMODULE hUxTheme = GetModuleHandleW(L"uxtheme.dll");
    if (hUxTheme) {
        fnSetPreferredAppMode SetPreferredAppMode =
            reinterpret_cast<fnSetPreferredAppMode>(GetProcAddress(hUxTheme, MAKEINTRESOURCEA(135)));
        if (SetPreferredAppMode) {
            SetPreferredAppMode(enable ? AllowDark : Default);
        }
        fnAllowDarkModeForWindow AllowDarkModeForWindow =
            reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxTheme, MAKEINTRESOURCEA(133)));
        if (AllowDarkModeForWindow) {
            AllowDarkModeForWindow(hwnd, enable ? TRUE : FALSE);
        }
    }
}

struct AppState {
    HWND hEdit{nullptr};
    HWND hStatus{nullptr};
    FileHandler file_handler;
    OptionsHandler options_handler;

    void UpdateTitle(HWND hwnd) {
        std::wstring title = file_handler.GetFileName();
        if (file_handler.IsDirty()) {
            title += L"*";
        }
        title += L" - Salt Text Editor";
        SetWindowTextW(hwnd, title.c_str());
    }

    void UpdateStatusBar() {
        if (!hStatus || !hEdit) return;

        DWORD start = 0, end = 0;
        SendMessageW(hEdit, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
        LRESULT line_index = SendMessageW(hEdit, EM_LINEFROMCHAR, start, 0);
        LRESULT line_start_char = SendMessageW(hEdit, EM_LINEINDEX, line_index, 0);
        LRESULT col_index = start - line_start_char;
        int total_chars = GetWindowTextLengthW(hEdit);

        wchar_t pos_text[64];
        swprintf_s(pos_text, L"  Ln %ld, Col %ld", static_cast<long>(line_index + 1), static_cast<long>(col_index + 1));
        SendMessageW(hStatus, SB_SETTEXTW, 0, reinterpret_cast<LPARAM>(pos_text));

        wchar_t count_text[64];
        swprintf_s(count_text, L"  %d characters", total_chars);
        SendMessageW(hStatus, SB_SETTEXTW, 1, reinterpret_cast<LPARAM>(count_text));

        SendMessageW(hStatus, SB_SETTEXTW, 2, reinterpret_cast<LPARAM>(L"  Windows (CRLF)"));
        SendMessageW(hStatus, SB_SETTEXTW, 3, reinterpret_cast<LPARAM>(L"  UTF-8"));

        const wchar_t* state_text = file_handler.IsDirty() ? L"  Modified" : L"  Saved";
        SendMessageW(hStatus, SB_SETTEXTW, 4, reinterpret_cast<LPARAM>(state_text));
    }
};

static void CreateAppMenu(HWND hwnd, const AppState& state) {
    HMENU hMenuBar = CreateMenu();

    // File Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_NEW, L"&New\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_SAVE_AS, L"Save &As...\tCtrl+Shift+S");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_PRINT, L"&Print...\tCtrl+P");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit\tAlt+F4");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hFileMenu), L"&File");

    // Edit Menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_CUT, L"Cu&t\tCtrl+X");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_COPY, L"&Copy\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_PASTE, L"&Paste\tCtrl+V");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, ID_EDIT_SELECT_ALL, L"Select &All\tCtrl+A");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hEditMenu), L"&Edit");

    // View Menu
    HMENU hViewMenu = CreatePopupMenu();
    UINT wrapFlags = MF_STRING | (state.options_handler.IsWordWrap() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, wrapFlags, ID_VIEW_WORD_WRAP, L"&Word Wrap\tCtrl+W");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_FONT, L"Choose &Font...");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_IN, L"Zoom &In\tCtrl++");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_OUT, L"Zoom &Out\tCtrl+-");
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_ZOOM_RESET, L"&Reset Zoom\tCtrl+0");
    AppendMenuW(hViewMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hViewMenu, MF_STRING, ID_VIEW_THEME_TOGGLE,
                state.options_handler.IsDarkMode() ? L"Switch to &Light Theme" : L"Switch to &Dark Theme");
    UINT statusFlags = MF_STRING | (state.options_handler.IsStatusBarVisible() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(hViewMenu, statusFlags, ID_VIEW_STATUS_BAR, L"&Status Bar");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hViewMenu), L"&View");

    // Help Menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, ID_HELP_ABOUT, L"&About Salt...");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hHelpMenu), L"&Help");

    SetMenu(hwnd, hMenuBar);
}

static void RecreateEditControl(HWND hwnd, AppState* state) {
    if (!state) return;

    // Save existing text and cursor position
    int len = state->hEdit ? GetWindowTextLengthW(state->hEdit) : 0;
    std::wstring buffer;
    if (len > 0) {
        buffer.resize(len);
        GetWindowTextW(state->hEdit, &buffer[0], len + 1);
    }

    if (state->hEdit) {
        DestroyWindow(state->hEdit);
    }

    DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL;
    if (!state->options_handler.IsWordWrap()) {
        style |= WS_HSCROLL | ES_AUTOHSCROLL;
    }

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    state->hEdit = CreateWindowExW(
        0, L"EDIT", buffer.c_str(), style,
        0, 0, rcClient.right, rcClient.bottom,
        hwnd, reinterpret_cast<HMENU>(IDC_MAIN_EDIT),
        GetModuleHandle(NULL), NULL);

    // Padding margins for modern comfortable reading
    SendMessageW(state->hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

    // Tab size: 4 spaces equivalent
    int tabStops = 16;
    SendMessageW(state->hEdit, EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&tabStops));

    state->file_handler.SetEditHandle(state->hEdit);
    state->options_handler.SetEditHandle(state->hEdit);
}

static HACCEL CreateAppAccelerators() {
    ACCEL accels[] = {
        {FCONTROL | FVIRTKEY, 'N', ID_FILE_NEW},
        {FCONTROL | FVIRTKEY, 'O', ID_FILE_OPEN},
        {FCONTROL | FVIRTKEY, 'S', ID_FILE_SAVE},
        {FCONTROL | FSHIFT | FVIRTKEY, 'S', ID_FILE_SAVE_AS},
        {FCONTROL | FVIRTKEY, 'P', ID_FILE_PRINT},
        {FCONTROL | FVIRTKEY, 'W', ID_VIEW_WORD_WRAP},
        {FCONTROL | FVIRTKEY, 'Z', ID_EDIT_UNDO},
        {FCONTROL | FVIRTKEY, 'A', ID_EDIT_SELECT_ALL},
        {FCONTROL | FVIRTKEY, VK_OEM_PLUS, ID_VIEW_ZOOM_IN},
        {FCONTROL | FVIRTKEY, VK_ADD, ID_VIEW_ZOOM_IN},
        {FCONTROL | FVIRTKEY, VK_OEM_MINUS, ID_VIEW_ZOOM_OUT},
        {FCONTROL | FVIRTKEY, VK_SUBTRACT, ID_VIEW_ZOOM_OUT},
        {FCONTROL | FVIRTKEY, '0', ID_VIEW_ZOOM_RESET},
        {FCONTROL | FVIRTKEY, VK_NUMPAD0, ID_VIEW_ZOOM_RESET},
    };
    return CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AppState* state = reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (uMsg) {
        case WM_CREATE: {
            state = new AppState();
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

            EnableWindowDarkMode(hwnd, true);

            // Create Status Bar
            state->hStatus = CreateWindowExW(
                0, STATUSCLASSNAMEW, NULL,
                WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                0, 0, 0, 0,
                hwnd, reinterpret_cast<HMENU>(IDC_MAIN_STATUS),
                GetModuleHandle(NULL), NULL);

            int statusParts[] = {160, 310, 440, 540, -1};
            SendMessageW(state->hStatus, SB_SETPARTS, 5, reinterpret_cast<LPARAM>(statusParts));

            // Create Edit Control
            RecreateEditControl(hwnd, state);
            state->options_handler.Init(hwnd, state->hEdit);

            CreateAppMenu(hwnd, *state);
            state->UpdateTitle(hwnd);
            state->UpdateStatusBar();

            SetFocus(state->hEdit);
            return 0;
        }

        case WM_SIZE: {
            if (!state) break;
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            int statusHeight = 0;
            if (state->hStatus && state->options_handler.IsStatusBarVisible()) {
                SendMessageW(state->hStatus, WM_SIZE, wParam, lParam);
                RECT rcStatus;
                GetWindowRect(state->hStatus, &rcStatus);
                statusHeight = rcStatus.bottom - rcStatus.top;
            }

            if (state->hEdit) {
                MoveWindow(state->hEdit, 0, 0, width, height - statusHeight, TRUE);
            }
            return 0;
        }

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            if (state && (reinterpret_cast<HWND>(lParam) == state->hEdit)) {
                HDC hdc = reinterpret_cast<HDC>(wParam);
                SetBkColor(hdc, state->options_handler.GetBackgroundColor());
                SetTextColor(hdc, state->options_handler.GetTextColor());
                return reinterpret_cast<LRESULT>(state->options_handler.GetBackgroundBrush());
            }
            break;
        }

        case WM_COMMAND: {
            if (!state) break;

            // Edit control notifications
            if (HIWORD(wParam) == EN_CHANGE && reinterpret_cast<HWND>(lParam) == state->hEdit) {
                if (!state->file_handler.IsDirty()) {
                    state->file_handler.SetDirty(true);
                    state->UpdateTitle(hwnd);
                }
                state->UpdateStatusBar();
                return 0;
            }

            switch (LOWORD(wParam)) {
                case ID_FILE_NEW:
                    if (state->file_handler.New(hwnd)) {
                        state->UpdateTitle(hwnd);
                        state->UpdateStatusBar();
                    }
                    break;

                case ID_FILE_OPEN:
                    if (state->file_handler.Open(hwnd)) {
                        state->UpdateTitle(hwnd);
                        state->UpdateStatusBar();
                    }
                    break;

                case ID_FILE_SAVE:
                    if (state->file_handler.Save(hwnd)) {
                        state->UpdateTitle(hwnd);
                        state->UpdateStatusBar();
                    }
                    break;

                case ID_FILE_SAVE_AS:
                    if (state->file_handler.SaveAs(hwnd)) {
                        state->UpdateTitle(hwnd);
                        state->UpdateStatusBar();
                    }
                    break;

                case ID_FILE_PRINT:
                    state->file_handler.Print(hwnd);
                    break;

                case ID_FILE_EXIT:
                    SendMessageW(hwnd, WM_CLOSE, 0, 0);
                    break;

                case ID_EDIT_UNDO:
                    SendMessageW(state->hEdit, EM_UNDO, 0, 0);
                    break;

                case ID_EDIT_CUT:
                    SendMessageW(state->hEdit, WM_CUT, 0, 0);
                    break;

                case ID_EDIT_COPY:
                    SendMessageW(state->hEdit, WM_COPY, 0, 0);
                    break;

                case ID_EDIT_PASTE:
                    SendMessageW(state->hEdit, WM_PASTE, 0, 0);
                    break;

                case ID_EDIT_SELECT_ALL:
                    SendMessageW(state->hEdit, EM_SETSEL, 0, -1);
                    break;

                case ID_VIEW_WORD_WRAP: {
                    state->options_handler.ToggleWordWrap();
                    RecreateEditControl(hwnd, state);
                    CreateAppMenu(hwnd, *state);
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    SendMessageW(hwnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
                    SetFocus(state->hEdit);
                    break;
                }

                case ID_VIEW_FONT:
                    state->options_handler.ChooseFontDialog();
                    break;

                case ID_VIEW_ZOOM_IN:
                    state->options_handler.ZoomIn();
                    break;

                case ID_VIEW_ZOOM_OUT:
                    state->options_handler.ZoomOut();
                    break;

                case ID_VIEW_ZOOM_RESET:
                    state->options_handler.ZoomReset();
                    break;

                case ID_VIEW_THEME_TOGGLE:
                    state->options_handler.ToggleTheme();
                    EnableWindowDarkMode(hwnd, state->options_handler.IsDarkMode());
                    CreateAppMenu(hwnd, *state);
                    break;

                case ID_VIEW_STATUS_BAR: {
                    bool visible = state->options_handler.ToggleStatusBar();
                    ShowWindow(state->hStatus, visible ? SW_SHOW : SW_HIDE);
                    CreateAppMenu(hwnd, *state);
                    RECT rc;
                    GetClientRect(hwnd, &rc);
                    SendMessageW(hwnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
                    break;
                }

                case ID_HELP_ABOUT:
                    MessageBoxW(hwnd,
                                L"Salt Text Editor v0.1.0\n\n"
                                L"A lightweight, modern Windows text editor.\n"
                                L"Built with Pure C++ and Win32.\n\n"
                                L"Features:\n"
                                L"• Immersive Dark Theme & Crisp Typography\n"
                                L"• UTF-8 / UTF-16 Multi-Encoding Support\n"
                                L"• Standalone, zero external dependencies",
                                L"About Salt", MB_OK | MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_SETFOCUS:
            if (state && state->hEdit) {
                SetFocus(state->hEdit);
                return 0;
            }
            break;

        case WM_CLOSE:
            if (state) {
                if (!state->file_handler.PromptSaveIfDirty(hwnd)) {
                    return 0; // Cancel close
                }
            }
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            if (state) {
                delete state;
                SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            }
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    // Enable High DPI Awareness for sharp text on 4K / scaled displays
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI* fnSetDpi)(DPI_AWARENESS_CONTEXT);
        fnSetDpi pfnSetDpi = reinterpret_cast<fnSetDpi>(GetProcAddress(hUser32, "SetProcessDpiAwarenessContext"));
        if (pfnSetDpi) {
            pfnSetDpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }

    // Initialize COM for modern IFileOpenDialog / IFileSaveDialog
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Initialize Common Controls v6
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    const wchar_t CLASS_NAME[] = L"SaltTextEditorWindowClass";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Untitled - Salt Text Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 680,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    HACCEL hAccel = CreateAppAccelerators();

    MSG msg{};
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!TranslateAcceleratorW(hwnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (hAccel) {
        DestroyAcceleratorTable(hAccel);
    }

    CoUninitialize();
    return static_cast<int>(msg.wParam);
}
