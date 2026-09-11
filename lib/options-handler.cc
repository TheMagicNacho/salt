#include "options-handler.h"
#include <commdlg.h>
#include <algorithm>

OptionsHandler::OptionsHandler(HWND main_hwnd, HWND edit_hwnd)
    : main_hwnd_(main_hwnd),
      edit_hwnd_(edit_hwnd),
      dark_bg_brush_(CreateSolidBrush(RGB(30, 30, 30))),
      light_bg_brush_(CreateSolidBrush(RGB(255, 255, 255))) {
    if (edit_hwnd_) {
        ApplyFont();
    }
}

OptionsHandler::~OptionsHandler() {
    if (current_font_) {
        DeleteObject(current_font_);
        current_font_ = nullptr;
    }
    if (dark_bg_brush_) {
        DeleteObject(dark_bg_brush_);
        dark_bg_brush_ = nullptr;
    }
    if (light_bg_brush_) {
        DeleteObject(light_bg_brush_);
        light_bg_brush_ = nullptr;
    }
}

void OptionsHandler::Init(HWND main_hwnd, HWND edit_hwnd) {
    main_hwnd_ = main_hwnd;
    edit_hwnd_ = edit_hwnd;
    ApplyFont();
}

void OptionsHandler::SetEditHandle(HWND edit_hwnd) {
    edit_hwnd_ = edit_hwnd;
    ApplyFont();
}

void OptionsHandler::ApplyFont() {
    if (!edit_hwnd_) return;

    HDC hdc = GetDC(edit_hwnd_);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(edit_hwnd_, hdc);

    int height = -MulDiv(font_size_pt_, dpi, 72);

    HFONT new_font = CreateFontW(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 FIXED_PITCH | FF_MODERN, font_name_.c_str());

    if (new_font) {
        SendMessageW(edit_hwnd_, WM_SETFONT, reinterpret_cast<WPARAM>(new_font), TRUE);
        if (current_font_) {
            DeleteObject(current_font_);
        }
        current_font_ = new_font;
    }
}

void OptionsHandler::ChooseFontDialog() {
    if (!main_hwnd_) return;

    LOGFONTW log_font{};
    HDC hdc = GetDC(main_hwnd_);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(main_hwnd_, hdc);

    log_font.lfHeight = -MulDiv(font_size_pt_, dpi, 72);
    log_font.lfWeight = FW_NORMAL;
    log_font.lfQuality = CLEARTYPE_QUALITY;
    wcsncpy_s(log_font.lfFaceName, font_name_.c_str(), _TRUNCATE);

    CHOOSEFONTW choose_font{};
    choose_font.lStructSize = sizeof(CHOOSEFONTW);
    choose_font.hwndOwner = main_hwnd_;
    choose_font.lpLogFont = &log_font;
    choose_font.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS;

    if (ChooseFontW(&choose_font)) {
        font_name_ = log_font.lfFaceName;
        font_size_pt_ = choose_font.iPointSize / 10;
        ApplyFont();
    }
}

void OptionsHandler::ZoomIn() {
    if (font_size_pt_ < 48) {
        font_size_pt_ = std::min(48, font_size_pt_ + 2);
        ApplyFont();
    }
}

void OptionsHandler::ZoomOut() {
    if (font_size_pt_ > 6) {
        font_size_pt_ = std::max(6, font_size_pt_ - 2);
        ApplyFont();
    }
}

void OptionsHandler::ZoomReset() {
    font_size_pt_ = 11;
    ApplyFont();
}

int OptionsHandler::GetFontSize() const { return font_size_pt_; }

bool OptionsHandler::ToggleWordWrap() {
    word_wrap_ = !word_wrap_;
    return word_wrap_;
}

bool OptionsHandler::IsWordWrap() const { return word_wrap_; }

void OptionsHandler::ToggleTheme() {
    theme_mode_ = (theme_mode_ == ThemeMode::Dark) ? ThemeMode::Light : ThemeMode::Dark;
    if (edit_hwnd_) {
        InvalidateRect(edit_hwnd_, NULL, TRUE);
        UpdateWindow(edit_hwnd_);
    }
    if (main_hwnd_) {
        InvalidateRect(main_hwnd_, NULL, TRUE);
        UpdateWindow(main_hwnd_);
    }
}

ThemeMode OptionsHandler::GetThemeMode() const { return theme_mode_; }

bool OptionsHandler::IsDarkMode() const { return theme_mode_ == ThemeMode::Dark; }

bool OptionsHandler::ToggleStatusBar() {
    show_status_bar_ = !show_status_bar_;
    return show_status_bar_;
}

bool OptionsHandler::IsStatusBarVisible() const { return show_status_bar_; }

HFONT OptionsHandler::GetCurrentFont() const { return current_font_; }

COLORREF OptionsHandler::GetBackgroundColor() const {
    return (theme_mode_ == ThemeMode::Dark) ? RGB(30, 30, 30) : RGB(255, 255, 255);
}

COLORREF OptionsHandler::GetTextColor() const {
    return (theme_mode_ == ThemeMode::Dark) ? RGB(212, 212, 212) : RGB(30, 30, 30);
}

HBRUSH OptionsHandler::GetBackgroundBrush() const {
    return (theme_mode_ == ThemeMode::Dark) ? dark_bg_brush_ : light_bg_brush_;
}
