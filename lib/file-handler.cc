#include "file-handler.h"
#include "text-utils.h"
#include <shobjidl.h>

namespace {

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
            IShellItem* item = nullptr;
            hr = open_file->GetResult(&item);
            if (SUCCEEDED(hr)) {
                PWSTR file_path = nullptr;
                hr = item->GetDisplayName(SIGDN_FILESYSPATH, &file_path);
                if (SUCCEEDED(hr)) {
                    result = file_path;
                    CoTaskMemFree(file_path);
                }
                item->Release();
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
                                    {L"JSON (*.json)", L"*.json"},
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

void FileHandler::SetFilePath(const std::wstring& path) { current_file_path_ = path; }

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
        std::wstring wide_buffer = salt::DecodeTextFile(raw_buffer, dw_size);
        wide_buffer = salt::NormalizeLineEndings(wide_buffer);

        if (text_edit_) {
            SetWindowTextW(text_edit_, wide_buffer.c_str());
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

    int utf_8_len = WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
    char* utf_8_buff = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf_8_len);
    WideCharToMultiByte(CP_UTF8, 0, buffer, len, utf_8_buff, utf_8_len, NULL, NULL);

    DWORD written = 0;
    BOOL write_success = WriteFile(hFile, utf_8_buff, utf_8_len, &written, NULL);

    HeapFree(GetProcessHeap(), 0, utf_8_buff);
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);

    if (write_success) {
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
