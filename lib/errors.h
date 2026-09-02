// lib/panic-handler.h
#ifndef SALT_LIB_PANIC_HANDLER_H_
#define SALT_LIB_PANIC_HANDLER_H_

#include <windows.h>
#include <functional>
#include <string>

namespace salt {

using EmergencySaveCallback = std::function<std::wstring()>;

class PanicHandler {
   public:
    static void Install();
    static void SetEmergencySaveCallback(EmergencySaveCallback callback);
    [[noreturn]] static void Panic(const std::wstring& message, const char* file = nullptr,
                                   int line = 0);
};

}  // namespace salt

#define SALT_PANIC(msg) salt::PanicHandler::Panic(L##msg, __FILE__, __LINE__)

#define SALT_ASSERT(expr, msg)                                                             \
    do {                                                                                   \
        if (!(expr)) {                                                                     \
            salt::PanicHandler::Panic(L"Assertion Failed: " L#expr L"\n" L##msg, __FILE__, \
                                      __LINE__);                                           \
        }                                                                                  \
    } while (0)

#endif
