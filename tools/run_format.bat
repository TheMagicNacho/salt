@echo off
setlocal
call "%~dp0check_format.bat" --fix
exit /b %ERRORLEVEL%
