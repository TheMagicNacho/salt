#include "file-handler.h"
#include <shobjidl.h>

namespace {

/// Normalizes line endings in the given text to \r\n format.
/// Used for managing CLRF files appropriately.r
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

    std::wstring wide_buffer(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, raw_buffer, raw_len, &wide_buffer[0], wideLen);

    if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
        wide_buffer.pop_back();
    }

    return wide_buffer;
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

    std::wstring wide_buffer(char_count, L'\0');

    for (int i = 0; i < char_count; ++i) {
        const unsigned char* p =
            reinterpret_cast<const unsigned char*>(raw_buffer + offset + (i * sizeof(wchar_t)));
        wchar_t ch = static_cast<wchar_t>(static_cast<unsigned short>(p[0]) |
                                          (static_cast<unsigned short>(p[1]) << 8));
        wide_buffer[i] = ch;
    }

    return wide_buffer;
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
        std::wstring wide_buffer(wideLen, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw_buffer, dw_size, &wide_buffer[0],
                            wideLen);

        if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
            wide_buffer.pop_back();
        }
        return wide_buffer;
    }

    wideLen = MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, NULL, 0);
    if (wideLen > 0) {
        std::wstring wide_buffer(wideLen, L'\0');
        MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, &wide_buffer[0], wideLen);

        if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
            wide_buffer.pop_back();
        }
        return wide_buffer;
    }

    return L"";
}

std::wstring GetOpenFilePathModern(HWND hwnd) {
    std::wstring result;
    IFileOpenDialog* open_file = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                                  reinterpret_cast<void**>(&open_file));
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC rgSpec[] = {
            {L"All Supported Files (*.txt, *.md, *.json, *.cpp, *.h)",
             L"*.txt;*.md;*.json;*.cpp;*.h;*.c;*.hpp;*.bzl;*.py;*.js;*.ts;*.html;*.css"},
            {L"Text Files (*.txt)", L"*.txt"},
            {L"Markdown Files (*.md)", L"*.md"},
            {L"All Files (*.*)", L"*.*"}};
        open_file->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);

        hr = open_file->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* pItem = nullptr;
            hr = open_file->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR file_path = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &file_path);
                if (SUCCEEDED(hr)) {
                    result = file_path;
                    CoTaskMemFree(file_path);
                }
                pItem->Release();
            }
        }
        open_file->Release();
    } else {
        // Fallback to legacy GetOpenFileName
        OPENFILENAME ofn = {sizeof(OPENFILENAME)};
        wchar_t file_size[MAX_PATH] = {0};
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = file_size;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = L"All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileName(&ofn)) {
            result = file_size;
        }
    }
    return result;
}

std::wstring GetSaveFilePathModern(HWND hwnd) {
    std::wstring result;
    IFileSaveDialog* file_save = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog,
                                  reinterpret_cast<void**>(&file_save));
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC spec[] = {{L"Text Files (*.txt)", L"*.txt"},
                                    {L"Markdown Files (*.md)", L"*.md"},
                                    {L"All Files (*.*)", L"*.*"}};
        file_save->SetFileTypes(ARRAYSIZE(spec), spec);
        file_save->SetDefaultExtension(L"txt");

        hr = file_save->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = file_save->GetResult(&item);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath = nullptr;
                hr = item->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    result = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                item->Release();
            }
        }
        file_save->Release();
    } else {
        // Fallback to legacy GetSaveFileName
        OPENFILENAME ofn = {sizeof(OPENFILENAME)};
        wchar_t file_size[MAX_PATH] = {0};
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = file_size;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileName(&ofn)) {
            result = file_size;
        }
    }
    return result;
}
}  // namespace

FileHandler::FileHandler(HWND edit_hwnd) : text_edit_(edit_hwnd), is_dirty_(false) {}

void FileHandler::SetEditHandle(HWND hEdit) { text_edit_ = hEdit; }

void FileHandler::SetDirty(bool dirty) { is_dirty_ = dirty; }

bool FileHandler::IsDirty() const { return is_dirty_; }

std::wstring FileHandler::GetFilePath() const { return current_file_path_; }

std::wstring FileHandler::GetFileName() const {
    if (current_file_path_.empty()) {
        return L"Untitled";
    }
    size_t last_slash = current_file_path_.find_last_of(L"\\/");
    if (last_slash != std::wstring::npos) {
        return current_file_path_.substr(last_slash + 1);
    }
    return current_file_path_;
}

