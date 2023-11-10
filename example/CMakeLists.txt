cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(example)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

# Dependencies
find_package(raylib 4.5.0 QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/4.5.0.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

set(
NEEDED_LIBRARIES
    dimensions
    font
    l2
    push-pop
    texture
    window
)

foreach(Lib IN ITEMS ${NEEDED_LIBRARIES})
    set(LibH "../library/${Lib}.h")
    set(LibCc "../library/${Lib}.cc")
    list(APPEND NEEDED_LIBRARIES_H ${LibH})
    list(APPEND NEEDED_LIBRARIES_CC ${LibCc}) 
endforeach()

file(GLOB SOURCES_H *.h ${NEEDED_LIBRARIES_H} ../core/*.h)
file(GLOB SOURCES *.cc ${NEEDED_LIBRARIES_CC} ../core/*.cc ${SOURCES_H})

set(CombineAllTestsPy "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/combine_all_tests.py")
set(TestH "${CMAKE_CURRENT_SOURCE_DIR}/test.h")
add_custom_command(
    COMMAND ../scripts/combine_all_tests.py ${SOURCES_H}
    DEPENDS ${CombineAllTestsPy} ${SOURCES}
    OUTPUT ${TestH}
    COMMENT "Generating code for test.h"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${TestH})
#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)
target_link_libraries(${PROJECT_NAME} m)

# Web Configurations
if (${PLATFORM} STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
