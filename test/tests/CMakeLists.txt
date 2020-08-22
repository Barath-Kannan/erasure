################################################
# Project Settings
################################################
project(bk-erasure-tests CXX)
set(CMAKE_CXX_STANDARD 17)

################################################
# Dependencies
################################################
find_package(Boost COMPONENTS unit_test_framework)

################################################
# Files
################################################
set(S any_tests)
set(ANY_TEST_FILES 
    ${S}/basic_is_empty_test.cpp
    ${S}/construct_and_get_test.cpp
    ${S}/copy_test.cpp
    ${S}/copy_test_non_trivial.cpp
    ${S}/destructor_test.cpp
    ${S}/move_test.cpp
    ${S}/move_test_non_trivial.cpp

    ${S}/main.cpp
)

set(S function_tests)
set(FUNCTION_TEST_FILES
    ${S}/basic_is_empty_test.cpp
    ${S}/construct_and_call_non_stateful_test.cpp
    ${S}/construct_and_call_stateful_test.cpp
    ${S}/copy_test.cpp
    ${S}/copy_test_non_trivial.cpp
    ${S}/destructor_test.cpp
    ${S}/move_test.cpp
    ${S}/move_test_non_trivial.cpp

    ${S}/main.cpp
)

################################################
# Targets
################################################
add_executable(bk_erasure_any_test
    ${ANY_TEST_FILES}
)

target_link_libraries(bk_erasure_any_test
    PUBLIC bk_erasure_fixtures
    PUBLIC Boost::unit_test_framework
)

add_executable(bk_erasure_function_test
    ${FUNCTION_TEST_FILES}
)

target_link_libraries(bk_erasure_function_test
    PUBLIC bk_erasure_fixtures
    PUBLIC Boost::unit_test_framework
)
