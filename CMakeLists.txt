cmake_minimum_required(VERSION 3.16)

project("Software Renderer" C CXX)  # C is for the generated glad loader.

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

foreach(build_type DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${build_type} ${CMAKE_SOURCE_DIR}/examples/bin)
endforeach()

# Setup compiler flags.
set(CMAKE_CXX_STANDARD 14)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i.86")
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
  elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX")
  endif()
endif()

include(FetchContent)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
FetchContent_Declare(
  glfw
  URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
  URL_HASH SHA256=a133ddc3d3c66143eba9035621db8e0bcf34dba1ee9514a9e23e96afd39fd57a
)
FetchContent_Declare(
  glad
  URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip
  URL_HASH SHA256=471886d363ea2b7e2f92081266c4775de91ee971a10359bd33559f1c5a1cbb92
  SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glfw glad)
glad_add_library(glad REPRODUCIBLE API gl:core=4.1)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(RENDERER_SOURCES
  src/renderer/matrix.cc
  src/renderer/pipeline.cc
)
set(APP_SOURCES
  src/app/app.cc
  src/app/fps_counter.cc
  src/app/obj_parser.cc
  src/app/tga_loader.cc
)
set(EXAMPLES_SOURCES
  examples/src/clipping.cc
  examples/src/culling.cc
  examples/src/mrt.cc
  examples/src/texturing.cc
  examples/src/zbuffer.cc
)

add_library(renderer STATIC ${RENDERER_SOURCES})
add_library(app STATIC ${APP_SOURCES})
target_link_libraries(app renderer glad glfw)

foreach(EXAMPLES_SOURCE ${EXAMPLES_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLES_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLES_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} app)
  set_target_properties(${EXAMPLE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
endforeach(EXAMPLES_SOURCE)
