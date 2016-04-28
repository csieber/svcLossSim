cmake_minimum_required (VERSION 2.6)
project (svcLossSim)

file(GLOB_RECURSE sources src/*.cpp src/*.c)

find_package( Boost REQUIRED COMPONENTS system program_options thread )
include_directories( ${Boost_INCLUDE_DIRS} )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(svclosssim ${sources})

target_link_libraries(svclosssim ${Boost_LIBRARIES})

