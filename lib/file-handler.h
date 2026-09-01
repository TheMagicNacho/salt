#ifndef SALT_LIB_FILE_HANDLER_H_
#define SALT_LIB_FILE_HANDLER_H_

#include <windows.h>
#include <commdlg.h>
#include <string>

/// @file file-handler.h
/// @brief File operations facade for the Salt text editor.

/// @class FileHandler
/// @brief Handles file operations for the application.
///
/// Implements a facade pattern that provides a simplified, high-level interface
/// for common file operations including creating new files, opening, saving,
/// save-as dialogs, printing, and tracking the modified (dirty) state.
class FileHandler {
    HWND text_edit_{nullptr};
    std::wstring current_file_path_;
    bool is_dirty_{false};

   public:
    /// @brief Constructs a new FileHandler instance.
    /// @param edit_hwnd Optional handle to the Win32 edit control.
    FileHandler(HWND edit_hwnd = nullptr);

    /// @brief Prompts to save if needed, then resets the editor for a new document.
    /// @param hwnd Handle to the parent window (used as modal parent for dialogs).
    /// @return true if the new document was created; false if cancelled.
    bool New(HWND hwnd);

    /// @brief Displays an Open File dialog and loads the selected file into the editor.
    /// @param hwnd Handle to the parent window.
    /// @return true if a file was successfully opened; false otherwise.
    bool Open(HWND hwnd);

    /// @brief Saves the current document. If untitled, prompts with Save As dialog.
    /// @param hwnd Handle to the parent window.
    /// @return true if saved successfully; false otherwise.
    bool Save(HWND hwnd);

    /// @brief Prompts the user with a Save As dialog and saves the document to the chosen path.
    /// @param hwnd Handle to the parent window.
    /// @return true if saved successfully; false otherwise.
    bool SaveAs(HWND hwnd);

    /// @brief Initiates a print operation for the document text.
    /// @param hwnd Handle to the parent window.
    void Print(HWND hwnd);

    /// @brief If document is dirty, prompts the user to save changes before closing/opening.
    /// @param hwnd Handle to the parent window.
    /// @return true if safe to proceed (saved or discarded); false if user cancelled.
    bool PromptSaveIfDirty(HWND hwnd);

    /// @brief Sets or updates the handle to the edit control.
    /// @param edit_hwnd Handle to the Win32 edit control.
    void SetEditHandle(HWND edit_hwnd);

    /// @brief Sets the dirty (modified) state of the document.
    /// @param dirty true if the document has unsaved modifications.
    void SetDirty(bool dirty);

    /// @brief Checks whether the document has unsaved modifications.
    /// @return true if dirty/modified; false otherwise.
    bool IsDirty() const;

    /// @brief Returns the full file path of the currently open document.
    /// @return Full file path as a wide string, or empty if untitled.
    std::wstring GetFilePath() const;

    /// @brief Returns the display file name (or "Untitled" if not saved yet).
    /// @return File name as a wide string.
    std::wstring GetFileName() const;
};

#endif  // SALT_LIB_FILE_HANDLER_H_
