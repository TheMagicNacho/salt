#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commdlg.h>
#include <lib/hello-greet.h>
#include <lib/defaults.h>
#include <lib/file-handler.h>

#define ID_FILE_OPEN 1
#define ID_FILE_SAVE 2
#define ID_FILE_PRINT 3
#define ID_FILE_EXIT 4

struct AppState {
    HWND hEdit = NULL;
    FileHandler fileHandler{NULL};
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    AppState* state = reinterpret_cast<AppState*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg) {
        case WM_CREATE: {
            state = new AppState();
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
            // Create Menu
            HMENU hMenu = CreateMenu();
            HMENU hFileMenu = CreatePopupMenu();
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open");
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save As");
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_PRINT, L"&Print");
            AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
            AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
            SetMenu(hwnd, hMenu);

            // Create EDIT Child Control
            state->hEdit = CreateWindowEx(0, L"EDIT", NULL,
                                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
                                              ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                          0, 0, 0, 0, hwnd, (HMENU)101,
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
                    state->fileHandler.Open(hwnd);
                    break;
                case ID_FILE_SAVE:
                    state->fileHandler.Save(hwnd);
                    break;
                case ID_FILE_PRINT:
                    state->fileHandler.Print(hwnd);
                    break;
                case ID_FILE_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }

        case WM_DESTROY:
            if (state) {
                delete state;
                SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            }
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SaltTextEditorWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Salt Text Editor", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
