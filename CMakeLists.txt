cmake_minimum_required(VERSION 3.0.0)
project(TermTools VERSION 0.1.0)

include(CTest)
enable_testing()

option(PDCURSES_ROOT "test" C:/PDCurses)

# adding a custom target for building PDCurses.
# also used of some tools as a dependency.
add_custom_target("build_pdcurses")
add_custom_command(TARGET "build_pdcurses"
                    COMMENT "=== Building: PDCurses ==="
                    WORKING_DIRECTORY ${PDCURSES_ROOT}/wincon
                    COMMAND nmake -f Makefile.vc WIDE=Y UTF8=Y)

add_subdirectory(tim)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
