load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

cc_binary(
    name="salt",
    srcs=["main.cc"],
    deps=[
        "//lib:salt_lib",
    ],
    linkopts=[
        "/SUBSYSTEM:WINDOWS",  # Tells the linker to look for wWinMain instead of main
        "user32.lib",  # For CreateWindowEx, GetMessage, etc.
        "gdi32.lib",  # For drawing text and printing later
        "comdlg32.lib",  # For Open/Save dialogs later
    ],
)
