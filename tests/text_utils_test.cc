#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "lib/text-utils.h"

namespace {

TEST(TextUtilsTest, NormalizeLineEndingsConvertsLoneLineFeedToCrLf) {
    // Arrange: Text with Unix-style '\n' line endings.
    std::wstring input = L"line 1\nline 2\nline 3";

    // Act: Normalize line endings.
    std::wstring result = salt::NormalizeLineEndings(input);

    // Assert: All lone '\n' line endings are converted to Windows '\r\n'.
    EXPECT_EQ(result, L"line 1\r\nline 2\r\nline 3");
}

TEST(TextUtilsTest, NormalizeLineEndingsConvertsLoneCarriageReturnToCrLf) {
    // Arrange: Text with classic Mac-style '\r' line endings.
    std::wstring input = L"line 1\rline 2\rline 3";

    // Act: Normalize line endings.
    std::wstring result = salt::NormalizeLineEndings(input);

    // Assert: All lone '\r' line endings are converted to Windows '\r\n'.
    EXPECT_EQ(result, L"line 1\r\nline 2\r\nline 3");
}

TEST(TextUtilsTest, NormalizeLineEndingsPreservesExistingCrLfWithoutDuplication) {
    // Arrange: Text already formatted with valid Windows '\r\n' line endings.
    std::wstring input = L"line 1\r\nline 2\r\nline 3\r\n";

    // Act: Normalize line endings.
    std::wstring result = salt::NormalizeLineEndings(input);

    // Assert: Standard '\r\n' sequences remain unchanged and are not doubled to '\r\r\n'.
    EXPECT_EQ(result, L"line 1\r\nline 2\r\nline 3\r\n");
}

TEST(TextUtilsTest, NormalizeLineEndingsHandlesMixedLineEndings) {
    // Arrange: Text containing a mix of '\r\n', '\n', and '\r'.
    std::wstring input = L"header\r\nitem 1\nitem 2\rend";

    // Act: Normalize line endings.
    std::wstring result = salt::NormalizeLineEndings(input);

    // Assert: All line endings are uniformly standardized to '\r\n'.
    EXPECT_EQ(result, L"header\r\nitem 1\r\nitem 2\r\nend");
}

TEST(TextUtilsTest, NormalizeLineEndingsHandlesEmptyString) {
    // Arrange: An empty wide string.
    std::wstring input = L"";

    // Act: Normalize line endings.
    std::wstring result = salt::NormalizeLineEndings(input);

    // Assert: Result is empty.
    EXPECT_EQ(result, L"");
}

TEST(TextUtilsTest, Utf8ToWideDecodesAsciiAndMultiByteUtf8) {
    // Arrange: UTF-8 encoded buffer containing ASCII and Unicode ("Salt, \u4E16\u754C").
    const char utf8_bytes[] = "Salt, \xE4\xB8\x96\xE7\x95\x8C";
    int length = static_cast<int>(sizeof(utf8_bytes) - 1);

    // Act: Decode UTF-8 buffer into wide string.
    std::wstring result = salt::Utf8ToWide(utf8_bytes, length);

    // Assert: Unicode code points are properly decoded into wchar_t string.
    EXPECT_EQ(result, L"Salt, \u4E16\u754C");
}

TEST(TextUtilsTest, Utf8ToWideReturnsEmptyStringForEmptyBuffer) {
    // Arrange: An empty buffer with zero length.
    const char empty_buf[] = "";

    // Act: Decode empty buffer.
    std::wstring result = salt::Utf8ToWide(empty_buf, 0);

    // Assert: Decoded string is empty.
    EXPECT_EQ(result, L"");
}

TEST(TextUtilsTest, Utf16LEToWideDecodesWithBom) {
    // Arrange: UTF-16 Little Endian byte buffer with BOM (0xFF, 0xFE) representing "Hi".
    // 'H' = 0x0048 (LE: 0x48, 0x00), 'i' = 0x0069 (LE: 0x69, 0x00)
    const unsigned char bytes[] = {0xFF, 0xFE, 0x48, 0x00, 0x69, 0x00};

    // Act: Decode buffer with LE BOM.
    std::wstring result =
        salt::Utf16LEToWide(reinterpret_cast<const char*>(bytes), sizeof(bytes));

    // Assert: BOM is stripped and string is properly decoded to "Hi".
    EXPECT_EQ(result, L"Hi");
}

TEST(TextUtilsTest, Utf16BEToWideDecodesWithBom) {
    // Arrange: UTF-16 Big Endian byte buffer with BOM (0xFE, 0xFF) representing "Hi".
    // 'H' = 0x0048 (BE: 0x00, 0x48), 'i' = 0x0069 (BE: 0x00, 0x69)
    const unsigned char bytes[] = {0xFE, 0xFF, 0x00, 0x48, 0x00, 0x69};

    // Act: Decode buffer with BE BOM.
    std::wstring result =
        salt::Utf16BEToWide(reinterpret_cast<const char*>(bytes), sizeof(bytes));

    // Assert: BOM is stripped and string is properly decoded to "Hi".
    EXPECT_EQ(result, L"Hi");
}

TEST(TextUtilsTest, DecodeTextFileDetectsUtf8WithBom) {
    // Arrange: Buffer starting with UTF-8 BOM (0xEF, 0xBB, 0xBF) followed by "Salt".
    const unsigned char bytes[] = {0xEF, 0xBB, 0xBF, 'S', 'a', 'l', 't'};

    // Act: Automatically detect and decode text buffer.
    std::wstring result =
        salt::DecodeTextFile(reinterpret_cast<const char*>(bytes), sizeof(bytes));

    // Assert: BOM is recognized and stripped, returning "Salt".
    EXPECT_EQ(result, L"Salt");
}

TEST(TextUtilsTest, DecodeTextFileDetectsUtf16LeWithBom) {
    // Arrange: Buffer with UTF-16 LE BOM (0xFF, 0xFE) and "Text".
    const unsigned char bytes[] = {0xFF, 0xFE, 'T', 0x00, 'e', 0x00, 'x', 0x00, 't', 0x00};

    // Act: Automatically detect and decode text buffer.
    std::wstring result =
        salt::DecodeTextFile(reinterpret_cast<const char*>(bytes), sizeof(bytes));

    // Assert: LE BOM is recognized and stripped, returning "Text".
    EXPECT_EQ(result, L"Text");
}

TEST(TextUtilsTest, DecodeTextFileDetectsUtf16BeWithBom) {
    // Arrange: Buffer with UTF-16 BE BOM (0xFE, 0xFF) and "Text".
    const unsigned char bytes[] = {0xFE, 0xFF, 0x00, 'T', 0x00, 'e', 0x00, 'x', 0x00, 't'};

    // Act: Automatically detect and decode text buffer.
    std::wstring result =
        salt::DecodeTextFile(reinterpret_cast<const char*>(bytes), sizeof(bytes));

    // Assert: BE BOM is recognized and stripped, returning "Text".
    EXPECT_EQ(result, L"Text");
}

TEST(TextUtilsTest, DecodeTextFileDecodesPlainUtf8WithoutBom) {
    // Arrange: Plain UTF-8 string without any BOM.
    const char text[] = "Pure C++ Win32 Text Editor";

    // Act: Decode text file.
    std::wstring result = salt::DecodeTextFile(text, static_cast<DWORD>(strlen(text)));

    // Assert: Decoded accurately to wide string.
    EXPECT_EQ(result, L"Pure C++ Win32 Text Editor");
}

TEST(TextUtilsTest, DecodeTextFileReturnsEmptyStringForEmptyBuffer) {
    // Arrange: Buffer with 0 size.
    const char empty[] = "";

    // Act: Decode empty buffer.
    std::wstring result = salt::DecodeTextFile(empty, 0);

    // Assert: Result is empty.
    EXPECT_EQ(result, L"");
}

}  // namespace
