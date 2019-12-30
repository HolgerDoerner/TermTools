cmake_minimum_required(VERSION 3.0.0)
project(TermTools  VERSION 0.4.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /nologo /W3 /EHsc /UTF-8")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /nologo /Ox /W3 /EHsc /UTF-8")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(TEST_FILES_DIR ${CMAKE_SOURCE_DIR}/etc/test-files)

# project-wide includes
include_directories(${CMAKE_SOURCE_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/include/tt_version.h.in
                ${CMAKE_SOURCE_DIR}/include/tt_version.h
                WIN32)

# include(CTest)
enable_testing()

# set path to 3rd-party directory
set(3RD_PARTY ${CMAKE_SOURCE_DIR}/3rd-party)

# ==== BEGIN PDCurses ====
include(ExternalProject)
ExternalProject_Add(PDCurses 
    GIT_REPOSITORY https://github.com/wmcbrine/PDCurses.git
    GIT_TAG 3.9
    PREFIX ${3RD_PARTY}
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cd ${3RD_PARTY}/src/PDCurses/wincon && nmake -f Makefile.vc WIDE=Y UTF8=Y
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

set(CURSES_NEED_CURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
set(CURSES_LIBRARY ${3RD_PARTY}/src/PDCurses/wincon/pdcurses.lib)
set(CURSES_INCLUDE_PATH ${3RD_PARTY}/src/PDCurses)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

set(PDCURSES_ROOT ${3RD_PARTY}/src/PDCurses)
# ==== END PDCurses ====

# ==== START CUSTOM TARGETS ====
# adding a custom target for cleaning PDCurses.
add_custom_target("PDCurses_clean")
add_custom_command(TARGET "PDCurses_clean"
                    COMMENT "=== Cleaning: PDCurses ==="
                    WORKING_DIRECTORY ${CURSES_INCLUDE_DIRS}/wincon
                    COMMAND nmake -f Makefile.vc clean)

add_custom_target("zip_release")
add_custom_command(TARGET "zip_release"
                    COMMENT "=== 7zip: Package Release Artefact ==="
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    COMMAND 7z a TermTools-Release.zip $ENV{USERPROFILE}/TermTools)               
# ==== END CUSTOM TARGETS ====

# ==== START TARGETS ====

set(TARGET_LIST pager
                counter
                winfetch
                timer)

foreach(target ${TARGET_LIST})
    add_subdirectory(src/${target})
endforeach()
# ==== END TARGETS ====

# ==== START INSTALL ====
set(CMAKE_INSTALL_PREFIX "$ENV{USERPROFILE}/TermTools" CACHE PATH "..." FORCE)

install(TARGETS ${TARGET_LIST}
        CONFIGURATIONS Release
        COMPONENT runtime)

install(FILES LICENSE DESTINATION doc COMPONENT doc)
install(FILES README.md DESTINATION doc COMPONENT doc)

foreach(target ${TARGET_LIST})
    install(FILES README.md DESTINATION doc/${target} COMPONENT doc)
endforeach()
# ==== END INSTALL ====

# SET(CPACK_GENERATOR "ZIP")
# SET ( CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tools for the Windows Commandline")
# SET ( CPACK_PACKAGE_VENDOR "Holger Dörner <holger.doerner@gmail.com>")
# SET ( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
# SET ( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
# SET ( CPACK_PACKAGE_VERSION_MAJOR "0")
# SET ( CPACK_PACKAGE_VERSION_MINOR "1")
# set(CPACK_PACKAGE_VERSION_PATCH "4")
# SET ( CPACK_PACKAGE_CONTACT "Holger Dörner <holger.doerner@gmail.com>")
# SET ( CPACK_PACKAGE_SECTION "Commandline-Tools")
# set(CPACK_ZIP_COMPONENT_INSTALL ON)
# set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
# include(CPack)
# cpack_add_component(doc REQUIRED)

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