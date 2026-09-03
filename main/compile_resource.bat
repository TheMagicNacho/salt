@echo off
setlocal enabledelayedexpansion

:: Check if rc.exe is already on PATH
where rc.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "RC_CMD=rc.exe"
    goto :compile
)

set "RC_CMD="

:: 1. Check ProgramFiles(x86) variable
if defined ProgramFiles(x86) (
    for /d %%d in ("%ProgramFiles(x86)%\Windows Kits\10\bin\10.*") do (
        if exist "%%d\x64\rc.exe" set "RC_CMD=%%d\x64\rc.exe"
    )
)

:: 2. Check standard 64-bit Program Files variable
if not defined RC_CMD (
    if defined ProgramFiles (
        for /d %%d in ("%ProgramFiles%\Windows Kits\10\bin\10.*") do (
            if exist "%%d\x64\rc.exe" set "RC_CMD=%%d\x64\rc.exe"
        )
    )
)

:: 3. Direct hardcoded fallback paths for standard Windows SDK installations
if not defined RC_CMD (
    for /d %%d in ("C:\Program Files (x86)\Windows Kits\10\bin\10.*") do (
        if exist "%%d\x64\rc.exe" set "RC_CMD=%%d\x64\rc.exe"
    )
)

if not defined RC_CMD (
    for /d %%d in ("C:\Program Files\Windows Kits\10\bin\10.*") do (
        if exist "%%d\x64\rc.exe" set "RC_CMD=%%d\x64\rc.exe"
    )
)

if not defined RC_CMD (
    echo Error: Could not locate rc.exe in Windows SDK paths. Please ensure Windows 10/11 SDK is installed. >&2
    exit /b 1
)

:compile
"%RC_CMD%" /nologo /i . /i lib /i main /i main\resources /fo "%~1" "%~2"
if %ERRORLEVEL% neq 0 (
    exit /b %ERRORLEVEL%
)
