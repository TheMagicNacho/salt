#include <gtest/gtest.h>
#include <windows.h>
#include <algorithm>
#include <vector>

#include "lib/context.h"
#include "lib/main-window.h"

namespace {

TEST(MainWindowTest, CreateAppAcceleratorsReturnsValidTableWithExpectedEntries) {
    // Arrange: Call the static factory method to generate application keyboard accelerators.
    HACCEL accel_table = MainWindow::CreateAppAccelerators();
    ASSERT_NE(accel_table, nullptr);

    // Act: Query the accelerator count and copy table entries.
    int count = CopyAcceleratorTableW(accel_table, nullptr, 0);
    std::vector<ACCEL> accels(count);
    CopyAcceleratorTableW(accel_table, accels.data(), count);

    // Assert 1: Exactly 14 accelerators must be registered.
    EXPECT_EQ(count, 14);

    // Assert 2: Verify specific critical command shortcuts are present.
    auto has_cmd = [&](WORD cmd) {
        return std::any_of(accels.begin(), accels.end(),
                           [cmd](const ACCEL& a) { return a.cmd == cmd; });
    };

    EXPECT_TRUE(has_cmd(Context::Command::FileNew));
    EXPECT_TRUE(has_cmd(Context::Command::FileOpen));
    EXPECT_TRUE(has_cmd(Context::Command::FileSave));
    EXPECT_TRUE(has_cmd(Context::Command::FileSaveAs));
    EXPECT_TRUE(has_cmd(Context::Command::FilePrint));
    EXPECT_TRUE(has_cmd(Context::Command::ViewWordWrap));
    EXPECT_TRUE(has_cmd(Context::Command::EditUndo));
    EXPECT_TRUE(has_cmd(Context::Command::EditSelectAll));
    EXPECT_TRUE(has_cmd(Context::Command::ViewZoomIn));
    EXPECT_TRUE(has_cmd(Context::Command::ViewZoomOut));
    EXPECT_TRUE(has_cmd(Context::Command::ViewZoomReset));

    // Cleanup GDI accelerator resource
    DestroyAcceleratorTable(accel_table);
}

}  // namespace
