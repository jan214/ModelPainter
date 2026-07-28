#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

#include "graphicsscene.h"

#include <QOpenGLWidget>
#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif
#include <QWheelEvent>
#include <QMatrix4x4>

#include "openglwidget.h"
#include "shader.h"

class GraphicsView : public QGraphicsView{
    Q_OBJECT
public:
    explicit GraphicsView(GLuint& baseColorTexture, QWidget* const parent = nullptr);
    ~GraphicsView(){}

    // returns the texture from the ViewportOpenGLWidget that is being drawn on as a QByteArray
    QByteArray GetBaseColorTexture();

signals:
    void drawChanged(const QPointF mousePosition);
    void TransformChanged(const QMatrix4x4 transform);

protected:
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    // handles mouse button press in the viewport
    virtual bool event(QEvent* event) override;
    // handles mouse move while mouse button pressed in the viewport
    bool eventFilter(QObject* object, QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    // handles zooming in the viewport
    virtual void wheelEvent(QWheelEvent* event) override;
    // handles open the export texture menu
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

    // ViewportOpenGLWidget class is used for rendering the opengl texture in the QGraphicsView viewport
#if defined(__EMSCRIPTEN__)
    class ViewportOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions{
#else
    class ViewportOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_0{
#endif

    public:
        explicit ViewportOpenGLWidget(Shader& drawTextureShader, const GLuint& baseColorTexture, const QMatrix4x4 transformMtrix, QWidget* parent = nullptr);
        ~ViewportOpenGLWidget(){}

        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        // called when Qts GraphicsView transform changes and updates the opengl viewport transform
        inline void OnTransformChanged(const QMatrix4x4 transform);

        // returns the texture that is being drawn on from opengl as QByteArray
        QByteArray GetBaseColorTexture();

        float ViewWidth;    // the viewports width
        float ViewHeight;   // the viewports height
        float AspectRatio;  // the viewport aspect ratio
    protected:
        // the shader that draws the texture that is being drawn on
        Shader& drawTextureShader;
        // the texture that is being drawn on
        const GLuint& baseColorTexture;

        // the transform matrix to transform the texture correctly in the viewport
        QMatrix4x4 transformMatrix;
    };

    void viewMoved(int value);

    Shader drawTextureShader;
    const GLuint& baseColorTexture;

    // the graphics scene of the graphics view
    GraphicsScene* graphicsScene;
    // the opengl viewport of this graphicsview
    ViewportOpenGLWidget* openGLWidget;

    // the model loader that holds all information about the 3d model, here for the uv coordinates
    class ModelLoader& modelLoader;
};

#endif // GRAPHICSVIEW_H
