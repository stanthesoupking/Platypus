cmake_minimum_required(VERSION 3.10.3)

project(platypus)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

include_directories("src")
file(GLOB_RECURSE PLT_SRC "src/platypus/platypus.c")

# On Windows you may need to set an environment variable called SDL2DIR to your SDL2 development library directory
find_package(SDL2 REQUIRED)

include_directories(platypus ${SDL2_INCLUDE_DIR})

add_library(platypus ${PLT_SRC})
target_link_libraries(platypus ${SDL2_LIBRARY} m)
