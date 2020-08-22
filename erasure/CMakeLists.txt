################################################
# Project Settings
################################################
project(bk-erasure CXX)
set(CMAKE_CXX_STANDARD 17)

################################################
# Dependencies
################################################
find_package(Boost REQUIRED)

################################################
# Files
################################################
set(H include/bk/erasure)

set(LIB_FILES 
    ${H}/bits/detectors.hpp
    ${H}/bits/generate_pointer_table.hpp
    ${H}/bits/generate_switch.hpp
    ${H}/bits/pointer_table_holder.hpp
    ${H}/bits/signature.hpp
    ${H}/bits/special_members.hpp
    ${H}/bits/swallow.hpp
    ${H}/bits/switch_function_pointer_holder.hpp
    ${H}/bits/traits.hpp
    ${H}/policies/binding_policy.hpp
    ${H}/policies/call_policy.hpp
    ${H}/policies/callable_checking_policy.hpp
    ${H}/policies/copy_policy.hpp
    ${H}/policies/destructor_policy.hpp
    ${H}/policies/enums.hpp
    ${H}/policies/move_policy.hpp
    ${H}/policies/null_policies.hpp
    ${H}/policies/policies_descriptor.hpp
    ${H}/policies/type_checking_policy.hpp
    ${H}/policies/type_id_policy.hpp
    ${H}/static_any.hpp
    ${H}/static_function.hpp
)

################################################
# Targets
################################################
add_library(bk_erasure 
    ${LIB_FILES}
)

target_include_directories(bk_erasure INTERFACE
    include
)

target_link_libraries(bk_erasure
    PUBLIC Boost::boost
)

set_target_properties(bk_erasure PROPERTIES
    LINKER_LANGUAGE CXX
)

add_library(bk::erasure ALIAS bk_erasure)
