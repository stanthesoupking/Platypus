cmake_minimum_required(VERSION 3.10.3)

project("platypus")

include_directories("src")
file(GLOB_RECURSE PLT_SRC "src/platypus.c")

include_directories("../../vendor/glfw/include")
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
add_subdirectory("../../vendor/glfw" glfw)

add_library(platypus ${PLT_SRC})
target_link_libraries(platypus glfw)