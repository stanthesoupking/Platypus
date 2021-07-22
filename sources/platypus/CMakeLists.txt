cmake_minimum_required(VERSION 3.10.3)

project(platypus)

include_directories("src")
file(GLOB_RECURSE PLT_SRC "src/platypus.c")

find_package(SDL2 REQUIRED)
include_directories(platypus ${SDL2_INCLUDE_DIRS})

add_library(platypus ${PLT_SRC})
target_link_libraries(platypus ${SDL2_LIBRARIES})