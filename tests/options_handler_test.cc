#include <gtest/gtest.h>
#include "lib/options-handler.h"

namespace {

TEST(OptionsHandlerTest, InitialStateHasDefaultSettings) {
    // Arrange: Create a default OptionsHandler instance with no window handles attached.
    OptionsHandler options;

    // Act: Query initial configuration state.
    int font_size = options.GetFontSize();
    bool is_word_wrap = options.IsWordWrap();
    bool is_status_bar = options.IsStatusBarVisible();
    bool is_dark_mode = options.IsDarkMode();
    ThemeMode theme_mode = options.GetThemeMode();
    COLORREF bg_color = options.GetBackgroundColor();
    COLORREF text_color = options.GetTextColor();

    // Assert: Verify all settings conform to initial default specifications.
    EXPECT_EQ(font_size, 11);
    EXPECT_TRUE(is_word_wrap);
    EXPECT_FALSE(is_status_bar);
    EXPECT_FALSE(is_dark_mode);
    EXPECT_EQ(theme_mode, ThemeMode::Light);
    EXPECT_EQ(bg_color, RGB(255, 255, 255));
    EXPECT_EQ(text_color, RGB(30, 30, 30));
}

TEST(OptionsHandlerTest, ZoomInIncreasesFontSizeByTwoPoints) {
    // Arrange: Instantiate OptionsHandler and record starting font size.
    OptionsHandler options;
    int initial_size = options.GetFontSize();

    // Act: Execute ZoomIn.
    options.ZoomIn();
    int new_size = options.GetFontSize();

    // Assert: Font size must have incremented by 2 points.
    EXPECT_EQ(new_size, initial_size + 2);
}

TEST(OptionsHandlerTest, ZoomInCapsAtMaximumOfFortyEightPoints) {
    // Arrange: Instantiate OptionsHandler at default 11pt.
    OptionsHandler options;

    // Act: Repeatedly trigger ZoomIn beyond the 48pt upper limit.
    for (int i = 0; i < 30; ++i) {
        options.ZoomIn();
    }
    int capped_size = options.GetFontSize();

    // Assert: Font size must not exceed the 48pt maximum ceiling.
    EXPECT_EQ(capped_size, 48);
}

TEST(OptionsHandlerTest, ZoomOutDecreasesFontSizeByTwoPoints) {
    // Arrange: Instantiate OptionsHandler and record starting font size.
    OptionsHandler options;
    int initial_size = options.GetFontSize();

    // Act: Execute ZoomOut.
    options.ZoomOut();
    int new_size = options.GetFontSize();

    // Assert: Font size must have decremented by 2 points.
    EXPECT_EQ(new_size, initial_size - 2);
}

TEST(OptionsHandlerTest, ZoomOutFloorsAtMinimumOfSixPoints) {
    // Arrange: Instantiate OptionsHandler at default 11pt.
    OptionsHandler options;

    // Act: Repeatedly trigger ZoomOut beyond the 6pt lower limit.
    for (int i = 0; i < 15; ++i) {
        options.ZoomOut();
    }
    int floored_size = options.GetFontSize();

    // Assert: Font size must not drop below the 6pt minimum floor.
    EXPECT_EQ(floored_size, 6);
}

TEST(OptionsHandlerTest, ZoomResetRestoresDefaultFontSize) {
    // Arrange: Instantiate OptionsHandler and alter font size via zooming.
    OptionsHandler options;
    options.ZoomIn();
    options.ZoomIn();
    options.ZoomIn();
    ASSERT_NE(options.GetFontSize(), 11);

    // Act: Reset zoom back to default.
    options.ZoomReset();
    int reset_size = options.GetFontSize();

    // Assert: Font size must be restored exactly to default 11pt.
    EXPECT_EQ(reset_size, 11);
}

TEST(OptionsHandlerTest, ToggleWordWrapTogglesState) {
    // Arrange: Instantiate OptionsHandler (word wrap defaults to true).
    OptionsHandler options;
    ASSERT_TRUE(options.IsWordWrap());

    // Act: Toggle word wrap once, then a second time.
    bool first_toggle = options.ToggleWordWrap();
    bool state_after_first = options.IsWordWrap();
    bool second_toggle = options.ToggleWordWrap();
    bool state_after_second = options.IsWordWrap();

    // Assert: First toggle turns it false; second toggle returns it to true.
    EXPECT_FALSE(first_toggle);
    EXPECT_FALSE(state_after_first);
    EXPECT_TRUE(second_toggle);
    EXPECT_TRUE(state_after_second);
}

TEST(OptionsHandlerTest, ToggleStatusBarTogglesVisibility) {
    // Arrange: Instantiate OptionsHandler (status bar defaults to false / hidden).
    OptionsHandler options;
    ASSERT_FALSE(options.IsStatusBarVisible());

    // Act: Toggle status bar visibility twice.
    bool first_toggle = options.ToggleStatusBar();
    bool visible_after_first = options.IsStatusBarVisible();
    bool second_toggle = options.ToggleStatusBar();
    bool visible_after_second = options.IsStatusBarVisible();

    // Assert: First toggle reveals status bar (true); second toggle hides it (false).
    EXPECT_TRUE(first_toggle);
    EXPECT_TRUE(visible_after_first);
    EXPECT_FALSE(second_toggle);
    EXPECT_FALSE(visible_after_second);
}

TEST(OptionsHandlerTest, ToggleThemeSwitchesBetweenLightAndDarkModes) {
    // Arrange: Instantiate OptionsHandler in default Light mode.
    OptionsHandler options;
    ASSERT_FALSE(options.IsDarkMode());
    ASSERT_EQ(options.GetThemeMode(), ThemeMode::Light);

    // Act & Assert 1: Toggle into Dark Mode and verify colors and state.
    options.ToggleTheme();
    EXPECT_TRUE(options.IsDarkMode());
    EXPECT_EQ(options.GetThemeMode(), ThemeMode::Dark);
    EXPECT_EQ(options.GetBackgroundColor(), RGB(30, 30, 30));
    EXPECT_EQ(options.GetTextColor(), RGB(212, 212, 212));

    // Act & Assert 2: Toggle back to Light Mode and verify restored colors and state.
    options.ToggleTheme();
    EXPECT_FALSE(options.IsDarkMode());
    EXPECT_EQ(options.GetThemeMode(), ThemeMode::Light);
    EXPECT_EQ(options.GetBackgroundColor(), RGB(255, 255, 255));
    EXPECT_EQ(options.GetTextColor(), RGB(30, 30, 30));
}

TEST(OptionsHandlerTest, GetBackgroundBrushReturnsValidGdiBrushes) {
    // Arrange: Instantiate OptionsHandler.
    OptionsHandler options;

    // Act: Retrieve background brushes in both light and dark modes.
    HBRUSH light_brush = options.GetBackgroundBrush();
    options.ToggleTheme();
    HBRUSH dark_brush = options.GetBackgroundBrush();

    // Assert: Both brush handles must be valid non-null GDI objects and distinct from each other.
    EXPECT_NE(light_brush, nullptr);
    EXPECT_NE(dark_brush, nullptr);
    EXPECT_NE(light_brush, dark_brush);
}

}  // namespace
