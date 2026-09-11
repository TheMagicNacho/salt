#ifndef SALT_LIB_MAIN_WINDOW_H_
#define SALT_LIB_MAIN_WINDOW_H_

#include <windows.h>
#include <commctrl.h>

#include "lib/file-handler.h"
#include "lib/options-handler.h"
#include "lib/menu-bar.h"
#include "lib/errors.h"
#include "lib/context.h"

/// @file main-window.h
/// @brief Main application window and message dispatcher for the Salt text editor.

/// @class MainWindow
/// @brief Represents the top-level main application window.
///
/// Encapsulates the Win32 window creation, event dispatching loop, menu management,
/// child controls (multiline edit and status bar), keyboard accelerators, and dark mode
/// integration.
class MainWindow {
   public:
    /// @brief The Win32 window class name registered for the main window.
    static constexpr const wchar_t* kClassName = L"SaltTextEditorWindowClass";

    /// @brief Default constructor.
    MainWindow() = default;

    /// @brief Default destructor.
    ~MainWindow() = default;

    /// @brief Registers the Win32 window class with the operating system.
    /// @param instance Application instance handle.
    /// @return true if window class registration succeeded; false otherwise.
    static bool RegisterClass(HINSTANCE instance);

    /// @brief Creates and loads the Win32 keyboard accelerator table.
    /// @return HACCEL handle to the accelerator table.
    static HACCEL CreateAppAccelerators();

    /// @brief Creates the main application window and displays it.
    /// @param instance Application instance handle.
    /// @param show_state Window show state (e.g. SW_SHOW, SW_MAXIMIZE).
    /// @return HWND handle to the created window, or nullptr on failure.
    HWND Create(HINSTANCE instance, int show_state);

    /// @brief Static Win32 window procedure that routes messages to the target MainWindow instance.
    /// @param hwnd Window handle.
    /// @param msg Message identifier.
    /// @param param_w Additional message parameter.
    /// @param param_l Additional message parameter.
    /// @return Result of the message processing.
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM param_w, LPARAM param_l);

   private:
    /// @brief Enables or disables immersive dark mode on the window title bar and menus.
    /// @param enable true for dark mode; false for light mode.
    void EnableDarkMode(bool enable);

    /// @brief Updates the window title bar text with the current file name and modified indicator.
    void UpdateTitle();

    /// @brief Updates the status bar parts with cursor position, character count, encoding, and
    /// save state.
    void UpdateStatusBar();

    /// @brief Builds and attaches the top-level application menu bar to the window.
    // void CreateAppMenu();

    /// @brief Recreates the child edit control (required when toggling word wrap styles in Win32).
    void RecreateEditControl();

    /// @brief Sets the menu bar for the window.
    void SetMenuBar();

    /// @brief Member window message handler for all Win32 messages dispatched to this window.
    /// @param msg Message identifier.
    /// @param param_w Additional message parameter.
    /// @param param_l Additional message parameter.
    /// @return Result of the message processing.
    LRESULT HandleMessage(UINT msg, WPARAM param_w, LPARAM param_l);

    HWND hwnd_{nullptr};
    HWND edit_hwnd_{nullptr};
    HWND status_hwnd_{nullptr};
    FileHandler file_handler_;
    OptionsHandler options_handler_;
    MenuBar menu_bar_;
};

#endif  // SALT_LIB_MAIN_WINDOW_H_
