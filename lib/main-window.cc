#include "main-window.h"
#include "lib/context.h"
#include "lib/menu-bar.h"
#include "resource.h"

#include <dwmapi.h>
#include <objbase.h>
#include <shobjidl_core.h>
#include <uxtheme.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#define DWMSBT_MAINWINDOW 2
#endif

// UXTheme dark mode undocumented helper functions
typedef enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max } PreferredAppMode;

typedef PreferredAppMode(WINAPI* fnSetPreferredAppMode)(PreferredAppMode app_mode);
typedef BOOL(WINAPI* fnAllowDarkModeForWindow)(HWND hwnd, BOOL allow);

bool MainWindow::RegisterClass(HINSTANCE instance) {
    WNDCLASSEXW wnd_class{};
    wnd_class.cbSize = sizeof(WNDCLASSEXW);
    wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc = MainWindow::StaticWndProc;
    wnd_class.hInstance = instance;
    wnd_class.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wnd_class.hIconSm = static_cast<HICON>(
        LoadImageW(instance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
    wnd_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wnd_class.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wnd_class.lpszClassName = kClassName;
    return RegisterClassExW(&wnd_class) != 0;
}

HACCEL MainWindow::CreateAppAccelerators() {
    ACCEL accels[] = {
        {FCONTROL | FVIRTKEY, 'N', Context::Command::FileNew},
        {FCONTROL | FVIRTKEY, 'O', Context::Command::FileOpen},
        {FCONTROL | FVIRTKEY, 'S', Context::Command::FileSave},
        {FCONTROL | FSHIFT | FVIRTKEY, 'S', Context::Command::FileSaveAs},
        {FCONTROL | FVIRTKEY, 'P', Context::Command::FilePrint},
        {FCONTROL | FVIRTKEY, 'W', Context::Command::ViewWordWrap},
        {FCONTROL | FVIRTKEY, 'Z', Context::Command::EditUndo},
        {FCONTROL | FVIRTKEY, 'A', Context::Command::EditSelectAll},
        {FCONTROL | FVIRTKEY, VK_OEM_PLUS, Context::Command::ViewZoomIn},
        {FCONTROL | FVIRTKEY, VK_ADD, Context::Command::ViewZoomIn},
        {FCONTROL | FVIRTKEY, VK_OEM_MINUS, Context::Command::ViewZoomOut},
        {FCONTROL | FVIRTKEY, VK_SUBTRACT, Context::Command::ViewZoomOut},
        {FCONTROL | FVIRTKEY, '0', Context::Command::ViewZoomReset},
        {FCONTROL | FVIRTKEY, VK_NUMPAD0, Context::Command::ViewZoomReset},
    };
    return CreateAcceleratorTableW(accels, sizeof(accels) / sizeof(accels[0]));
}

HWND MainWindow::Create(HINSTANCE instance, int show_state) {
    // Create window
    try {
        hwnd_ =
            CreateWindowExW(0, kClassName, L"Untitled - Salt Text Editor", WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT, CW_USEDEFAULT, 1000, 680, NULL, NULL, instance, this);
    } catch (...) {
        SALT_PANIC("Could not get window handle!");
    }

    // Render
    if (hwnd_) {
        ShowWindow(hwnd_, show_state);
        UpdateWindow(hwnd_);
    }
    return hwnd_;
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM param_w, LPARAM param_l) {
    MainWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* create_struct = reinterpret_cast<CREATESTRUCTW*>(param_l);
        self = reinterpret_cast<MainWindow*>(create_struct->lpCreateParams);
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
    HMODULE uxtheme_module = GetModuleHandleW(L"uxtheme.dll");
    if (uxtheme_module) {
        fnSetPreferredAppMode set_preferred_app_mode = reinterpret_cast<fnSetPreferredAppMode>(
            GetProcAddress(uxtheme_module, MAKEINTRESOURCEA(135)));
        if (set_preferred_app_mode) {
            set_preferred_app_mode(enable ? AllowDark : Default);
        }
        fnAllowDarkModeForWindow allow_dark_mode_for_window =
            reinterpret_cast<fnAllowDarkModeForWindow>(
                GetProcAddress(uxtheme_module, MAKEINTRESOURCEA(133)));
        if (allow_dark_mode_for_window) {
            allow_dark_mode_for_window(hwnd_, enable);
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

void MainWindow::SetMenuBar() {
    menu_bar_.Create(options_handler_);
    HMENU menu_handle = menu_bar_.GetMenuHandle();
    bool set_menu_success = SetMenu(hwnd_, menu_handle);
    if (!set_menu_success) {
        SALT_PANIC("Failed to set menu!");
    }
    DrawMenuBar(hwnd_);
}

void MainWindow::RecreateEditControl() {
    int text_len = edit_hwnd_ ? GetWindowTextLengthW(edit_hwnd_) : 0;
    std::wstring buffer;
    if (text_len > 0) {
        buffer.resize(text_len);
        GetWindowTextW(edit_hwnd_, &buffer[0], text_len + 1);
    }

    if (edit_hwnd_) {
        DestroyWindow(edit_hwnd_);
    }

    DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL;
    if (!options_handler_.IsWordWrap()) {
        style |= WS_HSCROLL | ES_AUTOHSCROLL;
    }

    RECT client_rect;
    GetClientRect(hwnd_, &client_rect);

    edit_hwnd_ = CreateWindowExW(
        0, L"EDIT", buffer.c_str(), style, 0, 0, client_rect.right, client_rect.bottom, hwnd_,
        reinterpret_cast<HMENU>(Context::ControlId::MainEdit), GetModuleHandle(NULL), NULL);

    // Padding margins for comfortable reading
    SendMessageW(edit_hwnd_, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(16, 16));

    // Tab size: 4 spaces equivalent
    int tab_stops = 16;
    SendMessageW(edit_hwnd_, EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&tab_stops));

    // Remove the default 30,000 / 64KB text limit for the edit control
    SendMessageW(edit_hwnd_, EM_SETLIMITTEXT, 0, 0);

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
            status_hwnd_ =
                CreateWindowExW(0, STATUSCLASSNAMEW, NULL, status_style, 0, 0, 0, 0, hwnd_,
                                reinterpret_cast<HMENU>(Context::ControlId::MainStatus),
                                GetModuleHandle(NULL), NULL);

            int status_parts[] = {160, 310, 440, 540, -1};
            SendMessageW(status_hwnd_, SB_SETPARTS, 5, reinterpret_cast<LPARAM>(status_parts));

            // Create Edit Control
            RecreateEditControl();
            options_handler_.Init(hwnd_, edit_hwnd_);

            MainWindow::SetMenuBar();

            UpdateTitle();
            UpdateStatusBar();

            SetFocus(edit_hwnd_);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(param_l);
            int height = HIWORD(param_l);

            int status_height = 0;
            if (status_hwnd_ && options_handler_.IsStatusBarVisible()) {
                SendMessageW(status_hwnd_, WM_SIZE, param_w, param_l);
                RECT status_rect;
                GetWindowRect(status_hwnd_, &status_rect);
                status_height = status_rect.bottom - status_rect.top;
            }

            if (edit_hwnd_) {
                MoveWindow(edit_hwnd_, 0, 0, width, height - status_height, TRUE);
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

            Context::Command cmd = static_cast<Context::Command>(LOWORD(param_w));
            switch (cmd) {
                case Context::Command::FileNew:
                    if (file_handler_.New(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Context::Command::FileOpen:
                    if (file_handler_.Open(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Context::Command::FileSave:
                    if (file_handler_.Save(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Context::Command::FileSaveAs:
                    if (file_handler_.SaveAs(hwnd_)) {
                        UpdateTitle();
                        UpdateStatusBar();
                    }
                    break;

                case Context::Command::FilePrint:
                    file_handler_.Print(hwnd_);
                    break;

                case Context::Command::FileExit:
                    SendMessageW(hwnd_, WM_CLOSE, 0, 0);
                    break;

                case Context::Command::EditUndo:
                    SendMessageW(edit_hwnd_, EM_UNDO, 0, 0);
                    break;

                case Context::Command::EditCut:
                    SendMessageW(edit_hwnd_, WM_CUT, 0, 0);
                    break;

                case Context::Command::EditCopy:
                    SendMessageW(edit_hwnd_, WM_COPY, 0, 0);
                    break;

                case Context::Command::EditPaste:
                    SendMessageW(edit_hwnd_, WM_PASTE, 0, 0);
                    break;

                case Context::Command::EditSelectAll:
                    SendMessageW(edit_hwnd_, EM_SETSEL, 0, -1);
                    break;

                case Context::Command::ViewWordWrap: {
                    options_handler_.ToggleWordWrap();
                    RecreateEditControl();
                    MainWindow::SetMenuBar();
                    RECT client_rect;
                    GetClientRect(hwnd_, &client_rect);
                    SendMessageW(hwnd_, WM_SIZE, 0,
                                 MAKELPARAM(client_rect.right, client_rect.bottom));
                    SetFocus(edit_hwnd_);
                    break;
                }

                case Context::Command::ViewFont:
                    options_handler_.ChooseFontDialog();
                    break;

                case Context::Command::ViewZoomIn:
                    options_handler_.ZoomIn();
                    break;

                case Context::Command::ViewZoomOut:
                    options_handler_.ZoomOut();
                    break;

                case Context::Command::ViewZoomReset:
                    options_handler_.ZoomReset();
                    break;

                case Context::Command::ViewThemeToggle:
                    options_handler_.ToggleTheme();
                    EnableDarkMode(options_handler_.IsDarkMode());
                    MainWindow::SetMenuBar();
                    break;

                case Context::Command::ViewStatusBar: {
                    bool visible = options_handler_.ToggleStatusBar();
                    ShowWindow(status_hwnd_, visible ? SW_SHOW : SW_HIDE);
                    MainWindow::SetMenuBar();
                    RECT client_rect;
                    GetClientRect(hwnd_, &client_rect);
                    SendMessageW(hwnd_, WM_SIZE, 0,
                                 MAKELPARAM(client_rect.right, client_rect.bottom));
                    break;
                }

                case Context::Command::HelpAbout:
                    MessageBoxW(hwnd_,
                                L"Salt Text Editor v0.1.1\n\n"
                                L"A simple text editor for Windows.\n"
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
