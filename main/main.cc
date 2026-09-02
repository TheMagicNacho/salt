#include <windows.h>
#include <commctrl.h>
#include <objbase.h>

#include "lib/main-window.h"
#include "lib/errors.h"

#pragma comment( \
    linker,      \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    // Initilize the panic handler runtime.
    salt::PanicHandler::Install();

    try {
        // Check and set DPI awareness.
        // If user32 is not pressent, we have other issues.
        // It is ok if we do not successfully set DPI awareness. Just continue.
        HMODULE user_32_lib = GetModuleHandleW(L"user32.dll");
        if (user_32_lib) {
            typedef BOOL(WINAPI * fnSetDpi)(DPI_AWARENESS_CONTEXT);
            fnSetDpi set_dpi_callback = reinterpret_cast<fnSetDpi>(
                GetProcAddress(user_32_lib, "SetProcessDpiAwarenessContext"));
            if (set_dpi_callback) {
                set_dpi_callback(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
        }

        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        INITCOMMONCONTROLSEX icex{};
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        if (!MainWindow::RegisterClass(hInstance)) {
            SALT_PANIC("Window Registration Failed!");
        }

        MainWindow window;
        HWND hwnd = window.Create(hInstance, nCmdShow);
        if (!hwnd) {
            SALT_PANIC("Window Creation Failed!");
        }

        HACCEL accelerators = MainWindow::CreateAppAccelerators();
        if (!accelerators) {
            SALT_PANIC("Failed to create accelerator table!");
        }

        MSG msg{};
        while (GetMessageW(&msg, NULL, 0, 0) > 0) {
            if (!TranslateAcceleratorW(hwnd, accelerators, &msg)) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        // Clean up the accelerator table when the app closes.
        if (accelerators) {
            DestroyAcceleratorTable(accelerators);
        }

        CoUninitialize();
        return static_cast<int>(msg.wParam);
    } catch (const std::exception& e) {
        salt::PanicHandler::Panic(std::wstring(e.what(), e.what() + strlen(e.what())));
    } catch (...) {
        salt::PanicHandler::Panic(L"An unknown error occurred.");
    }
}
