cmake_minimum_required(VERSION 3.0.0)
project(TermTools VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)

string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})
string(REPLACE "/MDd" "/MTd" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /nologo /W3 /EHsc")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /nologo /W3 /EHsc")

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("CMAKE_C_FLAGS_DEBUG: ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_C_FLAGS_RELEASE: ${CMAKE_C_FLAGS_RELEASE}")
message("\n")

include(CTest)
enable_testing()

set(3RD_PARTY ${PROJECT_SOURCE_DIR}/3rd-party)

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

# adding a custom target for cleaning PDCurses.
add_custom_target("PDCurses_clean")
add_custom_command(TARGET "PDCurses_clean"
                    COMMENT "=== Cleaning: PDCurses ==="
                    WORKING_DIRECTORY ${PDCURSES_ROOT}/wincon
                    COMMAND cd ${3RD_PARTY}/PDCurses/wincon && nmake -f Makefile.vc clean)

# targets
add_subdirectory(pager)

install(TARGETS pager
        CONFIGURATIONS Release)

install(CODE "execute_process(COMMAND ${PROJECT_SOURCE_DIR}/etc/set_env.bat)"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)

#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "c:/users/$ENV{USERNAME}/TermTools" CACHE PATH "..." FORCE)
#endif()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
