#include <gtest/gtest.h>
#include <windows.h>
#include <string>

#include "lib/log-tail-handler.h"

namespace {

TEST(LogTailHandlerTest, InitialStateIsNotTailingAndNotReversed) {
    salt::LogTailHandler handler;
    EXPECT_FALSE(handler.IsTailing());
    EXPECT_FALSE(handler.IsReversed());
    EXPECT_EQ(handler.GetLineCount(), 0);
    EXPECT_EQ(handler.GetFormattedText(), L"");
}

TEST(LogTailHandlerTest, AppendTextSplitsLinesAndFormatsChronological) {
    salt::LogTailHandler handler;
    handler.AppendText(L"line 1\r\nline 2\r\nline 3\r\n");

    EXPECT_EQ(handler.GetLineCount(), 3);
    EXPECT_FALSE(handler.IsReversed());
    EXPECT_EQ(handler.GetFormattedText(), L"line 1\r\nline 2\r\nline 3");
}

TEST(LogTailHandlerTest, SetReversedFormatsLinesInTacOrder) {
    salt::LogTailHandler handler;
    handler.AppendText(L"first\r\nsecond\r\nthird\r\n");

    handler.SetReversed(true);
    EXPECT_TRUE(handler.IsReversed());
    EXPECT_EQ(handler.GetFormattedText(), L"third\r\nsecond\r\nfirst");

    handler.SetReversed(false);
    EXPECT_FALSE(handler.IsReversed());
    EXPECT_EQ(handler.GetFormattedText(), L"first\r\nsecond\r\nthird");
}

TEST(LogTailHandlerTest, DynamicAppendWhileReversedPutsNewestAtTop) {
    salt::LogTailHandler handler;
    handler.SetReversed(true);
    handler.AppendText(L"log entry 1\r\n");
    EXPECT_EQ(handler.GetFormattedText(), L"log entry 1");

    handler.AppendText(L"log entry 2\r\n");
    EXPECT_EQ(handler.GetFormattedText(), L"log entry 2\r\nlog entry 1");
}

TEST(LogTailHandlerTest, RingBufferEnforcesMaxLinesLimit) {
    salt::LogTailHandler handler;
    handler.SetMaxLines(3);

    handler.AppendText(L"line 1\r\nline 2\r\nline 3\r\nline 4\r\nline 5\r\n");

    EXPECT_EQ(handler.GetLineCount(), 3);
    // Oldest lines (line 1, line 2) should have been popped
    EXPECT_EQ(handler.GetFormattedText(), L"line 3\r\nline 4\r\nline 5");

    handler.SetReversed(true);
    EXPECT_EQ(handler.GetFormattedText(), L"line 5\r\nline 4\r\nline 3");
}

TEST(LogTailHandlerTest, HandlesPartialLineStreamingChunks) {
    salt::LogTailHandler handler;
    // Chunk 1 has no newline at end
    handler.AppendText(L"first part...");
    EXPECT_EQ(handler.GetLineCount(), 0);
    EXPECT_EQ(handler.GetFormattedText(), L"first part...");

    // Chunk 2 completes the line and starts another
    handler.AppendText(L" completed!\r\nsecond line\r\n");
    EXPECT_EQ(handler.GetLineCount(), 2);
    EXPECT_EQ(handler.GetFormattedText(), L"first part... completed!\r\nsecond line");
}

}  // namespace
