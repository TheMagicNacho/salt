#ifndef SALT_LIB_LOG_TAIL_HANDLER_H_
#define SALT_LIB_LOG_TAIL_HANDLER_H_

#include <windows.h>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>

/// @file log-tail-handler.h
/// @brief Log file tailing (follow) and reversed lines (tac) support for Salt.

namespace salt {

/// @class LogTailHandler
/// @brief Manages live tailing of log files with support for normal and reversed line ordering.
///
/// Watches file changes using ReadDirectoryChangesW supplemented by a fallback timer,
/// maintains an in-memory ring buffer (default 10,000 lines), and generates text ready
/// for display in the Win32 edit control.
class LogTailHandler {
   public:
    static constexpr size_t kDefaultMaxLines = 10000;

    LogTailHandler();
    ~LogTailHandler();

    /// @brief Starts tailing the given file.
    /// @param notify_hwnd Window to receive Context::kMsgLogUpdated notifications.
    /// @param file_path Path to the file to tail.
    /// @param reverse Whether to format lines in reversed (tac) order.
    /// @return true if tailing was successfully started, false otherwise.
    bool StartTail(HWND notify_hwnd, const std::wstring& file_path, bool reverse = false);

    /// @brief Stops active tailing and cleans up background threads and handles.
    void StopTail();

    /// @brief Checks whether tailing is currently active.
    bool IsTailing() const;

    /// @brief Checks whether reversed (tac) mode is active.
    bool IsReversed() const;

    /// @brief Sets reversed (tac) mode.
    void SetReversed(bool reversed);

    /// @brief Reads newly appended content and updates the internal ring buffer.
    /// @return true if new content was read, false otherwise.
    bool PollFileChanges();

    /// @brief Formats the current buffer lines into a wide string with CRLF.
    /// @return Rendered text buffer.
    std::wstring GetFormattedText() const;

    /// @brief Gets the current file path being tailed.
    std::wstring GetFilePath() const;

    /// @brief Appends a batch of text into the internal line ring buffer (useful for unit testing).
    /// @param raw_text Text to parse into lines and append to the buffer.
    void AppendText(const std::wstring& raw_text);

    /// @brief Clears the line buffer.
    void ClearBuffer();

    /// @brief Returns the current number of lines in the ring buffer.
    size_t GetLineCount() const;

    /// @brief Sets the maximum lines kept in the ring buffer.
    void SetMaxLines(size_t max_lines);

   private:
    void WatcherThreadProc();

    HWND notify_hwnd_{nullptr};
    std::wstring file_path_;
    std::wstring dir_path_;
    std::wstring file_name_;
    HANDLE file_handle_{INVALID_HANDLE_VALUE};
    uint64_t file_offset_{0};
    size_t max_lines_{kDefaultMaxLines};

    std::atomic<bool> is_tailing_{false};
    std::atomic<bool> is_reversed_{false};
    std::atomic<bool> stop_requested_{false};

    mutable std::mutex buffer_mutex_;
    std::deque<std::wstring> lines_;
    std::wstring carryover_partial_line_;

    std::thread watcher_thread_;
    HANDLE dir_change_event_{NULL};
};

}  // namespace salt

#endif  // SALT_LIB_LOG_TAIL_HANDLER_H_
