QT += core gui widgets opengl openglwidgets

CONFIG += c++11

#wasm: {
#    QMAKE_LFLAGS += -sUSE_WEBGL2=1
#    QMAKE_CXXFLAGS += -sUSE_WEBGL2=1
#}

SOURCES += \
    brushwidget.cpp \
    dockwidget.cpp \
    errorlist.cpp \
    graphicsscene.cpp \
    graphicsview.cpp \
    main.cpp \
    openglwidget.cpp \
    shader.cpp \
    contactdialog.cpp \
    modelloader.cpp

HEADERS += \
    brushwidget.h \
    dockwidget.h \
    errorlist.h \
    graphicsscene.h \
    graphicsview.h \
    openglwidget.h \
    shader.h \
    contactdialog.h \
    modelloader.h
