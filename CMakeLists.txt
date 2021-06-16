cmake_minimum_required(VERSION 3.18)
project(canvas
    VERSION 0.1.0
    DESCRIPTION "\
Web implementation of a simple canvas, written \
using the SDL 2.0 library and Emscripten toolchain"
    HOMEPAGE_URL https://github.com/lem0nez/emcanvas
    LANGUAGES CXX)

# Require exact version of the standard library.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MIN_EMSCRIPTEN_VERSION 2.0.8)
if(EMSCRIPTEN_VERSION VERSION_LESS MIN_EMSCRIPTEN_VERSION)
  string(CONCAT MESSAGE "Version of the Emscripten toolchain must be at least "
                        "${MIN_EMSCRIPTEN_VERSION}! "
                        "Your version is ${EMSCRIPTEN_VERSION}")
  message(FATAL_ERROR "${MESSAGE}")
endif()

# + ---------- +
# | Executable |
# + ---------- +

set(SOURCES
  src/main.cpp
)

add_executable(canvas ${SOURCES})
set(CMAKE_EXECUTABLE_SUFFIX .html)
set_target_properties(canvas PROPERTIES
    LINK_DEPENDS "${CMAKE_SOURCE_DIR}/index.html"
    LINK_FLAGS "--shell-file \"${CMAKE_SOURCE_DIR}/index.html\" -s USE_SDL=2")

# + --------------- +
# | Dependent files |
# + --------------- +

set(ASSETS
  assets/favicon.png
  assets/ball.svg
)

foreach(ASSET ${ASSETS})
  # Using COPYONLY to copy a file without replacing any variable references.
  configure_file("${ASSET}" "${ASSET}" COPYONLY)
endforeach()

configure_file(styles.css styles.css COPYONLY)
configure_file(main.js main.js COPYONLY)
