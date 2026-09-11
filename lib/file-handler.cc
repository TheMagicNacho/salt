#include "file-handler.h"
#include "text-utils.h"
#include <shobjidl.h>

namespace {

/// @brief Displays the modern Common Item Dialog (IFileOpenDialog) with fallback to legacy dialog.
/// @param hwnd Owner window handle.
/// @return Selected file path, or empty string if cancelled.
std::wstring GetOpenFilePathModern(HWND hwnd) {
    std::wstring result;
    IFileOpenDialog* open_dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog,
                                  reinterpret_cast<void**>(&open_dialog));
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC filter_specs[] = {
            {L"All Supported Files (*.txt, *.md, *.json, *.cpp, *.h)",
             L"*.txt;*.md;*.json;*.cpp;*.h;*.c;*.hpp;*.bzl;*.py;*.js;*.ts;*.html;*.css"},
            {L"Text Files (*.txt)", L"*.txt"},
            {L"Markdown Files (*.md)", L"*.md"},
            {L"All Files (*.*)", L"*.*"}};
        open_dialog->SetFileTypes(ARRAYSIZE(filter_specs), filter_specs);

        hr = open_dialog->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = open_dialog->GetResult(&item);
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
        open_dialog->Release();
    } else {
        // Fallback to legacy GetOpenFileName dialog
        OPENFILENAMEW open_file_name = {sizeof(OPENFILENAMEW)};
        wchar_t file_path_buffer[MAX_PATH] = {0};
        open_file_name.hwndOwner = hwnd;
        open_file_name.lpstrFile = file_path_buffer;
        open_file_name.nMaxFile = MAX_PATH;
        open_file_name.lpstrFilter = L"All Files (*.*)\0*.*\0Text Files (*.txt)\0*.txt\0";
        open_file_name.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileNameW(&open_file_name)) {
            result = file_path_buffer;
        }
    }
    return result;
}

/// @brief Displays the modern Common Item Dialog (IFileSaveDialog) with fallback to legacy dialog.
/// @param hwnd Owner window handle.
/// @return Selected destination file path, or empty string if cancelled.
std::wstring GetSaveFilePathModern(HWND hwnd) {
    std::wstring result;
    IFileSaveDialog* save_dialog = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog,
                                  reinterpret_cast<void**>(&save_dialog));
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC filter_specs[] = {{L"Text Files (*.txt)", L"*.txt"},
                                            {L"Markdown Files (*.md)", L"*.md"},
                                            {L"JSON (*.json)", L"*.json"},
                                            {L"All Files (*.*)", L"*.*"}};
        save_dialog->SetFileTypes(ARRAYSIZE(filter_specs), filter_specs);
        save_dialog->SetDefaultExtension(L"txt");

        hr = save_dialog->Show(hwnd);
        if (SUCCEEDED(hr)) {
            IShellItem* item = nullptr;
            hr = save_dialog->GetResult(&item);
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
        save_dialog->Release();
    } else {
        // Fallback to legacy GetSaveFileName dialog
        OPENFILENAMEW open_file_name = {sizeof(OPENFILENAMEW)};
        wchar_t file_path_buffer[MAX_PATH] = {0};
        open_file_name.hwndOwner = hwnd;
        open_file_name.lpstrFile = file_path_buffer;
        open_file_name.nMaxFile = MAX_PATH;
        open_file_name.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        open_file_name.Flags = OFN_OVERWRITEPROMPT;
        if (GetSaveFileNameW(&open_file_name)) {
            result = file_path_buffer;
        }
    }
    return result;
}
}  // namespace

FileHandler::FileHandler(HWND edit_hwnd) : text_edit_(edit_hwnd), is_dirty_(false) {}

void FileHandler::SetEditHandle(HWND edit_hwnd) { text_edit_ = edit_hwnd; }

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

    std::wstring message = L"Do you want to save changes to " + GetFileName() + L"?";
    int result =
        MessageBoxW(hwnd, message.c_str(), L"Salt Text Editor", MB_YESNOCANCEL | MB_ICONQUESTION);
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

    DWORD file_size = GetFileSize(file_handle, NULL);
    if (file_size == INVALID_FILE_SIZE) {
        CloseHandle(file_handle);
        return false;
    }

    char* raw_buffer =
        static_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, file_size + 1));
    DWORD bytes_read = 0;
    bool success = false;

    if (ReadFile(file_handle, raw_buffer, file_size, &bytes_read, NULL)) {
        std::wstring wide_buffer = salt::DecodeTextFile(raw_buffer, file_size);
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

    HANDLE file_handle = CreateFileW(current_file_path_.c_str(), GENERIC_WRITE, 0, NULL,
                                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        MessageBoxW(hwnd, L"Failed to save file.", L"Error", MB_ICONERROR);
        return false;
    }

    int text_len = GetWindowTextLengthW(text_edit_);
    wchar_t* text_buffer = static_cast<wchar_t*>(
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (text_len + 1) * sizeof(wchar_t)));
    GetWindowTextW(text_edit_, text_buffer, text_len + 1);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, text_buffer, text_len, NULL, 0, NULL, NULL);
    char* utf8_buffer = static_cast<char*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, utf8_len));
    WideCharToMultiByte(CP_UTF8, 0, text_buffer, text_len, utf8_buffer, utf8_len, NULL, NULL);

    DWORD bytes_written = 0;
    BOOL write_success = WriteFile(file_handle, utf8_buffer, utf8_len, &bytes_written, NULL);

    HeapFree(GetProcessHeap(), 0, utf8_buffer);
    HeapFree(GetProcessHeap(), 0, text_buffer);
    CloseHandle(file_handle);

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
    PRINTDLGW print_dialog = {sizeof(PRINTDLGW)};
    print_dialog.hwndOwner = hwnd;
    print_dialog.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (PrintDlgW(&print_dialog)) {
        DOCINFOW doc_info = {sizeof(DOCINFOW), L"Salt Text Document"};
        if (StartDocW(print_dialog.hDC, &doc_info) > 0) {
            StartPage(print_dialog.hDC);

            if (text_edit_) {
                int text_len = GetWindowTextLengthW(text_edit_);
                wchar_t* text_buffer = static_cast<wchar_t*>(HeapAlloc(
                    GetProcessHeap(), HEAP_ZERO_MEMORY, (text_len + 1) * sizeof(wchar_t)));
                GetWindowTextW(text_edit_, text_buffer, text_len + 1);

                RECT print_rect = {100, 100, 2000, 3000};
                DrawTextW(print_dialog.hDC, text_buffer, -1, &print_rect, DT_LEFT | DT_WORDBREAK);

                HeapFree(GetProcessHeap(), 0, text_buffer);
            }

            EndPage(print_dialog.hDC);
            EndDoc(print_dialog.hDC);
        }
        DeleteDC(print_dialog.hDC);
    }
}
