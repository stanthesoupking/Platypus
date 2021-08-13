cmake_minimum_required(VERSION 3.10.3)

project("spinning_platypus")

include_directories("src" "../../platypus/src")
file(GLOB_RECURSE EXAMPLE_SRC "src/**.c")

add_executable(spinning_platypus ${EXAMPLE_SRC})
target_link_libraries(spinning_platypus platypus)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set_property(TARGET spinning_platypus PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
if(${CMAKE_VERSION} VERSION_GREATER "3.17.0") 
	set_property(TARGET spinning_platypus PROPERTY XCODE_GENERATE_SCHEME TRUE)
	set_property(TARGET spinning_platypus PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()