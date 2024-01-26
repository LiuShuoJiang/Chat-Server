# add all source files
aux_source_directory(. SRC_LIST)
aux_source_directory(./database DB_LIST)
aux_source_directory(./model MODEL_LIST)

# generate executable
add_executable(Chat_Server ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})

# delegate target libraries
target_link_libraries(Chat_Server muduo_net muduo_base mysqlclient pthread)
