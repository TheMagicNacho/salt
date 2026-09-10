#include "text-utils.h"

namespace salt {

std::wstring NormalizeLineEndings(const std::wstring& text) {
    std::wstring normalized;
    normalized.reserve(text.size() * 6 / 5);

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == L'\r') {
            normalized.push_back(L'\r');
            // Look ahead: if next char is '\n', consume it to avoid double-adding
            if (i + 1 < text.size() && text[i + 1] == L'\n') {
                normalized.push_back(L'\n');
                ++i;
            } else {
                // Lone \r -> convert to \r\n
                normalized.push_back(L'\n');
            }
        } else if (text[i] == L'\n') {
            // Lone \n -> convert to \r\n
            normalized.push_back(L'\r');
            normalized.push_back(L'\n');
        } else {
            normalized.push_back(text[i]);
        }
    }

    return normalized;
}

std::wstring Utf8ToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, raw_buffer, raw_len, NULL, 0);
    if (wideLen <= 0) return L"";

    std::wstring wide_buffer(wideLen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, raw_buffer, raw_len, &wide_buffer[0], wideLen);

    if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
        wide_buffer.pop_back();
    }

    return wide_buffer;
}

std::wstring Utf16LEToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int offset = 0;
    if (raw_len >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFE) {
        offset = 2;
    }

    int char_count = (raw_len - offset) / sizeof(wchar_t);
    if (char_count <= 0) return L"";

    std::wstring wide_buffer(char_count, L'\0');

    for (int i = 0; i < char_count; ++i) {
        const unsigned char* p =
            reinterpret_cast<const unsigned char*>(raw_buffer + offset + (i * sizeof(wchar_t)));
        wchar_t ch = static_cast<wchar_t>(static_cast<unsigned short>(p[0]) |
                                          (static_cast<unsigned short>(p[1]) << 8));
        wide_buffer[i] = ch;
    }

    return wide_buffer;
}

std::wstring Utf16BEToWide(const char* raw_buffer, int raw_len) {
    if (raw_len <= 0) return L"";

    int offset = 0;
    if (raw_len >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFE &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFF) {
        offset = 2;
    }

    int char_count = (raw_len - offset) / sizeof(wchar_t);
    if (char_count <= 0) return L"";

    std::wstring wideBuffer(char_count, L'\0');

    for (int i = 0; i < char_count; ++i) {
        const unsigned char* p =
            reinterpret_cast<const unsigned char*>(raw_buffer + offset + (i * sizeof(wchar_t)));
        wchar_t ch = static_cast<wchar_t>((static_cast<unsigned short>(p[0]) << 8) |
                                          static_cast<unsigned short>(p[1]));
        wideBuffer[i] = ch;
    }

    return wideBuffer;
}

std::wstring DecodeTextFile(const char* raw_buffer, DWORD dw_size) {
    if (dw_size == 0) return L"";

    if (dw_size >= 3 && static_cast<unsigned char>(raw_buffer[0]) == 0xEF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xBB &&
        static_cast<unsigned char>(raw_buffer[2]) == 0xBF) {
        return Utf8ToWide(raw_buffer + 3, static_cast<int>(dw_size - 3));
    }

    if (dw_size >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFF &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFE) {
        return Utf16LEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    if (dw_size >= 2 && static_cast<unsigned char>(raw_buffer[0]) == 0xFE &&
        static_cast<unsigned char>(raw_buffer[1]) == 0xFF) {
        return Utf16BEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    size_t zero_count = 0;
    for (DWORD i = 0; i < dw_size; ++i) {
        if (raw_buffer[i] == 0) ++zero_count;
    }
    if (zero_count > dw_size / 4) {
        return Utf16LEToWide(raw_buffer, static_cast<int>(dw_size));
    }

    int wideLen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw_buffer, dw_size, NULL, 0);
    if (wideLen > 0) {
        std::wstring wide_buffer(wideLen, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw_buffer, dw_size, &wide_buffer[0],
                            wideLen);

        if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
            wide_buffer.pop_back();
        }
        return wide_buffer;
    }

    wideLen = MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, NULL, 0);
    if (wideLen > 0) {
        std::wstring wide_buffer(wideLen, L'\0');
        MultiByteToWideChar(CP_ACP, 0, raw_buffer, dw_size, &wide_buffer[0], wideLen);

        if (!wide_buffer.empty() && wide_buffer.back() == L'\0') {
            wide_buffer.pop_back();
        }
        return wide_buffer;
    }

    return L"";
}

}  // namespace salt
