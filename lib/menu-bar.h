#ifndef SALT_LIB_MENU_BAR_H_
#define SALT_LIB_MENU_BAR_H_

#include <windows.h>
#include <commctrl.h>

#include "lib/file-handler.h"
#include "lib/options-handler.h"
#include "lib/context.h"

class MenuBar {
   private:
    HMENU menu_bar_{nullptr};

   public:
    MenuBar() = default;
    ~MenuBar();

    void Create(OptionsHandler& options_handler);

    HMENU GetMenuHandle() const;
};

#endif  // SALT_LIB_MENU_BAR_H_
