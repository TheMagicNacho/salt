load("@rules_cc//cc:cc_test.bzl", "cc_test")

cc_test(
    name = "options_handler_test",
    srcs = ["options_handler_test.cc"],
    defines = [
        "UNICODE",
        "_UNICODE",
        "WINVER=0x0A00",
        "_WIN32_WINNT=0x0A00",
    ],
    linkopts = [
        "user32.lib",
        "gdi32.lib",
        "comctl32.lib",
        "comdlg32.lib",
    ],
    deps = [
        "//lib:salt_lib",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "file_handler_test",
    srcs = ["file_handler_test.cc"],
    defines = [
        "UNICODE",
        "_UNICODE",
        "WINVER=0x0A00",
        "_WIN32_WINNT=0x0A00",
    ],
    linkopts = [
        "user32.lib",
        "gdi32.lib",
        "comctl32.lib",
        "comdlg32.lib",
        "ole32.lib",
        "shell32.lib",
    ],
    deps = [
        "//lib:salt_lib",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "text_utils_test",
    srcs = ["text_utils_test.cc"],
    defines = [
        "UNICODE",
        "_UNICODE",
        "WINVER=0x0A00",
        "_WIN32_WINNT=0x0A00",
    ],
    linkopts = [
        "user32.lib",
    ],
    deps = [
        "//lib:salt_lib",
        "@googletest//:gtest_main",
    ],
)

test_suite(
    name = "all_tests",
    tests = [
        ":options_handler_test",
        ":file_handler_test",
        ":text_utils_test",
    ],
)
