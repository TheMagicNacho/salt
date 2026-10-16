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

TEST(FileHandlerTest, OpenFileFailsOnNonexistentFile) {
    FileHandler file_handler;
    EXPECT_FALSE(file_handler.OpenFile(nullptr, L"C:\\nonexistent_salt_test_file_12345.txt"));
}

TEST(FileHandlerTest, OpenFileLoadsExistingFile) {
    wchar_t temp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_dir);
    std::wstring temp_file = std::wstring(temp_dir) + L"salt_test_sample.txt";
    HANDLE file_handle = CreateFileW(temp_file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_NE(file_handle, INVALID_HANDLE_VALUE);
    const char* content = "Hello Salt Test";
    DWORD bytes_written = 0;
    WriteFile(file_handle, content, static_cast<DWORD>(strlen(content)), &bytes_written, NULL);
    CloseHandle(file_handle);

    FileHandler file_handler;
    EXPECT_TRUE(file_handler.OpenFile(nullptr, temp_file));
    EXPECT_EQ(file_handler.GetFileName(), L"salt_test_sample.txt");
    EXPECT_FALSE(file_handler.IsDirty());

    DeleteFileW(temp_file.c_str());
}

TEST(FileHandlerTest, OpenFileLoadsDeveloperFilesTomlYamlAndRust) {
    wchar_t temp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_dir);

    struct FileTestCase {
        std::wstring filename;
        std::string content;
    } test_cases[] = {
        {L"salt_test_config.toml", "[package]\nname = \"salt\"\nversion = \"0.1.0\"\n"},
        {L"salt_test_workflow.yaml", "name: CI\non: [push]\njobs:\n  build:\n    runs-on: windows-latest\n"},
        {L"salt_test_main.rs", "fn main() {\n    println!(\"Hello Salt!\");\n}\n"},
    };

    for (const auto& test_case : test_cases) {
        std::wstring temp_path = std::wstring(temp_dir) + test_case.filename;
        HANDLE file_handle = CreateFileW(temp_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL, NULL);
        ASSERT_NE(file_handle, INVALID_HANDLE_VALUE);
        DWORD bytes_written = 0;
        WriteFile(file_handle, test_case.content.c_str(),
                  static_cast<DWORD>(test_case.content.size()), &bytes_written, NULL);
        CloseHandle(file_handle);

        FileHandler file_handler;
        EXPECT_TRUE(file_handler.OpenFile(nullptr, temp_path));
        EXPECT_EQ(file_handler.GetFileName(), test_case.filename);
        EXPECT_FALSE(file_handler.IsDirty());

        DeleteFileW(temp_path.c_str());
    }
}

TEST(FileHandlerTest, OpenFileCanOpenActivelyWrittenLogWithSharedWrite) {
    wchar_t temp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_dir);
    std::wstring temp_file = std::wstring(temp_dir) + L"salt_active_test.log";

    // Simulate an external logging service actively holding a write handle with FILE_SHARE_READ | FILE_SHARE_WRITE
    HANDLE writer_handle = CreateFileW(
        temp_file.c_str(), GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_NE(writer_handle, INVALID_HANDLE_VALUE);

    const char log_entry[] = "[INFO] Server started\n[INFO] Worker active\n";
    DWORD written = 0;
    WriteFile(writer_handle, log_entry, static_cast<DWORD>(strlen(log_entry)), &written, NULL);
    FlushFileBuffers(writer_handle);

    // Open file in Salt while writer_handle is still open!
    FileHandler file_handler;
    EXPECT_TRUE(file_handler.OpenFile(nullptr, temp_file));
    EXPECT_EQ(file_handler.GetFileName(), L"salt_active_test.log");

    CloseHandle(writer_handle);
    DeleteFileW(temp_file.c_str());
}

}  // namespace
