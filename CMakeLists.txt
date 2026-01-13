cmake_minimum_required(VERSION 3.22)
project(ModelPainter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_RQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Opengl OpenGLWidgets)

qt_standard_project_setup()
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

set(SOURCES
        main.cpp
	dockwidget.cpp
        dockwidget.h
	graphicsscene.cpp
        graphicsscene.h
	graphicsview.cpp
        graphicsview.h
	openglwidget.cpp
        openglwidget.h
        errorlist.cpp
	errorlist.h
        shader.cpp
	shader.h
        brushwidget.cpp
        brushwidget.h)

qt_add_executable(ModelPainter ${SOURCES})

target_link_libraries(ModelPainter PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::OpenGL Qt6::OpenGLWidgets)
