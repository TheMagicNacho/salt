#ifndef SALT_LIB_OPTIONS_HANDLER_H_
#define SALT_LIB_OPTIONS_HANDLER_H_

#include <windows.h>
#include <string>

/// @file options-handler.h
/// @brief UI and editor options management (fonts, zoom, themes, status bar, word wrap).

/// @enum ThemeMode
/// @brief Visual theme mode for the application.
enum class ThemeMode {
    Dark,   ///< Dark theme mode.
    Light   ///< Light theme mode.
};

/// @class OptionsHandler
/// @brief Manages editor configuration options and visual styling.
///
/// Handles font selection and sizing, zoom levels, word wrap toggle, status bar visibility,
/// and light/dark theme colors and background brushes.
class OptionsHandler {
    HWND edit_hwnd_{nullptr};
    HWND main_hwnd_{nullptr};
    HFONT current_font_{nullptr};
    std::wstring font_name_{L"Cascadia Code"};
    int font_size_pt_{11};
    bool word_wrap_{true};
    bool show_status_bar_{false};
    ThemeMode theme_mode_{ThemeMode::Light};
    HBRUSH dark_bg_brush_{nullptr};
    HBRUSH light_bg_brush_{nullptr};

    /// @brief Recreates and applies the current font to the edit control.
    void ApplyFont();

   public:
    /// @brief Constructs an OptionsHandler instance.
    /// @param main_hwnd Handle to the main application window.
    /// @param edit_hwnd Handle to the text edit control.
    OptionsHandler(HWND main_hwnd = nullptr, HWND edit_hwnd = nullptr);

    /// @brief Destructor that cleans up GDI font and brush objects.
    ~OptionsHandler();

    /// @brief Initializes the options handler with window handles and applies settings.
    /// @param main_hwnd Handle to the main window.
    /// @param edit_hwnd Handle to the edit control.
    void Init(HWND main_hwnd, HWND edit_hwnd);

    /// @brief Updates the edit control handle and applies font.
    /// @param edit_hwnd Handle to the edit control.
    void SetEditHandle(HWND edit_hwnd);

    /// @brief Displays the Win32 Choose Font modal dialog and updates editor font.
    void ChooseFontDialog();

    /// @brief Increases the editor font size by 2 points (up to 48pt maximum).
    void ZoomIn();

    /// @brief Decreases the editor font size by 2 points (down to 6pt minimum).
    void ZoomOut();

    /// @brief Resets the editor font size to the default 11pt.
    void ZoomReset();

    /// @brief Toggles word wrapping on or off.
    /// @return The new word wrap state (true = enabled).
    bool ToggleWordWrap();

    /// @brief Checks if word wrapping is currently enabled.
    /// @return true if word wrap is enabled; false otherwise.
    bool IsWordWrap() const;

    /// @brief Toggles between Light and Dark visual themes.
    void ToggleTheme();

    /// @brief Gets the current theme mode.
    /// @return The active ThemeMode (Dark or Light).
    ThemeMode GetThemeMode() const;

    /// @brief Checks if dark mode is currently active.
    /// @return true if theme mode is ThemeMode::Dark; false otherwise.
    bool IsDarkMode() const;

    /// @brief Toggles visibility of the status bar.
    /// @return The new status bar visibility state (true = visible).
    bool ToggleStatusBar();

    /// @brief Checks if the status bar is currently visible.
    /// @return true if visible; false otherwise.
    bool IsStatusBarVisible() const;

    /// @brief Gets the currently active HFONT handle.
    /// @return HFONT handle to the current font.
    HFONT GetCurrentFont() const;

    /// @brief Gets the background COLORREF matching the active theme mode.
    /// @return COLORREF value for background.
    COLORREF GetBackgroundColor() const;

    /// @brief Gets the text COLORREF matching the active theme mode.
    /// @return COLORREF value for text.
    COLORREF GetTextColor() const;

    /// @brief Gets the GDI HBRUSH for painting the edit control background.
    /// @return HBRUSH handle matching the current theme.
    HBRUSH GetBackgroundBrush() const;
};

#endif  // SALT_LIB_OPTIONS_HANDLER_H_
