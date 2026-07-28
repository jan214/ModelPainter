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

    // called when mouse is pressed or held down while in the viewport
    void OnDrawChanged(const QPointF mousePosition);
    // called when brush properties are changed in the BrushWidget
    void OnBrushChanged(const QImage& brushTexture);

signals:
    void DrawChanged();

protected:
    // currently unused, planned to be used to split a model into objects to be able to move them
    //struct Mesh{
    //    std::vector<QVector3D> vertices;
    //    std::vector<QVector2D> textureCoordinates;
    //    std::vector<QVector3D> normals;
    //};

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    // used to handle viewport interactions like drawing and camera movement
    virtual bool event(QEvent* event) override;
    // drag events will be used to navigate the viewport on the phone in webassembly
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    // drop event to handle loading a new model
    virtual void dropEvent(QDropEvent* event) override;

    // handles zooming in the viewport
    void wheelEvent(QWheelEvent* event) override;

    // raycast to axis aligned bounding boxes detection
    bool raycast(float mousePosition[2], const float* const mins, const float* const maxs, float& distance);
    // raycast to triangle detection
    bool raycast(float mousePosition[2], const float triangle[9], const float triangleTextureCoordinates[6], QVector3D& outHitPoint, float& distance);

    Shader defaultShader;                       // the default forward rendering shader
    Shader deferredObjectShader;                // the object phase shader for deferred rendering
    Shader deferredPostProcessingShader;        // the lighting phase shader for deferred rendering
    GLuint deferredCustomFramebuffer;           // the custom framebuffer that deferred rendering uses to render the gbuffers
    std::vector<GLuint> sharedRenderTextures;   // render textures that are shared between shaders
    GLuint& baseColorTexture;                   // the texture that is being drawn
    Shader brushShader;                         // the shader for the brush
    GLuint brushColorTexture;                   // the texture for the brush
    GLuint backFramebuffer;                     // the back buffer that holds the brushColorTexture

    float mousePosition[2];                     // the mouse click position for raycasting

    bool mouseDown;                             // holds if the mouse is currently down in the viewport
    float viewWidth;                            // holds the viewport width
    float viewHeight;                           // holds the viewport height

    float distance;                             // the distance to check which triangle is the closest to the viewport

    float halfFOV;                              // half of the field of view
    float aspectScale;                          // the scale for unstretching the x coordinate
    float fovScale;

    float viewPosition[3];                      // the view position in global space
    float viewRotation[3];                      // the views rotation

    float transformMatrix[16];                  // currently just the view matrix

    float nearClippingPlane;
    float farClippingPlane;
    float perspectiveMatrix[16];

    // Resources for 3d models of a triangle and a cube
    const float triangle[9];
    const float triangleTextureCoordinates[6];

    const float cubeVertices[108];
    const float cubeTextureCoordinates[72];

    class ModelLoader& modelLoader; // object that holds and loads models

    bool modelChanged;  // set true when the model was changed

    QVector3D hitPoint; // the hit point of a raycast triangle collision in global space
};

#endif // OPENGLWIDGET_H
