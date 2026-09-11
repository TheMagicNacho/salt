#include "errors.h"
#include <commctrl.h>
#include <fstream>
#include <sstream>

namespace salt {

/// @brief Registered callback to retrieve unsaved document text on abnormal termination.
static EmergencySaveCallback g_save_callback = nullptr;

/// @brief Writes unsaved document content to a temporary recovery file on disk.
/// @return Full path to the recovery file if successful, or empty string on failure.
static std::wstring PerformEmergencySave() {
    if (!g_save_callback) return L"";
    try {
        std::wstring content = g_save_callback();
        if (content.empty()) return L"";

        wchar_t temp_path[MAX_PATH];
        GetTempPathW(MAX_PATH, temp_path);
        std::wstring recovery_file = std::wstring(temp_path) + L"Salt_Crash_Recovery.txt";

        std::wofstream out(recovery_file);
        if (out) {
            out << content;
            return recovery_file;
        }
    } catch (...) {
        // Suppress any errors during emergency save to avoid recursive panics.
    }
    return L"";
}

/// @brief Displays a modal error dialog with crash details and optional recovery file path.
/// @param title Dialog window title.
/// @param details Detailed error message or exception information.
static void ShowCrashDialog(const std::wstring& title, const std::wstring& details) {
    std::wstring recovery_path = PerformEmergencySave();

    std::wstring message = details;
    if (!recovery_path.empty()) {
        message += L"\n\n[Recovery] Your unsaved text was saved to:\n" + recovery_path;
    }

    MessageBoxW(NULL, message.c_str(), title.c_str(),
                MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
}

/// @brief Structured Exception Handler (SEH) filter for fatal hardware/system exceptions.
/// @param exception_info Pointer to exception record and context pointers.
/// @return Exception execution handler action code.
static LONG WINAPI SehUnhandledExceptionFilter(EXCEPTION_POINTERS* exception_info) {
    wchar_t buffer[256];
    swprintf_s(buffer, L"A fatal system error occurred.\nException Code: 0x%08X\nAddress: 0x%p",
               exception_info->ExceptionRecord->ExceptionCode,
               exception_info->ExceptionRecord->ExceptionAddress);

    ShowCrashDialog(L"Salt Text Editor - Fatal Error", buffer);
    return EXCEPTION_EXECUTE_HANDLER;
}

/// @brief Global handler invoked by std::terminate on unhandled C++ exceptions.
static void TerminateHandler() {
    std::wstring message = L"An unhandled C++ exception occurred.";
    try {
        auto current_ex = std::current_exception();
        if (current_ex) std::rethrow_exception(current_ex);
    } catch (const std::exception& e) {
        std::string what_str = e.what();
        message = L"Unhandled Exception: " + std::wstring(what_str.begin(), what_str.end());
    } catch (...) {
        // Unknown exception type.
    }

    ShowCrashDialog(L"Salt Text Editor - Unhandled Exception", message);
    ExitProcess(1);
}

void PanicHandler::Install() {
    SetUnhandledExceptionFilter(SehUnhandledExceptionFilter);
    std::set_terminate(TerminateHandler);
}

void PanicHandler::SetEmergencySaveCallback(EmergencySaveCallback callback) {
    g_save_callback = std::move(callback);
}

[[noreturn]] void PanicHandler::Panic(const std::wstring& message, const char* file, int line) {
    std::wstringstream stream;
    stream << L"Application Panic!\n\n" << message;
    if (file && line > 0) {
        stream << L"\n\nLocation: " << file << L":" << line;
    }

    ShowCrashDialog(L"Salt Text Editor - PANIC", stream.str());
    ExitProcess(1);
}

}  // namespace salt

