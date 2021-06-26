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
  src/algorithms.cpp
  src/app.cpp
  src/brush.cpp
  src/main.cpp
)

add_executable(canvas ${SOURCES})
set(CMAKE_EXECUTABLE_SUFFIX .html)

set(COMPILE_FLAGS
    "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]'")
string(CONCAT LINK_FLAGS
    "--shell-file \"${CMAKE_SOURCE_DIR}/index.html\" "
    "--preload-file \"${CMAKE_SOURCE_DIR}/assets/app@/\" "
    # Multiply the total available memory by 3 to make
    # possible creation of SDL renderer on 4K displays.
    "-s TOTAL_MEMORY=48MB ${COMPILE_FLAGS}")

set_target_properties(canvas PROPERTIES
    COMPILE_FLAGS "${COMPILE_FLAGS}"
    LINK_DEPENDS "${CMAKE_SOURCE_DIR}/index.html;${CMAKE_SOURCE_DIR}/assets/app"
    LINK_FLAGS "${LINK_FLAGS}")

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
