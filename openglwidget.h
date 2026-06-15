#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif
#include <QPushButton>
#include <QVector2D>
#include <QVector3D>

#include <vector>

#include "shader.h"

#if defined(__EMSCRIPTEN__)
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions{
#else
class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_0{
#endif
    Q_OBJECT
public:
    explicit OpenGLWidget(GLuint& baseColorTexture, QWidget* const parent = nullptr);
    ~OpenGLWidget();

    void OnDrawChanged(const QVector2D mousePosition);
    void OnBrushChanged(const QImage& brushTexture);

signals:
    void DrawChanged();

protected:
    struct Mesh{
        std::vector<QVector3D> vertices;
        std::vector<QVector2D> textureCoordinates;
        std::vector<QVector3D> normals;
    };

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    virtual bool event(QEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;

    bool raycast(float mousePosition[2], const float* const mins, const float* const maxs, float& distance);
    bool raycast(float mousePosition[2], const float triangle[9], const float triangleTextureCoordinates[6], QVector3D& outHitPoint, float& distance);

#if defined(__EMSCRIPTEN__)
    QPushButton settingsButton;
#endif

    Shader defaultShader;
    GLuint baseColorTextureSampler;
    GLuint& baseColorTexture;
    Shader brushShader;
    GLuint brushColorTexture;
    GLuint mouseButtonClickPosition;

    float mousePosition[2];

    GLuint backFramebuffer;

    class ErrorList* errorList;

    bool mouseDown;
    float viewWidth;
    float viewHeight;

    float distance;

    float halfFOV;
    float aspectScale;
    float fovScale;

    float viewPosition[3];
    float viewRotation[3];

    float transformMatrix[16];

    float nearClippingPlane;
    float farClippingPlane;
    float perspectiveMatrix[16];

    const float triangle[9];
    const float triangleTextureCoordinates[6];

    const float cubeVertices[108];
    const float cubeTextureCoordinates[72];

    class ModelLoader& modelLoader;

    bool modelChanged;
    //std::vector<float> customModelVertices;
    int modelSize;
    //std::vector<float> customModelTextureCoordinates;
    //std::vector<float> customModelNormals;

    QVector3D hitPoint;
};

#endif // OPENGLWIDGET_H
