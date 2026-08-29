#pragma once
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <commdlg.h>
/// Handles file operations for the application.
/// A facade pattern that provides a simplified interface for file operations.
class FileHandler {
   public:
    FileHandler(HWND edit_hwnd);

    void Open(HWND hwnd);
    void Save(HWND hwnd);
    void Print(HWND hwnd);

    void SetEditHandle(HWND hwnd);

   private:
    HWND text_edit_;
};
