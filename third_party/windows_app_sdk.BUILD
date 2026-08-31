load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

_WINMD_FILES = glob(["lib/**/*.winmd"], allow_empty = True)
_LIB_FILES = glob(["lib/win10-x64/*.lib", "lib/native/win10-x64/*.lib", "c++/**/*.lib"], allow_empty = True)

filegroup(
    name = "winmd",
    srcs = _WINMD_FILES,
    visibility = ["//visibility:public"],
)

[
    cc_import(
        name = "lib_import_" + str(idx),
        static_library = lib_file,
    )
    for idx, lib_file in enumerate(_LIB_FILES)
]

cc_library(
    name = "libs",
    deps = [":lib_import_" + str(idx) for idx in range(len(_LIB_FILES))],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "headers",
    hdrs = glob(["include/**/*.h", "include/**/*.hpp"], allow_empty = True),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
