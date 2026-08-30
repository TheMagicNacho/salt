#ifndef SALT_LIB_FILE_HANDLER_H_
#define SALT_LIB_FILE_HANDLER_H_
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
#include <string>

/// Handles file operations for the application.
/// A facade pattern that provides a simplified interface for file operations.
class FileHandler {
    HWND text_edit_;
    std::wstring current_file_path_;

   public:
    FileHandler(HWND edit_hwnd);

    void Open(HWND window_handler);
    void Save(HWND window_handler);
    void SaveAs(HWND window_handler);
    void Print(HWND window_handler);

    void SetEditHandle(HWND window_handler);
};

#endif
