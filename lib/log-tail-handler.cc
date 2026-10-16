#include "lib/log-tail-handler.h"
#include "lib/context.h"
#include "lib/text-utils.h"

#include <algorithm>
#include <vector>

namespace salt {

LogTailHandler::LogTailHandler() = default;

LogTailHandler::~LogTailHandler() { StopTail(); }

bool LogTailHandler::IsTailing() const { return is_tailing_.load(); }

bool LogTailHandler::IsReversed() const { return is_reversed_.load(); }

void LogTailHandler::SetReversed(bool reverse) { is_reversed_.store(reverse); }

std::wstring LogTailHandler::GetFilePath() const { return file_path_; }

size_t LogTailHandler::GetLineCount() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return lines_.size();
}

void LogTailHandler::SetMaxLines(size_t max_lines) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    max_lines_ = max_lines > 0 ? max_lines : 1;
    while (lines_.size() > max_lines_) {
        lines_.pop_front();
    }
}

void LogTailHandler::ClearBuffer() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    lines_.clear();
    carryover_partial_line_.clear();
    file_offset_ = 0;
}

bool LogTailHandler::StartTail(HWND notify_hwnd, const std::wstring& file_path, bool reverse) {
    StopTail();

    if (file_path.empty()) {
        return false;
    }

    notify_hwnd_ = notify_hwnd;
    file_path_ = file_path;
    is_reversed_.store(reverse);
    stop_requested_.store(false);

    // Extract directory and filename
    size_t last_slash = file_path_.find_last_of(L"\\/");
    if (last_slash != std::wstring::npos) {
        dir_path_ = file_path_.substr(0, last_slash);
        file_name_ = file_path_.substr(last_slash + 1);
    } else {
        dir_path_ = L".";
        file_name_ = file_path_;
    }

    // Open file with full sharing permissions
    file_handle_ = CreateFileW(file_path_.c_str(), GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    ClearBuffer();

    // Read existing file contents
    PollFileChanges();

    is_tailing_.store(true);

    // Spawn watcher thread using ReadDirectoryChangesW
    dir_change_event_ = CreateEventW(NULL, TRUE, FALSE, NULL);
    watcher_thread_ = std::thread(&LogTailHandler::WatcherThreadProc, this);

    return true;
}

void LogTailHandler::StopTail() {
    stop_requested_.store(true);
    is_tailing_.store(false);

    if (dir_change_event_) {
        SetEvent(dir_change_event_);
    }

    if (watcher_thread_.joinable()) {
        watcher_thread_.join();
    }

    if (dir_change_event_) {
        CloseHandle(dir_change_event_);
        dir_change_event_ = NULL;
    }

    if (file_handle_ != INVALID_HANDLE_VALUE) {
        CloseHandle(file_handle_);
        file_handle_ = INVALID_HANDLE_VALUE;
    }

    file_offset_ = 0;
    notify_hwnd_ = nullptr;
}

void LogTailHandler::AppendText(const std::wstring& raw_text) {
    if (raw_text.empty()) {
        return;
    }

    std::wstring normalized = NormalizeLineEndings(raw_text);

    std::lock_guard<std::mutex> lock(buffer_mutex_);
    std::wstring combined = carryover_partial_line_ + normalized;
    carryover_partial_line_.clear();

    size_t start = 0;
    while (start < combined.size()) {
        size_t pos = combined.find(L"\r\n", start);
        if (pos == std::wstring::npos) {
            // Incomplete trailing line, store for next chunk
            carryover_partial_line_ = combined.substr(start);
            break;
        } else {
            lines_.push_back(combined.substr(start, pos - start));
            start = pos + 2;
        }
    }

    // Enforce ring buffer max line limit
    while (lines_.size() > max_lines_) {
        lines_.pop_front();
    }
}

bool LogTailHandler::PollFileChanges() {
    if (file_handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(file_handle_, &file_size)) {
        return false;
    }

    uint64_t current_size = static_cast<uint64_t>(file_size.QuadPart);
    if (current_size < file_offset_) {
        // File was truncated or rolled over! Reset offset and buffer
        file_offset_ = 0;
        ClearBuffer();
    }

    if (current_size == file_offset_) {
        return false;
    }

    // Seek to last read offset
    LARGE_INTEGER seek_pos;
    seek_pos.QuadPart = static_cast<LONGLONG>(file_offset_);
    if (!SetFilePointerEx(file_handle_, seek_pos, NULL, FILE_BEGIN)) {
        return false;
    }

    DWORD bytes_to_read = static_cast<DWORD>(
        (std::min)(current_size - file_offset_, static_cast<uint64_t>(1024 * 1024)));
    std::vector<char> buffer(bytes_to_read);
    DWORD bytes_read = 0;

    if (!ReadFile(file_handle_, buffer.data(), bytes_to_read, &bytes_read, NULL) ||
        bytes_read == 0) {
        return false;
    }

    file_offset_ += bytes_read;

    std::wstring wide = DecodeTextFile(buffer.data(), bytes_read);
    AppendText(wide);

    return true;
}

std::wstring LogTailHandler::GetFormattedText() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    if (lines_.empty() && carryover_partial_line_.empty()) {
        return L"";
    }

    std::wstring result;
    size_t estimated_len = 0;
    for (const auto& line : lines_) {
        estimated_len += line.size() + 2;
    }
    estimated_len += carryover_partial_line_.size() + 2;
    result.reserve(estimated_len);

    if (is_reversed_.load()) {
        // Reverse mode (tac): newest at the top
        if (!carryover_partial_line_.empty()) {
            result += carryover_partial_line_;
            if (!lines_.empty()) {
                result += L"\r\n";
            }
        }
        for (auto it = lines_.rbegin(); it != lines_.rend(); ++it) {
            result += *it;
            if (std::next(it) != lines_.rend()) {
                result += L"\r\n";
            }
        }
    } else {
        // Normal chronological mode
        for (size_t i = 0; i < lines_.size(); ++i) {
            result += lines_[i];
            if (i + 1 < lines_.size() || !carryover_partial_line_.empty()) {
                result += L"\r\n";
            }
        }
        if (!carryover_partial_line_.empty()) {
            result += carryover_partial_line_;
        }
    }

    return result;
}

