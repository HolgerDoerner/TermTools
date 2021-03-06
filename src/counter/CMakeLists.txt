cmake_minimum_required(VERSION 3.0)

project(counter VERSION 0.1.0 LANGUAGES C)

configure_file(${PROJECT_SOURCE_DIR}/counter_version.h.in
                ${PROJECT_SOURCE_DIR}/counter_version.h
                WIN32)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

add_executable(${PROJECT_NAME} counter.c)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME ${PROJECT_NAME})

# CTest cases
add_test(NAME counter_linecount_37
        WORKING_DIRECTORY ${TEST_FILES_DIR}
        COMMAND ${PROJECT_NAME} test.txt)
set_tests_properties(counter_linecount_37 PROPERTIES 
        PASS_REGULAR_EXPRESSION "37")

add_test(NAME counter_wordcount_395
        WORKING_DIRECTORY ${TEST_FILES_DIR}
        COMMAND ${PROJECT_NAME} /w test.txt)
set_tests_properties(counter_wordcount_395 PROPERTIES 
        PASS_REGULAR_EXPRESSION "395")