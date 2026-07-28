# 3D ModelPainter

A lightweight cross-platform 3D model painting application built with **Qt 6.7.3**, supporting direct texture painting on 3D meshes and 2D layouts.

## Features

* **Dual Viewport Editing**: Paint seamlessly in both the 3D viewport and 2D texture space.
* **Hybrid Rendering Pipeline**: Optimized performance via deferred rendering (currently only on desktop, webassembly uses forward rendering).
* **Dynamic Brush Customization**:
  * Load custom images as brushes.
  * Adjust tint color via multiplication.
  * Scale and blur brushes dynamically.
* **Texture Export**: Save and export your final painted textures.
  
## Supported Platforms

* **Windows**: Fully tested and supported.
* **WebAssembly**: Fully tested and supported via Qt for WebAssembly. [Test the browser version here!](https://jan214.github.io/TestQtProject/TestQtProject.html)
* **Linux & macOS**: Compiles via Qt, but currently untested.

* ## Prerequisites

* **Qt 6.7.3 SDK** (Windows target)
* CMake 3.22 or higher

* ## Building the Project

### Windows Desktop (CMake)
Run the following commands from your terminal to build and package the desktop application:
```cmd
cmake -S .. -B build -DCMAKE_PREFIX_PATH="C:\Qt\Qt-6.7.3"
cmake --build build --config Release
cmake --install build --config Release --prefix "%cd%\dist"
```
