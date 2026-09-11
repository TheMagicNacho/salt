#ifndef SALT_LIB_TEXT_UTILS_H_
#define SALT_LIB_TEXT_UTILS_H_

#include <windows.h>
#include <string>

/// @file text-utils.h
/// @brief Text encoding detection, wide string conversions, and newline normalization.

namespace salt {

/// @brief Normalizes line endings in the given text to Windows CRLF (\\r\\n) format.
/// @param text Input wide string.
/// @return Wide string with all line endings normalized to \\r\\n.
std::wstring NormalizeLineEndings(const std::wstring& text);


/// @brief Decodes a UTF-8 byte buffer into a wide string.
/// @param raw_buffer Pointer to UTF-8 encoded byte buffer.
/// @param raw_len Number of bytes in the buffer.
/// @return Decoded wide string.
std::wstring Utf8ToWide(const char* raw_buffer, int raw_len);

/// @brief Decodes a UTF-16 Little Endian byte buffer into a wide string.
/// @param raw_buffer Pointer to UTF-16 LE encoded byte buffer.
/// @param raw_len Number of bytes in the buffer.
/// @return Decoded wide string.
std::wstring Utf16LEToWide(const char* raw_buffer, int raw_len);

/// @brief Decodes a UTF-16 Big Endian byte buffer into a wide string.
/// @param raw_buffer Pointer to UTF-16 BE encoded byte buffer.
/// @param raw_len Number of bytes in the buffer.
/// @return Decoded wide string.
std::wstring Utf16BEToWide(const char* raw_buffer, int raw_len);

/// @brief Automatically detects encoding (UTF-8 with/without BOM, UTF-16 LE, UTF-16 BE, ACP) and
/// decodes text.
/// @param raw_buffer Pointer to raw file byte buffer.
/// @param buffer_size Total size of the buffer in bytes.
/// @return Decoded wide string.
std::wstring DecodeTextFile(const char* raw_buffer, DWORD buffer_size);

}  // namespace salt

#endif  // SALT_LIB_TEXT_UTILS_H_

