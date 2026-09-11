#ifndef SALT_LIB_TEXT_UTILS_H_
#define SALT_LIB_TEXT_UTILS_H_

#include <windows.h>
#include <string>

namespace salt {

/// @brief Normalizes line endings in the given text to Windows \r\n format.
std::wstring NormalizeLineEndings(const std::wstring& text);

/// @brief Decodes a UTF-8 byte buffer into a wide string.
std::wstring Utf8ToWide(const char* raw_buffer, int raw_len);

/// @brief Decodes a UTF-16 Little Endian byte buffer into a wide string.
std::wstring Utf16LEToWide(const char* raw_buffer, int raw_len);

/// @brief Decodes a UTF-16 Big Endian byte buffer into a wide string.
std::wstring Utf16BEToWide(const char* raw_buffer, int raw_len);

/// @brief Automatically detects encoding (UTF-8 with/without BOM, UTF-16 LE, UTF-16 BE, ACP) and
/// decodes text.
std::wstring DecodeTextFile(const char* raw_buffer, DWORD dw_size);

}  // namespace salt

#endif  // SALT_LIB_TEXT_UTILS_H_
