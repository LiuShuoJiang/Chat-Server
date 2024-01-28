cmake_minimum_required(VERSION 3.22)
project(Chat_Project)

set(CMAKE_CXX_STANDARD 17)

# config the output path for executable
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# config header file search paths
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/database)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)

include_directories(${PROJECT_SOURCE_DIR}/third_party)

# load sub-directory
add_subdirectory(src)
