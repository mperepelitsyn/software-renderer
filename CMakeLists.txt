cmake_minimum_required(VERSION 3.4)
project("Software Renderer")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -march=native -mtune=native")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/bin)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB RENDERER_SOURCES "src/renderer/*.cc")
file(GLOB APP_SOURCES "src/app/*.cc")
file(GLOB EXAMPLES_SOURCES "examples/src/*.cc")

add_library(renderer ${RENDERER_SOURCES})
add_library(app ${APP_SOURCES})

foreach(EXAMPLES_SOURCE ${EXAMPLES_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLES_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLES_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} renderer app ${GLFW_LIBRARIES} GL GLEW)
endforeach(EXAMPLES_SOURCE)