void LogTailHandler::WatcherThreadProc() {
    HANDLE dir_handle =
        CreateFileW(dir_path_.c_str(), FILE_LIST_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                    FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (dir_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    BYTE change_buf[1024];
    while (!stop_requested_.load()) {
        ResetEvent(overlapped.hEvent);
        DWORD bytes_returned = 0;
        BOOL read_ok =
            ReadDirectoryChangesW(dir_handle, change_buf, sizeof(change_buf), FALSE,
                                  FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
                                  &bytes_returned, &overlapped, NULL);

        if (!read_ok && GetLastError() != ERROR_IO_PENDING) {
            break;
        }

        HANDLE wait_events[2] = {dir_change_event_, overlapped.hEvent};
        DWORD wait_res = WaitForMultipleObjects(2, wait_events, FALSE, 1000);

        if (wait_res == WAIT_OBJECT_0) {
            // Stop event signaled
            CancelIo(dir_handle);
            break;
        } else if (wait_res == WAIT_OBJECT_0 + 1) {
            // Directory changed! Notify UI
            if (notify_hwnd_ && !stop_requested_.load()) {
                PostMessageW(notify_hwnd_, Context::kMsgLogUpdated, 0, 0);
            }
        } else if (wait_res == WAIT_TIMEOUT) {
            // Check if stop requested
            if (stop_requested_.load()) {
                break;
            }
        }
    }

    CloseHandle(overlapped.hEvent);
    CloseHandle(dir_handle);
}

}  // namespace salt
