load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "hello-greet",
    srcs = ["hello-greet.cpp"],
    hdrs = ["hello-greet.h"],
)

cc_binary(
    name = "salt",
    srcs = ["main.cpp"],
    deps = [
        ":hello-greet",
    ],
   linkopts = [
        "/SUBSYSTEM:WINDOWS",  # Tells the linker to look for wWinMain instead of main
        "user32.lib",          # For CreateWindowEx, GetMessage, etc.
        "gdi32.lib",           # For drawing text and printing later
        "comdlg32.lib",        # For Open/Save dialogs later
    ],
)
