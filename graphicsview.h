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

    void OnDrawChanged();

signals:
    void drawChanged(const QVector2D mousePosition);
    void TransformChanged(const QTransform transform);

protected:
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

        void OnTransformChanged(const QTransform transform);

        float ViewWidth;
        float ViewHeight;
        float AspectRatio;
        float CurrentScale;
    protected:
        Shader& drawTextureShader;
        const GLuint& baseColorTexture;

        QMatrix4x4 transformMatrix;
    };

    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual bool event(QEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    Shader drawTextureShader;
    const GLuint& baseColorTexture;

    GraphicsScene* graphicsScene;
    ViewportOpenGLWidget* openGLWidget;

    qreal currentScale;

    class ModelLoader& modelLoader;
};

#endif // GRAPHICSVIEW_H
