load("@rules_cc//cc:defs.bzl", "cc_import", "cc_library")

# 1. Glob all the .lib files
_LIB_FILES = glob(["lib/**/*.lib", "c++/**/*.lib"], allow_empty = True)

# 2. Generate a cc_import for each file
[
    cc_import(
        name = "lib_import_" + str(idx),
        static_library = lib_file,
    )
    for idx, lib_file in enumerate(_LIB_FILES)
]

# 3. Expose a single cc_library that groups them all
cc_library(
    name = "libs",
    deps = [":lib_import_" + str(idx) for idx in range(len(_LIB_FILES))],
    visibility = ["//visibility:public"],
)
