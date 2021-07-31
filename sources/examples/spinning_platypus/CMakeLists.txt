cmake_minimum_required(VERSION 3.10.3)

project("spinning_platypus")

include_directories("src" "../../platypus/src")
file(GLOB_RECURSE EXAMPLE_SRC "src/**.c")

add_executable(spinning_platypus ${EXAMPLE_SRC})
target_link_libraries(spinning_platypus platypus)