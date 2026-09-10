#include <gtest/gtest.h>
#include <string>

#include "lib/errors.h"

namespace {

TEST(PanicHandlerTest, SetEmergencySaveCallbackAcceptsCallbackAndResetsCleanly) {
    // Arrange: Create a callback producing emergency recovery content.
    bool callback_called = false;
    salt::EmergencySaveCallback callback = [&callback_called]() -> std::wstring {
        callback_called = true;
        return L"Emergency unsaved content";
    };

    // Act 1: Register the emergency callback.
    EXPECT_NO_THROW(salt::PanicHandler::SetEmergencySaveCallback(callback));

    // Act 2: Reset the emergency save callback to nullptr.
    EXPECT_NO_THROW(salt::PanicHandler::SetEmergencySaveCallback(nullptr));

    // Assert: Operations complete without exceptions or faults.
    EXPECT_FALSE(callback_called);
}

}  // namespace
