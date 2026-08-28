#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commdlg.h>

#define ID_FILE_OPEN  1
#define ID_FILE_SAVE  2
#define ID_FILE_PRINT 3
#define ID_FILE_EXIT  4

HWND g_hEdit = NULL;

void DoOpenFile(HWND hwnd) {
    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD dwSize = GetFileSize(hFile, NULL);
            if (dwSize != INVALID_FILE_SIZE) {
                // 1. Read raw 8-bit bytes from the file
                char* rawBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize + 1);
                DWORD dwRead;

                if (ReadFile(hFile, rawBuffer, dwSize, &dwRead, NULL)) {
                    // 2. Calculate required size for the 16-bit wide character buffer
                    int wideLen = MultiByteToWideChar(CP_UTF8, 0, rawBuffer, dwSize, NULL, 0);

                    // 3. Allocate wide buffer and translate
                    wchar_t* wideBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (wideLen + 1) * sizeof(wchar_t));
                    MultiByteToWideChar(CP_UTF8, 0, rawBuffer, dwSize, wideBuffer, wideLen);

                    // 4. Send translated text to the EDIT control
                    SetWindowText(g_hEdit, wideBuffer);

                    HeapFree(GetProcessHeap(), 0, wideBuffer);
                }
                HeapFree(GetProcessHeap(), 0, rawBuffer);
            }
            CloseHandle(hFile);
        }
    }
}

void DoSaveFile(HWND hwnd) {
    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    wchar_t szFile[MAX_PATH] = { 0 };

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            int len = GetWindowTextLength(g_hEdit);
            wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(wchar_t));
            GetWindowText(g_hEdit, buffer, len + 1);

            DWORD dwWritten;
            WriteFile(hFile, buffer, len * sizeof(wchar_t), &dwWritten, NULL);

            HeapFree(GetProcessHeap(), 0, buffer);
            CloseHandle(hFile);
        }
    }
}

void DoPrintFile(HWND hwnd) {
    PRINTDLG pd = { sizeof(PRINTDLG) };
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (PrintDlg(&pd)) {
        DOCINFO di = { sizeof(DOCINFO), L"Salt Text Document" };
        if (StartDoc(pd.hDC, &di) > 0) {
            StartPage(pd.hDC);

            int len = GetWindowTextLength(g_hEdit);
            wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(wchar_t));
            GetWindowText(g_hEdit, buffer, len + 1);

            RECT rect = { 100, 100, 2000, 3000 }; // Standard page printable area margin
            DrawText(pd.hDC, buffer, -1, &rect, DT_LEFT | DT_WORDBREAK);

            HeapFree(GetProcessHeap(), 0, buffer);

            EndPage(pd.hDC);
            EndDoc(pd.hDC);
        }
        DeleteDC(pd.hDC);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Create Menu
        HMENU hMenu = CreateMenu();
        HMENU hFileMenu = CreatePopupMenu();
        AppendMenu(hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open");
        AppendMenu(hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save...");
        AppendMenu(hFileMenu, MF_STRING, ID_FILE_PRINT, L"&Print...");
        AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
        SetMenu(hwnd, hMenu);

        // Create EDIT Child Control
        g_hEdit = CreateWindowEx(
            0, L"EDIT", NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            0, 0, 0, 0,
            hwnd, (HMENU)101, ((LPCREATESTRUCT)lParam)->hInstance, NULL
        );
        return 0;
    }

    case WM_SIZE: {
        // Resize child EDIT control to match main window client bounds
        MoveWindow(g_hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_FILE_OPEN:  DoOpenFile(hwnd); break;
        case ID_FILE_SAVE:  DoSaveFile(hwnd); break;
        case ID_FILE_PRINT: DoPrintFile(hwnd); break;
        case ID_FILE_EXIT:  DestroyWindow(hwnd); break;
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SaltTextEditorWindow";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Salt Text Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
