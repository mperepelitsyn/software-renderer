cmake_minimum_required(VERSION 3.4)
project("Rasterizer")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_COMPILER clang++)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB SOURCES "src/*.cc")
add_executable(rasterizer ${SOURCES})
target_link_libraries(rasterizer ${GLFW_LIBRARIES} GL GLEW)
