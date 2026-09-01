# main/BUILD
load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "salt",
    srcs = ["main.cc"],
    defines = [
        "UNICODE",
        "_UNICODE",
        "WINVER=0x0A00",
        "_WIN32_WINNT=0x0A00",
    ],
    deps = [
        "//lib:salt_lib",
    ],
    linkopts = [
        "/SUBSYSTEM:WINDOWS",
        "user32.lib",
        "gdi32.lib",
        "comctl32.lib",
        "comdlg32.lib",
        "dwmapi.lib",
        "uxtheme.lib",
        "ole32.lib",
        "shell32.lib",
    ],
)
