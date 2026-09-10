load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@doxygen//:doxygen.bzl", "doxygen")

exports_files(["resource.h"])

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
        "text-utils.cc",
        "menu-bar.cc",
    ],
    hdrs=[
        "errors.h",
        "main-window.h",
        "file-handler.h",
        "options-handler.h",
        "resource.h",
        "text-utils.h",
        "menu-bar.h",
        "context.h",
    ],
    visibility=["//visibility:public"],
    linkopts = [
        "user32.lib",
        "gdi32.lib",
        "comctl32.lib",
        "comdlg32.lib",
        "dwmapi.lib",
        "uxtheme.lib",
        "ole32.lib",
        "shell32.lib",
        "menu-bar.h",
        "context.h",
    ],
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
        "text-utils.h",
        "text-utils.cc",
        "menu-bar.h",
        "menu-bar.cc",
        "context.h",
    ],
    project_name = "Salt Text Editor",
    project_brief = "Pure C++ Win32 Text Editor",
)
