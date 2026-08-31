# main\BUILD
load("//tools/windows:cppwinrt_projection.bzl", "cppwinrt_projection")
load("@rules_cc//cc:defs.bzl", "cc_binary")

cppwinrt_projection(
    name = "winrt_headers",
    winmd_inputs = [
        "@windows_app_sdk//:winmd",
        "@webview2//:winmd",
    ],
)

cc_binary(
    name = "salt",
    srcs = ["main.cc"],
    deps = [
        ":winrt_headers",
        "@windows_app_sdk//:headers",
        "@windows_app_sdk//:libs",
        "//lib:salt_lib",
    ],
    linkopts = [
        "/SUBSYSTEM:WINDOWS",
        "WindowsApp.lib",
        "runtimeobject.lib",
    ],
)
