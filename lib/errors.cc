#include "errors.h"
#include <commctrl.h>
#include <fstream>
#include <sstream>

namespace salt {

static EmergencySaveCallback g_save_callback = nullptr;

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
    }
    return L"";
}

static void ShowCrashDialog(const std::wstring& title, const std::wstring& details) {
    std::wstring recovery_path = PerformEmergencySave();

    std::wstring message = details;
    if (!recovery_path.empty()) {
        message += L"\n\n[Recovery] Your unsaved text was saved to:\n" + recovery_path;
    }

    MessageBoxW(NULL, message.c_str(), title.c_str(),
                MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
}

static LONG WINAPI SehUnhandledExceptionFilter(EXCEPTION_POINTERS* ep) {
    wchar_t buf[256];
    swprintf_s(buf, L"A fatal system error occurred.\nException Code: 0x%08X\nAddress: 0x%p",
               ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);

    ShowCrashDialog(L"Salt Text Editor - Fatal Error", buf);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void TerminateHandler() {
    std::wstring msg = L"An unhandled C++ exception occurred.";
    try {
        auto ex = std::current_exception();
        if (ex) std::rethrow_exception(ex);
    } catch (const std::exception& e) {
        std::string what = e.what();
        msg = L"Unhandled Exception: " + std::wstring(what.begin(), what.end());
    } catch (...) {
    }

    ShowCrashDialog(L"Salt Text Editor - Unhandled Exception", msg);
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
    std::wstringstream ss;
    ss << L"Application Panic!\n\n" << message;
    if (file && line > 0) {
        ss << L"\n\nLocation: " << file << L":" << line;
    }

    ShowCrashDialog(L"Salt Text Editor - PANIC", ss.str());
    ExitProcess(1);
}

}  // namespace salt
