#ifndef SALT_LIB_ERRORS_H_
#define SALT_LIB_ERRORS_H_

#include <windows.h>
#include <functional>
#include <string>

/// @file errors.h
/// @brief Crash handling, assertions, and emergency recovery callback mechanisms.

namespace salt {

/// @brief Callback type for retrieving unsaved editor text during an emergency crash save.
using EmergencySaveCallback = std::function<std::wstring()>;

/// @class PanicHandler
/// @brief Manages unhandled exception filters, panic dialogs, and crash recovery.
class PanicHandler {
   public:
    /// @brief Installs global SEH and C++ terminate exception handlers.
    static void Install();

    /// @brief Registers a callback to retrieve unsaved document text on application crash.
    /// @param callback Function returning the wide string content of the current buffer.
    static void SetEmergencySaveCallback(EmergencySaveCallback callback);

    /// @brief Displays a crash dialog with diagnostic info, triggers emergency save, and terminates.
    /// @param message Descriptive error message.
    /// @param file Source file name where panic originated (optional).
    /// @param line Source line number where panic originated (optional).
    [[noreturn]] static void Panic(const std::wstring& message, const char* file = nullptr,
                                   int line = 0);
};

}  // namespace salt

/// @def SALT_PANIC
/// @brief Convenience macro to trigger an application panic with file and line location info.
#define SALT_PANIC(msg) salt::PanicHandler::Panic(L##msg, __FILE__, __LINE__)

/// @def SALT_ASSERT
/// @brief Evaluates an expression and triggers a panic if the condition evaluates to false.
#define SALT_ASSERT(expr, msg)                                                             \
    do {                                                                                   \
        if (!(expr)) {                                                                     \
            salt::PanicHandler::Panic(L"Assertion Failed: " L#expr L"\n" L##msg, __FILE__, \
                                      __LINE__);                                           \
        }                                                                                  \
    } while (0)

#endif  // SALT_LIB_ERRORS_H_

