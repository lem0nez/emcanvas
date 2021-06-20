# Canvas
A simple web application, written in modern C++17. You can use the left mouse
button, finger (on touchscreens) or any part of your body to draw. Just amazing!

## Build
You need the CMake version at least **3.18** and Emscripten toolchain version at
least **2.0.8**.
```
cmake -E make_directory build
cd build
emcmake cmake ..
cmake --build .
```
