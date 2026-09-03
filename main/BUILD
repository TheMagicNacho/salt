# main/BUILD
load("@rules_cc//cc:defs.bzl", "cc_binary")

genrule(
    name = "resource",
    srcs = [
        "compile_resource.bat",
        "resources.rc",
        "//lib:resource.h",
        "resources/app.ico",
    ],
    outs = ["resource.res"],
    cmd_bat = "$(location compile_resource.bat) $(location resource.res) $(location resources.rc)",
)

cc_binary(
    name = "salt",
    visibility = ["//visibility:public"],
    srcs = [
        "main.cc",
    ],
    additional_linker_inputs = [
        ":resource",
    ],
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
        "$(location :resource)",
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
