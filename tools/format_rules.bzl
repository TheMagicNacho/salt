# tools/format_rules.bzl

def _format_test_impl(ctx):
    out = ctx.actions.declare_file(ctx.label.name + ".bat")
    content = """@echo off
setlocal enabledelayedexpansion

set "CHECKER_BAT="
if defined RUNFILES_MANIFEST_FILE (
    for /f "usebackq tokens=1,*" %%a in ("%RUNFILES_MANIFEST_FILE%") do (
        if "%%a"=="_main/tools/check_format.bat" set "CHECKER_BAT=%%b"
    )
)
if not defined CHECKER_BAT if defined TEST_SRCDIR if exist "%TEST_SRCDIR%\\MANIFEST" (
    for /f "usebackq tokens=1,*" %%a in ("%TEST_SRCDIR%\\MANIFEST") do (
        if "%%a"=="_main/tools/check_format.bat" set "CHECKER_BAT=%%b"
    )
)
if not defined CHECKER_BAT if defined BUILD_WORKSPACE_DIRECTORY (
    if exist "%BUILD_WORKSPACE_DIRECTORY%\\tools\\check_format.bat" set "CHECKER_BAT=%BUILD_WORKSPACE_DIRECTORY%\\tools\\check_format.bat"
)
if not defined CHECKER_BAT (
    if exist "%CD%\\tools\\check_format.bat" set "CHECKER_BAT=%CD%\\tools\\check_format.bat"
)
if not defined CHECKER_BAT (
    if exist "%~dp0..\\tools\\check_format.bat" set "CHECKER_BAT=%~dp0..\\tools\\check_format.bat"
)

if not defined CHECKER_BAT (
    echo Error: Could not locate check_format.bat in runfiles or workspace. >&2
    exit /b 1
)

for %%I in ("!CHECKER_BAT!") do set "CHECKER_DIR=%%~dpI"
cd /d "!CHECKER_DIR!..\\"
call "!CHECKER_BAT!" --check
exit /b !ERRORLEVEL!
"""
    ctx.actions.write(
        output = out,
        content = content,
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
    content = """@echo off
setlocal enabledelayedexpansion

set "CHECKER_BAT="
if defined BUILD_WORKSPACE_DIRECTORY (
    if exist "%BUILD_WORKSPACE_DIRECTORY%\\tools\\check_format.bat" set "CHECKER_BAT=%BUILD_WORKSPACE_DIRECTORY%\\tools\\check_format.bat"
)
if not defined CHECKER_BAT if defined RUNFILES_MANIFEST_FILE (
    for /f "usebackq tokens=1,*" %%a in ("%RUNFILES_MANIFEST_FILE%") do (
        if "%%a"=="_main/tools/check_format.bat" set "CHECKER_BAT=%%b"
    )
)
if not defined CHECKER_BAT if defined TEST_SRCDIR if exist "%TEST_SRCDIR%\\MANIFEST" (
    for /f "usebackq tokens=1,*" %%a in ("%TEST_SRCDIR%\\MANIFEST") do (
        if "%%a"=="_main/tools/check_format.bat" set "CHECKER_BAT=%%b"
    )
)
if not defined CHECKER_BAT (
    if exist "%CD%\\tools\\check_format.bat" set "CHECKER_BAT=%CD%\\tools\\check_format.bat"
)
if not defined CHECKER_BAT (
    if exist "%~dp0..\\tools\\check_format.bat" set "CHECKER_BAT=%~dp0..\\tools\\check_format.bat"
)

if not defined CHECKER_BAT (
    echo Error: Could not locate check_format.bat in runfiles or workspace. >&2
    exit /b 1
)

for %%I in ("!CHECKER_BAT!") do set "CHECKER_DIR=%%~dpI"
cd /d "!CHECKER_DIR!..\\"
call "!CHECKER_BAT!" --fix
exit /b !ERRORLEVEL!
"""
    ctx.actions.write(
        output = out,
        content = content,
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
