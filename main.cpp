#include <QMainWindow>
#include <QApplication>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QScreen>
#include <QDebug>
#include <QObject>
#include <QSlider>
#include <QMenuBar>
#include <QFileDialog>
#include <QTextStream>
#include <QBuffer>

#include "openglwidget.h"
#include "dockwidget.h"
#include "graphicsview.h"
#include "shader.h"
#include "brushwidget.h"
#include "contactdialog.h"
#include "modelloader.h"

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);

#ifndef __EMSCRIPTEN__
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
#endif

    QSurfaceFormat defaultFormat;
    defaultFormat.setVersion(3,0);
    defaultFormat.setSamples(4);
    defaultFormat.setProfile(QSurfaceFormat::CompatibilityProfile);
#if defined(__EMSCRIPTEN__)
    defaultFormat.setRenderableType(QSurfaceFormat::OpenGLES);
#else
    defaultFormat.setRenderableType(QSurfaceFormat::OpenGL);
#endif
    defaultFormat.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    defaultFormat.setOption(QSurfaceFormat::DeprecatedFunctions);
    defaultFormat.setDepthBufferSize(24);
    defaultFormat.setStencilBufferSize(8);

    QSurfaceFormat::setDefaultFormat(defaultFormat);

    QApplication application(argc, argv);

    QScreen* const screen = QGuiApplication::primaryScreen();

    QMainWindow window;
    const float relationWindowSize = 0.7f;
    window.resize(screen->size().width() * relationWindowSize, screen->size().height() * relationWindowSize);

    QFile qssFile(":/styles/style.qss");
    if (qssFile.open(QFile::ReadOnly | QFile::Text)) {
        printf("qssFile open\n");
        QString qssFileText = QString::fromUtf8(qssFile.readAll());
        application.setStyleSheet(qssFileText);
        qssFile.close();
    }

    // this line needs to be moved once this architecture is better
    GLuint baseColorTexture;

    OpenGLWidget openGLWidget(baseColorTexture, &window);

    DockWidget brushDockWidget(&window);

    brushDockWidget.setWindowTitle("Brush Configuration");
    BrushWidget brushWidget(&brushDockWidget);

    QObject::connect(&brushWidget, &BrushWidget::BrushChanged, &openGLWidget, &OpenGLWidget::OnBrushChanged, Qt::QueuedConnection);
    brushWidget.InitializeBrushImage();

    brushDockWidget.setWidget(&brushWidget);
    window.addDockWidget(Qt::RightDockWidgetArea, &brushDockWidget);

    DockWidget graphicsViewDockWidget(&window);
    graphicsViewDockWidget.setWindowTitle("Graphics View");
    QWidget graphicsViewWrapperWidget(&graphicsViewDockWidget);
    QVBoxLayout graphicsViewWrapperLayout(&graphicsViewWrapperWidget);
    GraphicsView graphicsView(baseColorTexture, &graphicsViewWrapperWidget);
    graphicsViewWrapperLayout.addWidget(&graphicsView);

    QObject::connect(&openGLWidget, &OpenGLWidget::DrawChanged, graphicsView.viewport(), qOverload<>(&QWidget::update));
    QObject::connect(&graphicsView, &GraphicsView::drawChanged, &openGLWidget, &OpenGLWidget::OnDrawChanged, Qt::QueuedConnection);

    graphicsViewDockWidget.setWidget(&graphicsViewWrapperWidget);
    window.addDockWidget(Qt::RightDockWidgetArea, &graphicsViewDockWidget);

    // main window menu bar actions should be moved into custom QMainWindow class when created
    QMenu* fileMenu = window.menuBar()->addMenu("File");
    QAction* const loadModelAction = fileMenu->addAction("Load Model...");
    QObject::connect(loadModelAction, &QAction::triggered, [&window]() {
        auto fileContentReady = [](const QString& fileName, const QByteArray& fileContent) {
            if (!fileName.isEmpty()) {
                QString objText = QString::fromUtf8(fileContent);
                QTextStream objTextStream(&objText);
                ModelLoader::GetInstance().LoadModel(objTextStream);
            }
            };

        QFileDialog::getOpenFileContent("Model Files(*.obj *.fbx);; All Files(*)", fileContentReady, &window);
    });

    QAction* const exportTextureAction = fileMenu->addAction("Export Texture...");
    QObject::connect(exportTextureAction, &QAction::triggered, [&graphicsView]() {
        QByteArray textureByteArray = graphicsView.GetBaseColorTexture();
        QByteArray newByteArray;
        QImage saveImage(reinterpret_cast<const uchar*>(textureByteArray.constData()), 512, 512, QImage::Format_RGBA8888);
        QBuffer buffer(&newByteArray);
        buffer.open(QIODevice::WriteOnly);
        if (!saveImage.save(&buffer, "PNG")) {
            printf("image could not be saved in format\n");
        }

        QFileDialog::saveFileContent(newByteArray, "texture");
    });

    QMenu* helpMenu = window.menuBar()->addMenu("Help");
    QAction* const testAction2 = helpMenu->addAction("Contact");
    QObject::connect(testAction2, &QAction::triggered, []() { ContactDialog::GetInstance().show(); });

    window.setCentralWidget(&openGLWidget);
    window.show();

    return application.exec();
}
