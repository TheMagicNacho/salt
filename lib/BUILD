load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@doxygen//:doxygen.bzl", "doxygen")

cc_library(
    name="salt_lib",
    defines = [
        "UNICODE",
        "_UNICODE",
        "WINVER=0x0A00",
        "_WIN32_WINNT=0x0A00",
    ],
    srcs=[
        "errors.cc",
        "main-window.cc",
        "file-handler.cc",
        "options-handler.cc",
    ],
    hdrs=[
        "errors.h",
        "main-window.h",
        "file-handler.h",
        "options-handler.h",
    ],
    visibility=["//main:__pkg__"],
)

doxygen(
    name = "docs",
    srcs = [
        "file-handler.h",
        "file-handler.cc",
        "options-handler.h",
        "options-handler.cc",
        "main-window.h",
        "main-window.cc",
    ],
    project_name = "Salt Text Editor",
    project_brief = "Pure C++ Win32 Text Editor",
)
