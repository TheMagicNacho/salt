#include "file-handler.h"

FileHandler::FileHandler(HWND edit_hwnd) : text_edit_(edit_hwnd) {}

void FileHandler::SetEditHandle(HWND hEdit) { text_edit_ = hEdit; }

void FileHandler::Open(HWND hwnd) {
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    wchar_t szFile[MAX_PATH] = {0};

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        HANDLE file_handle = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_handle != INVALID_HANDLE_VALUE) {
            DWORD dw_size = GetFileSize(file_handle, NULL);
            if (dw_size != INVALID_FILE_SIZE) {
                // 1. Read raw 8-bit bytes from the file
                char* raw_buffer =
                    (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dw_size + 1);
                DWORD dw_read;

                if (ReadFile(file_handle, raw_buffer, dw_size, &dw_read, NULL)) {
                    // 2. Calculate required size for the 16-bit wide character buffer
                    int wideLen = MultiByteToWideChar(CP_UTF8, 0, raw_buffer, dw_size, NULL, 0);

                    // 3. Allocate wide buffer and translate
                    wchar_t* wideBuffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                              (wideLen + 1) * sizeof(wchar_t));
                    MultiByteToWideChar(CP_UTF8, 0, raw_buffer, dw_size, wideBuffer, wideLen);

                    // 4. Send translated text to the EDIT control
                    if (!text_edit_) return;
                    SetWindowText(text_edit_, wideBuffer);

                    HeapFree(GetProcessHeap(), 0, wideBuffer);
                }
                HeapFree(GetProcessHeap(), 0, raw_buffer);
            }
            CloseHandle(file_handle);
        }
    }
}

void FileHandler::Save(HWND hwnd) {
    OPENFILENAME ofn = {sizeof(OPENFILENAME)};
    wchar_t szFile[MAX_PATH] = {0};

    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) {
        HANDLE hFile =
            CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            if (!text_edit_) return;  // DoPrintFile(hwnd);
            int len = GetWindowTextLength(text_edit_);
            wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                  (len + 1) * sizeof(wchar_t));
            GetWindowText(text_edit_, buffer, len + 1);

            DWORD dwWritten;
            WriteFile(hFile, buffer, len * sizeof(wchar_t), &dwWritten, NULL);

            HeapFree(GetProcessHeap(), 0, buffer);
            CloseHandle(hFile);
        }
    }
}

void FileHandler::Print(HWND hwnd) {
    PRINTDLG pd = {sizeof(PRINTDLG)};
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (PrintDlg(&pd)) {
        DOCINFO di = {sizeof(DOCINFO), L"Salt Text Document"};
        if (StartDoc(pd.hDC, &di) > 0) {
            StartPage(pd.hDC);

            if (!text_edit_) return;
            int len = GetWindowTextLength(text_edit_);
            wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                  (len + 1) * sizeof(wchar_t));
            GetWindowText(text_edit_, buffer, len + 1);

            RECT rect = {100, 100, 2000, 3000};  // Standard page printable area margin
            DrawText(pd.hDC, buffer, -1, &rect, DT_LEFT | DT_WORDBREAK);

            HeapFree(GetProcessHeap(), 0, buffer);

            EndPage(pd.hDC);
            EndDoc(pd.hDC);
        }
        DeleteDC(pd.hDC);
    }
}
