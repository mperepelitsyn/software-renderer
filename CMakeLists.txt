cmake_minimum_required(VERSION 3.16)

project("Software Renderer" C CXX)  # C is for SDL.

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

foreach(build_type DEBUG RELEASE MINSIZEREL RELWITHDEBINFO)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${build_type} ${CMAKE_SOURCE_DIR}/examples/bin)
endforeach()

# Setup compiler flags.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
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
set(SDL_TEST_LIBRARY OFF)
FetchContent_Declare(
  sdl3
  URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.12.zip
  URL_HASH SHA256=ce4e4b92e628b376b59091fcaa7358044f4c7009b6d35be0d81abe8e1de7847a
)
FetchContent_MakeAvailable(sdl3)

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
target_link_libraries(app renderer SDL3::SDL3)

foreach(EXAMPLES_SOURCE ${EXAMPLES_SOURCES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLES_SOURCE} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLES_SOURCE})
  target_link_libraries(${EXAMPLE_NAME} app)
  target_compile_definitions(${EXAMPLE_NAME} PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/examples/assets")
  set_target_properties(${EXAMPLE_NAME} PROPERTIES DEBUG_POSTFIX "_d")
endforeach(EXAMPLES_SOURCE)
