load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

_WINMD_FILES = glob(["lib/**/*.winmd"], allow_empty = True)

filegroup(
    name = "winmd",
    srcs = _WINMD_FILES,
    visibility = ["//visibility:public"],
)

filegroup(
    name = "bootstrap_dll",
    srcs = ["runtimes/win-x64/native/Microsoft.WindowsAppRuntime.Bootstrap.dll"],
    visibility = ["//visibility:public"],
)

cc_import(
    name = "bootstrap_lib",
    interface_library = "lib/win10-x64/Microsoft.WindowsAppRuntime.Bootstrap.lib",
    shared_library = "runtimes/win-x64/native/Microsoft.WindowsAppRuntime.Bootstrap.dll",
)

cc_library(
    name = "libs",
    deps = [
        ":bootstrap_lib",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "headers",
    hdrs = glob(["include/**/*.h", "include/**/*.hpp"], allow_empty = True),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
