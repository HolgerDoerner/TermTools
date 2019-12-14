cmake_minimum_required(VERSION 3.0.0)
project(TermTools VERSION 0.1.0)

include(CTest)
enable_testing()

option(PDCURSES_ROOT "test" C:/PDCurses)

# adding a custom target for building PDCurses.
# also used of some tools as a dependency.
add_custom_target("pdcurses_build")
add_custom_command(TARGET "pdcurses_build"
                    COMMENT "=== Building: PDCurses ==="
                    WORKING_DIRECTORY ${PDCURSES_ROOT}/wincon
                    COMMAND nmake -f Makefile.vc WIDE=Y UTF8=Y)

# adding a custom target for cleaning PDCurses.
add_custom_target("pdcurses_clean")
add_custom_command(TARGET "pdcurses_clean"
                    COMMENT "=== Cleaning: PDCurses ==="
                    WORKING_DIRECTORY ${PDCURSES_ROOT}/wincon
                    COMMAND nmake -f Makefile.vc clean)

add_subdirectory(tim)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
