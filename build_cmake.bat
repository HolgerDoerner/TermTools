@echo off

if "%1"=="/?" (
    goto :usage
)

if [%1]==[] (
    set CMAKE_BUILD_TYPE=Debug
) else (
    set CMAKE_BUILD_TYPE=%1
)

if [%2]==[] (
    set GENERATOR=NMake Makefiles
) else (
    set GENERATOR=%2
)

echo ===========================================
echo === GENERATING CMAKE PROJECT: TermTools ===
echo ===========================================
echo.

call cmake -DCMAKE_BUILD_TYPE=%CMAKE_BUILD_TYPE% -G"%GENERATOR%" -B .\build

cd .\build

echo.
echo You are now at build-directory of the TermTools-Project.
echo.
echo Use 'nmake' or 'nmake all' to build the Targets,
echo or 'name help' to get a list of available Targets.
echo.

goto :eof

:usage
echo build_cmake.bat [CMAKE_BUILD_TYPE] [GENERATOR]
echo.

@echo on