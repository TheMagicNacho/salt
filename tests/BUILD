load("@rules_cc//cc:cc_test.bzl", "cc_test")

COMMON_DEFINES = [
    "UNICODE",
    "_UNICODE",
    "WINVER=0x0A00",
    "_WIN32_WINNT=0x0A00",
]

COMMON_DEPS = [
    "//lib:salt_lib",
    "@googletest//:gtest_main",
]

cc_test(
    name = "options_handler_test",
    srcs = ["options_handler_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

cc_test(
    name = "file_handler_test",
    srcs = ["file_handler_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

cc_test(
    name = "text_utils_test",
    srcs = ["text_utils_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

cc_test(
    name = "menu_bar_test",
    srcs = ["menu_bar_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

cc_test(
    name = "main_window_test",
    srcs = ["main_window_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

cc_test(
    name = "errors_test",
    srcs = ["errors_test.cc"],
    defines = COMMON_DEFINES,
    deps = COMMON_DEPS,
)

test_suite(
    name = "all_tests",
    tests = [
        ":options_handler_test",
        ":file_handler_test",
        ":text_utils_test",
        ":menu_bar_test",
        ":main_window_test",
        ":errors_test",
    ],
)
