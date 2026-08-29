load("@rules_cc//cc:cc_library.bzl", "cc_library")


cc_library(
    name="salt_lib",
    srcs=["defaults.cc", "hello-greet.cc", "file-handler.cc"],
    hdrs=["defaults.h", "hello-greet.h", "file-handler.h"],
    visibility=["//main:__pkg__"],
)
