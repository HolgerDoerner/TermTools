@echo off

setlocal enabledelayedexpansion

set HOME=%HOMEDRIVE%%HOMEPATH%\TermTools

:: remove existing entry from path (if any)
set TMP=PATH:%HOME%;=
call set "PATH=%%%TMP%%%"

echo * Setting PATH entrys for TermTools (current user)

reg add HKCU\Environment /V TERMTOOLS_HOME /T REG_SZ /D "%HOME%" /F > NUL 2>&1
if %ERRORLEVEL% neq 0 goto :error
reg add HKCU\Environment /V Path /T REG_SZ /D "%PATH%;%HOME%\bin" /F > NUL 2>&1
if %ERRORLEVEL% neq 0 goto :error

echo * DONE
echo.
goto :eof

:error
echo * Error setting environment variables !!!
echo.

@echo on