bool FileHandler::PromptSaveIfDirty(HWND hwnd) {
    if (!is_dirty_) return true;

    std::wstring msg = L"Do you want to save changes to " + GetFileName() + L"?";
    int result =
        MessageBoxW(hwnd, msg.c_str(), L"Salt Text Editor", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (result == IDYES) {
        return Save(hwnd);
    } else if (result == IDNO) {
        return true;
    }
    return false;  // IDCANCEL
}

bool FileHandler::New(HWND hwnd) {
    if (!PromptSaveIfDirty(hwnd)) return false;

    current_file_path_.clear();
    is_dirty_ = false;
    if (text_edit_) {
        SetWindowTextW(text_edit_, L"");
    }
    return true;
}

bool FileHandler::Open(HWND hwnd) {
    if (!PromptSaveIfDirty(hwnd)) return false;

    std::wstring selected_file = GetOpenFilePathModern(hwnd);
    if (selected_file.empty()) return false;

    HANDLE file_handle = CreateFileW(selected_file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        MessageBoxW(hwnd, L"Failed to open file.", L"Error", MB_ICONERROR);
        return false;
    }

    DWORD dw_size = GetFileSize(file_handle, NULL);
    if (dw_size == INVALID_FILE_SIZE) {
        CloseHandle(file_handle);
        return false;
    }

    char* raw_buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dw_size + 1);
    DWORD dw_read = 0;
    bool success = false;

    if (ReadFile(file_handle, raw_buffer, dw_size, &dw_read, NULL)) {
        std::wstring wideBuffer = DecodeTextFile(raw_buffer, dw_size);
        wideBuffer = NormalizeLineEndings(wideBuffer);

        if (text_edit_) {
            SetWindowTextW(text_edit_, wideBuffer.c_str());
            current_file_path_ = selected_file;
            is_dirty_ = false;
            success = true;
        }
    }

    HeapFree(GetProcessHeap(), 0, raw_buffer);
    CloseHandle(file_handle);
    return success;
}

bool FileHandler::Save(HWND hwnd) {
    if (current_file_path_.empty()) {
        return SaveAs(hwnd);
    }

    if (!text_edit_) return false;

    HANDLE hFile = CreateFileW(current_file_path_.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        MessageBoxW(hwnd, L"Failed to save file.", L"Error", MB_ICONERROR);
        return false;
    }

    int len = GetWindowTextLengthW(text_edit_);
    wchar_t* buffer =
        (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len + 1) * sizeof(wchar_t));
    GetWindowTextW(text_edit_, buffer, len + 1);

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
    char* utf8Buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf8Len);
    WideCharToMultiByte(CP_UTF8, 0, buffer, len, utf8Buffer, utf8Len, NULL, NULL);

    DWORD dwWritten = 0;
    BOOL writeSuccess = WriteFile(hFile, utf8Buffer, utf8Len, &dwWritten, NULL);

    HeapFree(GetProcessHeap(), 0, utf8Buffer);
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);

    if (writeSuccess) {
        is_dirty_ = false;
        return true;
    }
    return false;
}

bool FileHandler::SaveAs(HWND hwnd) {
    std::wstring selected_file = GetSaveFilePathModern(hwnd);
    if (selected_file.empty()) return false;

    current_file_path_ = selected_file;
    return Save(hwnd);
}

void FileHandler::Print(HWND hwnd) {
    PRINTDLGW pd = {sizeof(PRINTDLGW)};
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (PrintDlgW(&pd)) {
        DOCINFOW di = {sizeof(DOCINFOW), L"Salt Text Document"};
        if (StartDocW(pd.hDC, &di) > 0) {
            StartPage(pd.hDC);

            if (text_edit_) {
                int len = GetWindowTextLengthW(text_edit_);
                wchar_t* buffer = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                                      (len + 1) * sizeof(wchar_t));
                GetWindowTextW(text_edit_, buffer, len + 1);

                RECT rect = {100, 100, 2000, 3000};
                DrawTextW(pd.hDC, buffer, -1, &rect, DT_LEFT | DT_WORDBREAK);

                HeapFree(GetProcessHeap(), 0, buffer);
            }

            EndPage(pd.hDC);
            EndDoc(pd.hDC);
        }
        DeleteDC(pd.hDC);
    }
}
