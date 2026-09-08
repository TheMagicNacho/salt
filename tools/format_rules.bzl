# tools/format_rules.bzl

def _format_test_impl(ctx):
    out = ctx.actions.declare_file(ctx.label.name + ".bat")
    ctx.actions.write(
        output = out,
        content = "@echo off\r\ncall \"%s\" --check\r\nif %%ERRORLEVEL%% neq 0 exit /b %%ERRORLEVEL%%\r\n" % ctx.file.checker.path.replace("/", "\\"),
        is_executable = True,
    )
    return [DefaultInfo(
        executable = out,
        runfiles = ctx.runfiles(files = [ctx.file.checker] + ctx.files.srcs + [ctx.file.config]),
    )]

format_test = rule(
    implementation = _format_test_impl,
    test = True,
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "config": attr.label(allow_single_file = True, mandatory = True),
        "checker": attr.label(allow_single_file = True, mandatory = True),
    },
)

def _format_fix_impl(ctx):
    out = ctx.actions.declare_file(ctx.label.name + ".bat")
    ctx.actions.write(
        output = out,
        content = "@echo off\r\ncall \"%s\" --fix\r\nif %%ERRORLEVEL%% neq 0 exit /b %%ERRORLEVEL%%\r\n" % ctx.file.checker.path.replace("/", "\\"),
        is_executable = True,
    )
    return [DefaultInfo(
        executable = out,
        runfiles = ctx.runfiles(files = [ctx.file.checker] + ctx.files.srcs + [ctx.file.config]),
    )]

format_fix = rule(
    implementation = _format_fix_impl,
    executable = True,
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "config": attr.label(allow_single_file = True, mandatory = True),
        "checker": attr.label(allow_single_file = True, mandatory = True),
    },
)
