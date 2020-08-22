################################################
# Project Settings
################################################
project(bk-erasure-fixtures CXX)
set(CMAKE_CXX_STANDARD 17)

################################################
# Files
################################################
set(H include/bk/erasure/fixtures)

set(LIB_FILES 
    ${H}/erasure_test_policies.hpp
)

################################################
# Targets
################################################
add_library(bk_erasure_fixtures
    ${LIB_FILES}
)

target_include_directories(bk_erasure_fixtures INTERFACE
    include
)

target_link_libraries(bk_erasure_fixtures
    PUBLIC bk_erasure
)

set_target_properties(bk_erasure_fixtures PROPERTIES
    LINKER_LANGUAGE CXX
)

add_library(bk::erasure::fixtures ALIAS bk_erasure_fixtures)
