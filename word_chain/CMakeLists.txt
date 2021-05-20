cmake_minimum_required(VERSION 3.10)

set(TARGET word-chain)

# set the project name
project(${TARGET})

file(GLOB_RECURSE SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
     ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cc
)

# add the executable
add_executable(${TARGET} ${SOURCES})

target_include_directories(${TARGET}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
