cmake_minimum_required(VERSION 3.10.3)

project("Platypus")

add_subdirectory("sources/platypus" platypus)
add_subdirectory("sources/examples/spinning_platypus" "examples/spinning_platypus")
add_subdirectory("sources/examples/shooter" "examples/shooter")