#ifndef SALT_LIB_OPTIONS_HANDLER_H_
#define SALT_LIB_OPTIONS_HANDLER_H_
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <string>

enum class ThemeMode { Dark, Light };

class OptionsHandler {
    HWND edit_hwnd_{nullptr};
    HWND main_hwnd_{nullptr};
    HFONT current_font_{nullptr};
    std::wstring font_name_{L"Cascadia Code"};
    int font_size_pt_{11};
    bool word_wrap_{true};
    bool show_status_bar_{true};
    ThemeMode theme_mode_{ThemeMode::Light};
    HBRUSH dark_bg_brush_{nullptr};
    HBRUSH light_bg_brush_{nullptr};

    void ApplyFont();

   public:
    OptionsHandler(HWND main_hwnd = nullptr, HWND edit_hwnd = nullptr);
    ~OptionsHandler();

    void Init(HWND main_hwnd, HWND edit_hwnd);
    void SetEditHandle(HWND edit_hwnd);

    void ChooseFontDialog();
    void ZoomIn();
    void ZoomOut();
    void ZoomReset();

    bool ToggleWordWrap();
    bool IsWordWrap() const;

    void ToggleTheme();
    ThemeMode GetThemeMode() const;
    bool IsDarkMode() const;

    bool ToggleStatusBar();
    bool IsStatusBarVisible() const;

    HFONT GetCurrentFont() const;
    COLORREF GetBackgroundColor() const;
    COLORREF GetTextColor() const;
    HBRUSH GetBackgroundBrush() const;
};

#endif
