#ifndef SALT_LIB_MENU_BAR_H_
#define SALT_LIB_MENU_BAR_H_

#include <windows.h>
#include <commctrl.h>

#include "lib/context.h"
#include "lib/file-handler.h"
#include "lib/options-handler.h"

/// @file menu-bar.h
/// @brief Menu bar builder and handle container for the Salt text editor.

/// @class MenuBar
/// @brief Encapsulates the application's top-level menu hierarchy and creation.
class MenuBar {
   private:
    HMENU menu_bar_{nullptr};

   public:
    /// @brief Default constructor.
    MenuBar() = default;

    /// @brief Destructor that cleans up the managed HMENU if present.
    ~MenuBar();

    /// @brief Builds or rebuilds the top-level menu hierarchy reflecting current options state.
    /// @param options_handler Reference to options handler for querying checkmark states.
    void Create(OptionsHandler& options_handler);

    /// @brief Returns the Win32 handle to the built menu bar.
    /// @return HMENU handle, or nullptr if not created.
    HMENU GetMenuHandle() const;
};

#endif  // SALT_LIB_MENU_BAR_H_

