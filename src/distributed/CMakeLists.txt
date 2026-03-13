add_subdirectory(messages)
add_subdirectory(logic)
add_subdirectory(sockets)
add_subdirectory(utils)
add_subdirectory(dynamic)

add_library(distributed INTERFACE)

target_link_libraries(distributed INTERFACE distributed_logic
        distributed_messages
        distributed_sockets
        distributed_utils
        distributed_dynamic
)