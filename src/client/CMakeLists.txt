aux_source_directory(. SRC_LIST)

add_executable(Chat_Client ${SRC_LIST})

target_link_libraries(Chat_Client pthread)
