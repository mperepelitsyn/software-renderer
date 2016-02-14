cmake_minimum_required(VERSION 3.0)

option(IS_SUPERBUILD "Superbuild" ON)

if (IS_SUPERBUILD)
  project(SUPERBUILD NONE)
  include(ExternalProject)
  set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR})

  ExternalProject_Add(
    glfw3
    URL https://github.com/glfw/glfw/archive/f7228bb56294714e8cdb8b39be3b7c6967240c54.zip
    CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=off
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      -DGLFW_BUILD_DOCS=off
      -DGLFW_BUILD_TESTS=off
  )

  ExternalProject_Add(
    glew
    URL https://sourceforge.net/projects/glew/files/glew/1.13.0/glew-1.13.0.tgz
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
      -DBUILD_UTILS=off
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      <SOURCE_DIR>/build/cmake
  )

  ExternalProject_Add(
    software-renderer
    DEPENDS glew glfw3
    SOURCE_DIR ${PROJECT_SOURCE_DIR}
    CMAKE_ARGS
      -DIS_SUPERBUILD=off
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND ""
  )

  return()
endif()

project("Software Renderer" CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/bin)

# Setup compiler flags.
set(CMAKE_CXX_STANDARD 14)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -mavx")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(glfw3 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)

file(GLOB RENDERER_SOURCES "src/renderer/*.cc")
file(GLOB APP_SOURCES "src/app/*.cc")
file(GLOB EXAMPLES_SOURCES "examples/src/*.cc")

add_library(renderer STATIC ${RENDERER_SOURCES})
add_library(app STATIC ${APP_SOURCES})
target_link_libraries(app renderer glfw GLEW::glew_s)

foreach(EXAMPLES_SOURCE ${EXAMPLES_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLES_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLES_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} app)
endforeach(EXAMPLES_SOURCE)
