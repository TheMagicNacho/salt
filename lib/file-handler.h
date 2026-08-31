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
    HWND text_edit_{nullptr};
    std::wstring current_file_path_;
    bool is_dirty_{false};

   public:
    FileHandler(HWND edit_hwnd = nullptr);

    bool New(HWND hwnd);
    bool Open(HWND hwnd);
    bool Save(HWND hwnd);
    bool SaveAs(HWND hwnd);
    void Print(HWND hwnd);

    bool PromptSaveIfDirty(HWND hwnd);
    void SetEditHandle(HWND edit_hwnd);
    void SetDirty(bool dirty);
    bool IsDirty() const;

    std::wstring GetFilePath() const;
    std::wstring GetFileName() const;
};

#endif
