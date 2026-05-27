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

protected:
#if defined(__EMSCRIPTEN__)
    class ViewportOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions{
#else
    class ViewportOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_0{
#endif

    public:
        explicit ViewportOpenGLWidget(Shader& drawTextureShader, const GLuint& baseColorTexture, QWidget* parent = nullptr);
        ~ViewportOpenGLWidget(){}

        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int w, int h) override;

        float ViewWidth;
        float ViewHeight;
    protected:
        Shader& drawTextureShader;
        const GLuint& baseColorTexture;
    };

    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual bool event(QEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;

    Shader drawTextureShader;
    const GLuint& baseColorTexture;

    GraphicsScene* graphicsScene;
    ViewportOpenGLWidget* openGLWidget;

    qreal currentScale;

    class ModelLoader& modelLoader;
};

#endif // GRAPHICSVIEW_H
