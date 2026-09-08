@echo off
setlocal enabledelayedexpansion

:: Check mode: default is --check, --fix applies in-place
set "MODE=check"
if "%~1"=="--fix" set "MODE=fix"
if "%~1"=="-i" set "MODE=fix"

:: Find clang-format.exe
set "CF_CMD="

:: 1. Check if clang-format is already on PATH
where clang-format.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "CF_CMD=clang-format.exe"
    goto :found_cf
)

:: 2. Check standard LLVM install path
if exist "C:\Program Files\LLVM\bin\clang-format.exe" (
    set "CF_CMD=C:\Program Files\LLVM\bin\clang-format.exe"
    goto :found_cf
)

:: 3. Check Visual Studio LLVM tool paths
for %%v in (2022 2019) do (
    for %%e in (Enterprise Professional Community BuildTools) do (
        if exist "C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Tools\Llvm\x64\bin\clang-format.exe" (
            set "CF_CMD=C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Tools\Llvm\x64\bin\clang-format.exe"
            goto :found_cf
        )
        if exist "C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Tools\Llvm\bin\clang-format.exe" (
            set "CF_CMD=C:\Program Files\Microsoft Visual Studio\%%v\%%e\VC\Tools\Llvm\bin\clang-format.exe"
            goto :found_cf
        )
    )
)

if not defined CF_CMD (
    echo Error: Could not locate clang-format.exe. Please ensure clang-format or LLVM / Visual Studio Clang tools are installed. >&2
    exit /b 1
)

:found_cf
if "%MODE%"=="fix" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "$files = Get-ChildItem -Path . -Recurse -Include *.cc, *.h | Where-Object { $_.FullName -notmatch '[\\\\/]bazel-' -and $_.FullName -notmatch '[\\\\/]\.git' };" ^
        "if ($files.Count -eq 0) { Write-Host 'No files found to format.'; exit 0 };" ^
        "& '%CF_CMD%' -i $files.FullName;" ^
        "Write-Host ('Formatted ' + $files.Count + ' files.')"
    exit /b %ERRORLEVEL%
) else (
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "$files = Get-ChildItem -Path . -Recurse -Include *.cc, *.h | Where-Object { $_.FullName -notmatch '[\\\\/]bazel-' -and $_.FullName -notmatch '[\\\\/]\.git' };" ^
        "if ($files.Count -eq 0) { exit 0 };" ^
        "$failed = 0;" ^
        "foreach ($f in $files) {" ^
        "    & '%CF_CMD%' --dry-run --Werror $f.FullName 2>&1 | Out-Null;" ^
        "    if ($LASTEXITCODE -ne 0) {" ^
        "        Write-Host ('Format violation detected: ' + $f.FullName);" ^
        "        $failed = 1;" ^
        "    }" ^
        "};" ^
        "if ($failed -ne 0) {" ^
        "    Write-Error 'Clang-format check failed! Run `bazelisk run //:format` to auto-format.';" ^
        "    exit 1;" ^
        "} else {" ^
        "    Write-Host ('Clang-format check passed for ' + $files.Count + ' files.');" ^
        "}"
    exit /b %ERRORLEVEL%
)
