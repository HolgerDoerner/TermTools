cmake_minimum_required(VERSION 3.0.0)
project(TermTools VERSION 0.1.4 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /nologo /W3 /EHsc")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /nologo /W3 /EHsc")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(TEST_FILES_DIR ${CMAKE_SOURCE_DIR}/etc/test-files)

# include(CTest)
enable_testing()

# set path to 3rd-party directory
set(3RD_PARTY ${PROJECT_SOURCE_DIR}/3rd-party)

# ==== BEGIN PDCurses ====
include(ExternalProject)
ExternalProject_Add(PDCurses 
    GIT_REPOSITORY https://github.com/wmcbrine/PDCurses.git
    PREFIX ${3RD_PARTY}
    SOURCE_DIR ${3RD_PARTY}/PDCurses
    TMP_DIR ${3RD_PARTY}/PDCurses_tmp
    STAMP_DIR ${3RD_PARTY}/PDCurses_stamp
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${3RD_PARTY}/PDCurses/wincon && nmake -f Makefile.vc WIDE=Y UTF8=Y
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

set(CURSES_NEED_CURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
set(CURSES_LIBRARY ${3RD_PARTY}/PDCurses/wincon/pdcurses.lib)
set(CURSES_INCLUDE_PATH ${3RD_PARTY}/PDCurses)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})
# ==== END PDCurses ====

# ==== START CUSTOM TARGETS ====
# adding a custom target for cleaning PDCurses.
add_custom_target("PDCurses_clean")
add_custom_command(TARGET "PDCurses_clean"
                    COMMENT "=== Cleaning: PDCurses ==="
                    WORKING_DIRECTORY ${PDCURSES_ROOT}/wincon
                    COMMAND nmake -f Makefile.vc clean)

add_custom_target("zip_release")
add_custom_command(TARGET "zip_release"
                    COMMENT "=== 7zip: Package Release Artefact ==="
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND 7z a TermTools-Release.zip $ENV{USERPROFILE}/TermTools)               
# ==== END CUSTOM TARGETS ====

# targets
add_subdirectory(pager)
add_subdirectory(counter)

# ==== START INSTALL ====
set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/TermTools" CACHE PATH "..." FORCE)

install(TARGETS pager counter
        CONFIGURATIONS Release)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES README.md DESTINATION ${CMAKE_INSTALL_PREFIX})
# ==== END INSTALL ====

message("General:")
message("\tCMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("\tCMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")
message("\tCMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message("\tEXECUTABLE_OUTPUT_PATH: ${EXECUTABLE_OUTPUT_PATH}")
message("\t3RD-PARTY DIR: ${3RD_PARTY}")
message("\tCMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}\n")

message("PDCURSES:")
message("\tCURSES_FOUND: ${CURSES_FOUND}")
message("\tCURSES_HAVE_CURSES_H: ${CURSES_HAVE_CURSES_H}")
message("\tCURSES_INCLUDE_DIRS: ${CURSES_INCLUDE_DIRS}")
message("\tCURSES_LIBRARIES: ${CURSES_LIBRARIES}")
message("\tCURSES_CFLAGS: ${CURSES_CFLAGS}\n")