load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain", "use_cc_toolchain", "CC_TOOLCHAIN_ATTRS")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

def _cppwinrt_projection_impl(ctx):
    out_dir = ctx.actions.declare_directory(ctx.label.name)
    args = ctx.actions.args()
    args.add("-in", "local")
    for f in ctx.files.winmd_inputs:
        args.add("-in", f.dirname)
    args.add("-out", out_dir.path)
    ctx.actions.run(
        outputs = [out_dir],
        inputs = ctx.files.winmd_inputs,
        executable = ctx.executable._cppwinrt,
        arguments = [args],
        mnemonic = "CppWinrtProjection",
        execution_requirements = {"no-sandbox": "1"},
    )
    cc_toolchain = find_cc_toolchain(ctx)
    compilation_context = cc_common.create_compilation_context(
        includes = depset([out_dir.path]),
        headers = depset([out_dir]),
    )
    return [DefaultInfo(files = depset([out_dir])), CcInfo(compilation_context = compilation_context)]

cppwinrt_projection = rule(
    implementation = _cppwinrt_projection_impl,
    attrs = dict(
        CC_TOOLCHAIN_ATTRS,
        winmd_inputs = attr.label_list(allow_files = True),
        _cppwinrt = attr.label(default = "@cppwinrt_tool//:cppwinrt_exe", executable = True, cfg = "exec"),
    ),
    fragments = ["cpp"],
    toolchains = use_cc_toolchain(),
)
