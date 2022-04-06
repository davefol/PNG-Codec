cmake_minimum_required(VERSION 3.16)
project(PNGViewer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
FILE(GLOB SRCFILES src/*.cpp)
add_executable(viewer ${SRCFILES})
target_link_libraries(viewer PRIVATE SDL2::SDL2)

find_package(Catch2 REQUIRED)
file(GLOB TESTFILES tests/*.cpp)
add_executable(tests ${TESTFILES})
target_link_libraries(tests PRIVATE Catch2::Catch2)

add_custom_command(TARGET viewer PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/examples/ $<TARGET_FILE_DIR:viewer>/examples)
