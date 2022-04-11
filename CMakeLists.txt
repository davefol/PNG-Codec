cmake_minimum_required(VERSION 3.16)
project(PNGViewer)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)
set(
    SRCFILES
    src/IDAT.cpp
    src/IHDR.cpp
    src/PLTE.cpp
    src/ImagePartial.cpp
    src/ImageVector.cpp
    src/UnknownChunk.cpp
    src/bitstream.cpp
    src/consume.cpp
    src/deflateutils.cpp
    src/dumputils.cpp
    src/huffmanutils.cpp
    src/inflate.cpp
    src/zlibutils.cpp
)
set (
    HEADERFILES
    include/Chunk.h
    include/IDAT.h
    include/IHDR.h
    include/PLTE.h
    include/ImagePartial.h
    include/ImageVector.h
    include/UnknownChunk.h
    include/bitstream.h
    include/consume.h
    include/deflateutils.h
    include/dumputils.h
    include/huffmanutils.h
    include/inflate.h
    include/zlibutils.h
)
include_directories(include)
add_executable(viewer src/viewer.cpp ${SRCFILES} ${HEADERFILES})
target_link_libraries(viewer PRIVATE SDL2::SDL2)

find_package(Catch2 REQUIRED)
set(
    TESTFILES
    tests/main.cpp
    tests/test_deflateutils.cpp
)
add_executable(tests ${TESTFILES} ${HEADERFILES} ${SRCFILES})
target_link_libraries(tests PRIVATE Catch2::Catch2)
#target_link_libraries(tests PRIVATE SDL2::SDL2)

# copy examples
add_custom_command(TARGET viewer PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/examples/ $<TARGET_FILE_DIR:viewer>/examples)
# copy test data
add_custom_command(TARGET viewer PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/test_data/ $<TARGET_FILE_DIR:viewer>/test_data)
