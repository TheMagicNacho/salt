#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commdlg.h>
#include <lib/defaults.h>
#include <lib/file-handler.h>

#define ID_FILE_OPEN 1
#define ID_FILE_SAVE 2
#define ID_FILE_SAVE_AS 3
#define ID_FILE_PRINT 4
#define ID_FILE_EXIT 5

struct AppState {
    HWND hEdit = NULL;
    FileHandler fileHandler{NULL};
};

LRESULT CALLBACK WindowProc(HWND window_handle, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AppState* state = reinterpret_cast<AppState*>(GetWindowLongPtr(window_handle, GWLP_USERDATA));

    switch (uMsg) {
        case WM_CREATE: {
            state = new AppState();
            SetWindowLongPtr(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            // Create Menu
            HMENU menu_container = CreateMenu();
            HMENU file_menu = CreatePopupMenu();
            HMENU view_menu = CreatePopupMenu();
            // FILE MENU
            AppendMenu(file_menu, MF_STRING, ID_FILE_OPEN, L"&Open");
            AppendMenu(file_menu, MF_STRING, ID_FILE_SAVE, L"&Save");
            AppendMenu(file_menu, MF_STRING, ID_FILE_SAVE_AS, L"&Save As");
            AppendMenu(file_menu, MF_STRING, ID_FILE_PRINT, L"&Print");
            AppendMenu(file_menu, MF_SEPARATOR, 0, NULL);
            AppendMenu(file_menu, MF_STRING, ID_FILE_EXIT, L"E&xit");
            // OPTIONS MENU
            AppendMenu(view_menu, MF_STRING, 10, L"&Toggle Word Wrap");
            AppendMenu(view_menu, MF_STRING, 11, L"&Select Font");

            AppendMenu(menu_container, MF_POPUP, (UINT_PTR)file_menu, L"&File");
            AppendMenu(menu_container, MF_POPUP, (UINT_PTR)view_menu, L"&View");
            SetMenu(window_handle, menu_container);

            // Create EDIT Child Control
            state->hEdit = CreateWindowEx(0, L"EDIT", NULL,
                                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                              ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                          0, 0, 0, 0, window_handle, (HMENU)101,
                                          ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            state->fileHandler.SetEditHandle(state->hEdit);
            return 0;
        }

        case WM_SIZE: {
            if (state && state->hEdit) {
                MoveWindow(state->hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
            }
            return 0;
        }

        case WM_COMMAND: {
            if (!state && state->hEdit) break;
            switch (LOWORD(wParam)) {
                case ID_FILE_OPEN:
                    state->fileHandler.Open(window_handle);
                    break;
                case ID_FILE_SAVE:
                    state->fileHandler.Save(window_handle);
                    break;
                case ID_FILE_SAVE_AS:
                    state->fileHandler.SaveAs(window_handle);
                    break;
                case ID_FILE_PRINT:
                    state->fileHandler.Print(window_handle);
                    break;
                case ID_FILE_EXIT:
                    DestroyWindow(window_handle);
                    break;
            }
            return 0;
        }

        case WM_DESTROY:
            if (state) {
                delete state;
                SetWindowLongPtr(window_handle, GWLP_USERDATA, 0);
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(window_handle, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SaltTextEditorWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND window_handle =
        CreateWindowEx(0, CLASS_NAME, L"Salt Text Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                       CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (window_handle == NULL) return 0;

    ShowWindow(window_handle, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
