#include "file-handler.h"

namespace {
std::wstring NormalizeLineEndings(const std::wstring& text) {
    std::wstring normalized;
    normalized.reserve(text.size() * 6 / 5);

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == L'\r') {
            normalized.push_back(L'\r');
            // Look ahead: if next char is '\n', consume it to avoid double-adding
            if (i + 1 < text.size() && text[i + 1] == L'\n') {
                normalized.push_back(L'\n');
                ++i;
            } else {
                // Lone \r -> convert to \r\n
                normalized.push_back(L'\n');
            }
        } else if (text[i] == L'\n') {
            // Lone \n -> convert to \r\n
            normalized.push_back(L'\r');
            normalized.push_back(L'\n');
        } else {
            normalized.push_back(text[i]);
        }
    }

    return normalized;
}
std::wstring Utf8ToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, raw_buffer, raw_len, NULL, 0);
    if (wideLen <= 0) return L"";

    std::wstring wideBuffer(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, raw_buffer, raw_len, &wideBuffer[0], wideLen);

    if (!wideBuffer.empty() && wideBuffer.back() == L'\0') {
        wideBuffer.pop_back();
    }

    return wideBuffer;
}

std::wstring Utf16LEToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int offset = 0;
    if (raw_len >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFE) {
        offset = 2;
    }

    int char_count = (raw_len - offset) / sizeof(wchar_t);
    if (char_count <= 0) return L"";

    std::wstring wideBuffer(char_count, L'\0');

    for (int i = 0; i < char_count; ++i) {
        const unsigned char* p =
            reinterpret_cast<const unsigned char*>(raw_buffer + offset + (i * sizeof(wchar_t)));
        wchar_t ch = static_cast<wchar_t>(static_cast<unsigned short>(p[0]) |
                                          (static_cast<unsigned short>(p[1]) << 8));
        wideBuffer[i] = ch;
    }

    return wideBuffer;
}

std::wstring Utf16BEToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int offset = 0;
    if (raw_len >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFE &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFF) {
        offset = 2;
    }

    int char_count = (raw_len - offset) / sizeof(wchar_t);
    if (char_count <= 0) return L"";

    std::wstring wideBuffer(char_count, L'\0');

    for (int i = 0; i < char_count; ++i) {
        const unsigned char* p =
            reinterpret_cast<const unsigned char*>(raw_buffer + offset + (i * sizeof(wchar_t)));
        wchar_t ch = static_cast<wchar_t>((static_cast<unsigned short>(p[0]) << 8) |
                                          static_cast<unsigned short>(p[1]));
        wideBuffer[i] = ch;
    }

    return wideBuffer;
}

std::wstring DecodeTextFile(const char* raw_buffer, DWORD dw_size) {
    if (dw_size == 0) return L"";

    if (dw_size >= 3 && static_cast<unsigned char>(raw_buffer[0]) == 0xEF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xBB &&
        static_cast<unsigned char>(raw_buffer[2]) == 0xBF) {
        return Utf8ToWide(raw_buffer + 3, static_cast<int>(dw_size - 3));
    }

    if (dw_size >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFE) {
        return Utf16LEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    if (dw_size >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFE &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFF) {
        return Utf16BEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    size_t zero_count = 0;
    for (DWORD i = 0; i < dw_size; ++i) {
        if (raw_buffer[i] == 0) ++zero_count;
    }
    if (zero_count > dw_size / 4) {
        return Utf16LEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    int wideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw_buffer, dw_size, NULL, 0);
    if (wideLen > 0) {
        std::wstring wideBuffer(wideLen, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw_buffer, dw_size, &wideBuffer[0],
                            wideLen);

        if (!wideBuffer.empty() && wideBuffer.back() == L'\0') {
            wideBuffer.pop_back();
        }
        return wideBuffer;
    }

    wideLen = MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, NULL, 0);
    if (wideLen > 0) {
        std::wstring wideBuffer(wideLen, L'\0');
        MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, &wideBuffer[0], wideLen);

        if (!wideBuffer.empty() && wideBuffer.back() == L'\0') {
            wideBuffer.pop_back();
        }
        return wideBuffer;
    }

    return L"";
}
}  // namespace

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
        current_file_path_ = szFile;

        HANDLE file_handle = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL, NULL);
        if (file_handle != INVALID_HANDLE_VALUE) {
            DWORD dw_size = GetFileSize(file_handle, NULL);
            if (dw_size != INVALID_FILE_SIZE) {
                // Read raw 8-bit bytes from the file
                char* raw_buffer =
                    (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dw_size + 1);
                DWORD dw_read;

                if (ReadFile(file_handle, raw_buffer, dw_size, &dw_read, NULL)) {
                    std::wstring wideBuffer = DecodeTextFile(raw_buffer, dw_size);
                    wideBuffer = NormalizeLineEndings(wideBuffer);

                    // if (!text_edit_) return;

                    SetWindowTextW(text_edit_, wideBuffer.c_str());
                }

                HeapFree(GetProcessHeap(), 0, raw_buffer);
            }
            CloseHandle(file_handle);
        }
    }
}
void FileHandler::Save(HWND hwnd) {
    // If we don't have an active file, fall back to "Save As"
    if (current_file_path_.empty()) {
        SaveAs(hwnd);
        return;
    }

    if (!text_edit_) return;

    // Open the existing file and overwrite it (CREATE_ALWAYS)
    HANDLE hFile = CreateFile(current_file_path_.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        int len = GetWindowTextLength(text_edit_);
        wchar_t* buffer =
            (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(wchar_t));
        GetWindowText(text_edit_, buffer, len + 1);

        // Convert UTF-16 back to UTF-8 for consistent file output
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
        char* utf8Buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf8Len);
        WideCharToMultiByte(CP_UTF8, 0, buffer, len, utf8Buffer, utf8Len, NULL, NULL);

        DWORD dwWritten;
        WriteFile(hFile, utf8Buffer, utf8Len, &dwWritten, NULL);

        HeapFree(GetProcessHeap(), 0, utf8Buffer);
        HeapFree(GetProcessHeap(), 0, buffer);
        CloseHandle(hFile);
    }
}

void FileHandler::SaveAs(HWND hwnd) {
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

// PRIVATE METHODS
