#include <windows.h>
#include <commctrl.h>
#include <objbase.h>

#include "lib/main-window.h"

#pragma comment( \
    linker,      \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef BOOL(WINAPI * fnSetDpi)(DPI_AWARENESS_CONTEXT);
        fnSetDpi pfnSetDpi =
            reinterpret_cast<fnSetDpi>(GetProcAddress(hUser32, "SetProcessDpiAwarenessContext"));
        if (pfnSetDpi) {
            pfnSetDpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
    }

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    if (!MainWindow::RegisterClass(hInstance)) {
        MessageBoxW(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = MainWindow().Create(hInstance, nCmdShow);
    if (!hwnd) {
        MessageBoxW(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Windows calls them accelerators, but esentally they are keyboard shortcuts.
    HACCEL accelerators = MainWindow::CreateAppAccelerators();

    MSG msg{};
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (!TranslateAcceleratorW(hwnd, accelerators, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (accelerators) {
        DestroyAcceleratorTable(accelerators);
    }

    CoUninitialize();
    return static_cast<int>(msg.wParam);
}
