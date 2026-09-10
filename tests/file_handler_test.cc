#include <gtest/gtest.h>
#include "lib/file-handler.h"

namespace {

TEST(FileHandlerTest, InitialStateIsNotDirtyAndHasEmptyPath) {
    // Arrange: Create a default FileHandler without an attached edit control.
    FileHandler file_handler;

    // Act: Query initial dirty status, file path, and display name.
    bool dirty = file_handler.IsDirty();
    std::wstring path = file_handler.GetFilePath();
    std::wstring display_name = file_handler.GetFileName();

    // Assert: Document starts clean with no file path and "Untitled" display name.
    EXPECT_FALSE(dirty);
    EXPECT_TRUE(path.empty());
    EXPECT_EQ(display_name, L"Untitled");
}

TEST(FileHandlerTest, SetDirtyUpdatesModifiedState) {
    // Arrange: Create a clean FileHandler instance.
    FileHandler file_handler;
    ASSERT_FALSE(file_handler.IsDirty());

    // Act 1: Mark the document as dirty.
    file_handler.SetDirty(true);
    bool state_after_dirty = file_handler.IsDirty();

    // Act 2: Mark the document as clean.
    file_handler.SetDirty(false);
    bool state_after_clean = file_handler.IsDirty();

    // Assert: IsDirty reflects the updated boolean state accurately.
    EXPECT_TRUE(state_after_dirty);
    EXPECT_FALSE(state_after_clean);
}

TEST(FileHandlerTest, GetFileNameReturnsUntitledWhenFilePathIsEmpty) {
    // Arrange: Ensure the file path is explicitly empty.
    FileHandler file_handler;
    file_handler.SetFilePath(L"");

    // Act: Query file display name.
    std::wstring name = file_handler.GetFileName();

    // Assert: Display name must be "Untitled".
    EXPECT_EQ(name, L"Untitled");
}

TEST(FileHandlerTest, GetFileNameExtractsFileNameFromWindowsPath) {
    // Arrange: Set a Windows-style path with backslashes.
    FileHandler file_handler;
    file_handler.SetFilePath(L"C:\\Users\\salt\\Documents\\notes.txt");

    // Act: Extract display name.
    std::wstring name = file_handler.GetFileName();

    // Assert: Only the trailing file name component is returned.
    EXPECT_EQ(name, L"notes.txt");
}

TEST(FileHandlerTest, GetFileNameExtractsFileNameFromUnixPath) {
    // Arrange: Set a Unix-style path with forward slashes.
    FileHandler file_handler;
    file_handler.SetFilePath(L"projects/salt/README.md");

    // Act: Extract display name.
    std::wstring name = file_handler.GetFileName();

    // Assert: Only the trailing file name component is returned.
    EXPECT_EQ(name, L"README.md");
}

TEST(FileHandlerTest, GetFileNameReturnsFullNameWhenNoSlashesPresent) {
    // Arrange: Set a simple filename without any directory prefixes.
    FileHandler file_handler;
    file_handler.SetFilePath(L"standalone_file.json");

    // Act: Extract display name.
    std::wstring name = file_handler.GetFileName();

    // Assert: The filename itself is preserved.
    EXPECT_EQ(name, L"standalone_file.json");
}

TEST(FileHandlerTest, PromptSaveIfDirtyReturnsTrueImmediatelyWhenClean) {
    // Arrange: FileHandler in a non-dirty state.
    FileHandler file_handler;
    file_handler.SetDirty(false);

    // Act: Prompt to save changes before closing/opening (with null parent hwnd).
    bool proceed = file_handler.PromptSaveIfDirty(nullptr);

    // Assert: Returns true immediately without prompting because there are no unsaved changes.
    EXPECT_TRUE(proceed);
}

TEST(FileHandlerTest, NewResetsFilePathAndDirtyStateWhenClean) {
    // Arrange: Document with an existing path and clean state.
    FileHandler file_handler;
    file_handler.SetFilePath(L"C:\\workspace\\doc.txt");
    file_handler.SetDirty(false);

    // Act: Create a new document.
    bool success = file_handler.New(nullptr);

    // Assert: Path is cleared, document is clean, and display name resets to Untitled.
    EXPECT_TRUE(success);
    EXPECT_TRUE(file_handler.GetFilePath().empty());
    EXPECT_FALSE(file_handler.IsDirty());
    EXPECT_EQ(file_handler.GetFileName(), L"Untitled");
}

}  // namespace
