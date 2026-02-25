#include "openglwidget.h"

#include <QLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QGesture>
#include <QMatrix4x4>

#include "errorlist.h"

OpenGLWidget::OpenGLWidget(GLuint& baseColorTexture, QWidget* const parent) :
QOpenGLWidget(parent),
#if defined(__EMSCRIPTEN__)
QOpenGLFunctions(),
#else
QOpenGLFunctions_3_0(),
#endif
settingsButton(this),
defaultShader(),
baseColorTextureSampler(),
baseColorTexture(baseColorTexture),
brushShader(),
brushColorTexture(),
mouseButtonClickPosition(),
mousePosition{0.0f,0.0f},
backFramebuffer(),
errorList(new ErrorList(this)),
mouseDown(false),
viewWidth(0.0f),
viewHeight(0.0f),
distance(3.0f),
halfFOV(45.0f * (3.141f/180.0f)),
aspectScale(1.0f),
fovScale(cosf(halfFOV)/sinf(halfFOV)),
viewPosition{0.0f,0.0f,-distance},
viewRotation{0.0f,0.0f,0.0f},
transformMatrix{1.0f,0.0f,0.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,1.0f,0.0f ,0.0f,0.0f,0.0f,1.0f},
nearClippingPlane(0.1f),
farClippingPlane(100.0f),
//perspectiveMatrix{1.0f / (aspectScale * fovScale),0.0f,0.0f,0.0f,
//                    0.0f,1.0f / fovScale,0.0f,0.0f,
//                    0.0f,0.0f,-(farClippingPlane+nearClippingPlane)/(farClippingPlane-nearClippingPlane),-1.0f,
//                    0.0f,0.0f,-(2.0f*farClippingPlane*nearClippingPlane)/(farClippingPlane-nearClippingPlane),0.0f},
perspectiveMatrix{fovScale / aspectScale,0.0f,0.0f,0.0f,
                  0.0f,fovScale,0.0f,0.0f,
                  0.0f,0.0f,(farClippingPlane+nearClippingPlane)/(nearClippingPlane-farClippingPlane),(2.0f*farClippingPlane*nearClippingPlane)/(nearClippingPlane-farClippingPlane),
                  0.0f,0.0f,-1.0f,0.0f},
//perspectiveMatrix{fovScale / aspectScale,0.0f,0.0f,0.0f,
//                    0.0f,fovScale,0.0f,0.0f,
//                    0.0f,0.0f,(farClippingPlane+nearClippingPlane)/(nearClippingPlane-farClippingPlane),-1.0f,
//                    0.0f,0.0f,(2.0f*farClippingPlane*nearClippingPlane)/(nearClippingPlane-farClippingPlane),0.0f},
triangle{-1.0f,-1.0f,0.0f, 1.0f,-1.0f,0.0f, 1.0f,1.0f,0.0f},
triangleTextureCoordinates{0.0,0.0,1.0,0.0,1.0,1.0},
cubeVertices{1.0f,-1.0f,-1.0f,
             -1.0f, -1.0, 1.0f,
             -1.0f, -1.0, -1.0f,
             -1.0f, 1.0f, -1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0, -1.0f,
             1.0f, 1.0, -1.0f,
             1.0f, -1.0, 1.0f,
             1.0f, -1.0, -1.0f,
             1.0f, 1.0f, 1.0f,
             -1.0f, -1.0, 1.0f,
             1.0f, -1.0, 1.0f,
             -1.0f, -1.0, 1.0f,
             -1.0f, 1.0f, -1.0f,
             -1.0f, -1.0, -1.0f,
             1.0f, -1.0, -1.0f,
             -1.0f, 1.0f, -1.0f,
             1.0f, 1.0, -1.0f,
             1.0f, -1.0, -1.0f,
             1.0f, -1.0, 1.0f,
             -1.0f, -1.0, 1.0f,
             -1.0f, 1.0f, -1.0f,
             -1.0f, 1.0f, 1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, 1.0, -1.0f,
             1.0f, 1.0f, 1.0f,
             1.0f, -1.0, 1.0f,
             1.0f, 1.0f, 1.0f,
             -1.0f, 1.0f, 1.0f,
             -1.0f, -1.0, 1.0f,
             -1.0f, -1.0, 1.0f,
             -1.0f, 1.0f, 1.0f,
             -1.0f, 1.0f, -1.0f,
             1.0f, -1.0, -1.0f,
             -1.0f, -1.0, -1.0f,
             -1.0f, 1.0f, -1.0f},
cubeTextureCoordinates{0.0001f, 0.3334f,
                       0.3334f, 0.0001f,
                       0.3334f, 0.3334f,
                       0.6666f, 0.3334f,
                       0.9999f, 0.0001f,
                       0.9999f, 0.3334f,
                       0.6666f, 0.6669f,
                       0.3334f, 0.3336f,
                       0.6666f, 0.3336f,
                       0.3334f, 0.6669f,
                       0.0001f, 0.3336f,
                       0.3334f, 0.3336f,
                       0.3334f, 0.0001f,
                       0.6666f, 0.3334f,
                       0.3334f, 0.3334f,
                       0.6666f, 0.3336f,
                       0.9999f, 0.6669f,
                       0.6666f, 0.6669f,
                       0.0001f, 0.3334f,
                       0.0001f, 0.0001f,
                       0.3334f, 0.0001f,
                       0.6666f, 0.3334f,
                       0.6666f, 0.0001f,
                       0.9999f, 0.0001f,
                       0.6666f, 0.6669f,
                       0.3334f, 0.6669f,
                       0.3334f, 0.3336f,
                       0.3334f, 0.6669f,
                       0.0001f, 0.6669f,
                       0.0001f, 0.3336f,
                       0.3334f, 0.0001f,
                       0.6666f, 0.0001f,
                       0.6666f, 0.3334f,
                       0.6666f, 0.3336f,
                       0.9999f, 0.3336f,
                       0.9999f, 0.6669f},
hitPoint(0.0f,0.0f,0.0f)
{
    setAttribute(Qt::WA_AlwaysStackOnTop, false);
    setAttribute(Qt::WA_AcceptTouchEvents, true);

    grabGesture(Qt::PanGesture);

    setAcceptDrops(true);

    if(layout() != nullptr)
    {
        layout()->addWidget(errorList);
    }

    printf("mousePosition: %f %f\n", mousePosition[0], mousePosition[1]);

    transformMatrix[0] = cosf(-viewRotation[1]);
    transformMatrix[1] = 0.0f;
    transformMatrix[2] = sinf(-viewRotation[1]);
    transformMatrix[3] = viewPosition[0]*cosf(-viewRotation[1])+viewPosition[2]*sinf(-viewRotation[1]);
    transformMatrix[4] = 0.0f;
    transformMatrix[5] = 1.0f;
    transformMatrix[6] = 0.0f;
    transformMatrix[7] = viewPosition[1];
    transformMatrix[8] = -sinf(-viewRotation[1]);
    transformMatrix[9] = 0.0f;
    transformMatrix[10] = cosf(-viewRotation[1]);
    transformMatrix[11] = viewPosition[0]*-sinf(-viewRotation[1])+viewPosition[2]*cosf(-viewRotation[1]);
    transformMatrix[12] = 0.0f;
    transformMatrix[13] = 0.0f;
    transformMatrix[14] = 0.0f;
    transformMatrix[15] = 1.0f;

    errorList->AddError("Error: 1", ErrorList::MessageType::Error);
    errorList->AddError("Warning: 2", ErrorList::MessageType::Warning);
    errorList->AddError("Message: 3", ErrorList::MessageType::None);

    printf("openGLWidget constructed\n");
}

OpenGLWidget::~OpenGLWidget(){
    delete errorList;
}

void OpenGLWidget::OnDrawChanged(const QVector2D mousePosition){
    printf("OpenGLWidget::OnDrawChanged\n");
    hitPoint.setX(mousePosition.x());
    hitPoint.setY(mousePosition.y());

    mouseDown = true;
//    makeCurrent();
//    paintGL();
    update();
    emit DrawChanged();
}

void OpenGLWidget::OnBrushChanged(const QImage& brushTexture){
    brushShader.UseProgram();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, brushColorTexture);
    const int brushTextureSize = 128;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, brushTextureSize, brushTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, brushTexture.bits());

    brushShader.ChangeUniform(1, nullptr, 1, GL_FALSE);
}

void OpenGLWidget::initializeGL(){
    initializeOpenGLFunctions();

    printf("initializeGL\n");

    const char* vertexShaderSource = "#version 300 es\n"
                                     "precision highp float;\n"
                                     "in vec3 iPosition;\n"
                                     "in vec2 iTextureCoordinates;\n"
                                     "uniform mat4 viewMatrix;\n"
                                     "uniform mat4 perspectiveMatrix;\n"
                                     "out vec2 oTextureCoordinates;\n"
                                     "void main(){\n"
                                            "oTextureCoordinates = iTextureCoordinates;\n"
                                            "vec4 position = viewMatrix * vec4(iPosition, 1.0);\n"
                                            "gl_Position = position;\n"
                                     "}";

    const char* fragmentShaderSource = "#version 300 es\n"
                                       "precision highp float;\n"
                                       "in vec2 oTextureCoordinates;\n"
                                       "uniform sampler2D baseColorTexture;\n"
                                       "out vec4 fragColor;\n"
                                       "void main(){\n"
                                            "vec4 color = texture(baseColorTexture, oTextureCoordinates);\n"
                                            "fragColor = color;\n"
                                       "}";

    defaultShader.InitializeGLFunctions(context());
    defaultShader.CreateProgram(vertexShaderSource, fragmentShaderSource);
    defaultShader.UseProgram();
//    defaultShader.AddAttribute(&triangle[0], 9, "iPosition", 3);
    defaultShader.AddAttribute(&cubeVertices[0], 108, "iPosition", 3);

//    defaultShader.AddAttribute(&triangleTextureCoordinates[0], 6, "iTextureCoordinates", 2);
    defaultShader.AddAttribute(&cubeTextureCoordinates[0], 72, "iTextureCoordinates", 2);

    glGenTextures(1, &baseColorTexture);
    printf("openglwidget baseColorTexture: %i\n", baseColorTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // this is for testing this currently leaks memory
    unsigned char* data = new unsigned char[4*512*512*sizeof(unsigned char)];
    for(int counter = 0; counter < 4*512*512; counter+=4){
        data[counter] = (unsigned char)0.0f;
        data[counter+1] = (unsigned char)0.0f;
        data[counter+2] = (unsigned char)255.0f;
        data[counter+3] = (unsigned char)255.0f;
    }

    const int textureSize = 512;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

    defaultShader.AddUniform(nullptr, 1, "baseColorTexture", GL_FALSE);

    QMatrix4x4 perspectiveMat(&perspectiveMatrix[0]);
    QMatrix4x4 viewMat(&transformMatrix[0]);
    QMatrix4x4 transformMat = perspectiveMat*viewMat;

    defaultShader.AddUniform(&perspectiveMatrix[0], 16, "perspectiveMatrix", GL_FALSE);
    defaultShader.AddUniform(transformMat.data(), 16, "viewMatrix", GL_FALSE);

    const char* brushVertexShaderSource = "#version 300 es\n"
                                            "precision highp float;\n"
                                            "in vec2 position;\n"
                                            "in vec2 textureCoordinates;\n"
                                            "uniform vec2 mouseClickPosition;\n"
                                            "out vec4 colorHelper;\n"
                                            "out vec2 aTextureCoordinates;\n"
                                            "void main(){\n"
                                                "aTextureCoordinates = textureCoordinates;\n"
                                                "vec2 brushPosition = position*0.01 + mouseClickPosition;\n"
                                                "colorHelper = vec4(brushPosition, 0.0, 1.0);\n"
                                                "gl_Position = vec4(brushPosition, 0.0, 1.0);\n"
                                            "}";

    const char* brushFragmentShaderSource = "#version 300 es\n"
                                              "precision highp float;\n"
                                              "in vec4 colorHelper;"
                                              "uniform sampler2D brushColorTexture;\n"
                                              "in vec2 aTextureCoordinates;\n"
                                              "layout (location = 0)out vec4 fragColor;\n"
                                              "void main(){\n"
                                                  "vec4 color = texture(brushColorTexture, aTextureCoordinates);\n"
                                                  "fragColor = color;\n;"//vec4(colorHelper);\n"
                                              "}";

    const float brushQuadVertices[12] = {-1.0f,-1.0f, 1.0f,1.0f, 1.0f,-1.0f,
                                   -1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f};

    const float brushTextureCoordinates[12] = {0.0f,0.0f, 1.0f,1.0f, 1.0f,0.0f,
                                         0.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f};

    brushShader.InitializeGLFunctions(context());
    brushShader.CreateProgram(brushVertexShaderSource, brushFragmentShaderSource);
    brushShader.UseProgram();
    brushShader.AddAttribute(&brushQuadVertices[0], 12, "position", 2);
    brushShader.AddAttribute(&brushTextureCoordinates[0], 12, "textureCoordinates", 2);

    const float mousePosition[2] = {0.0f,0.0f};

    brushShader.AddUniform(&mousePosition[0], 2, "mouseClickPosition", GL_FALSE);

    glGenFramebuffers(1, &backFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, backFramebuffer);

    // this is for testing this currently leaks memory
    unsigned char* data2 = new unsigned char[4*512*512*sizeof(unsigned char)];
    for(int counter = 0; counter < 4*512*512; counter+=4){
        data2[counter] = (unsigned char)0.0f;
        data2[counter+1] = (unsigned char)255.0f;
        data2[counter+2] = (unsigned char)255.0f;
        data2[counter+3] = (unsigned char)255.0f;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data2[0]);

    glGenTextures(1, &brushColorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, brushColorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const int brushTextureSize = 128;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, brushTextureSize, brushTextureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

    brushShader.AddUniform(nullptr, 1, "brushColorTexture", GL_FALSE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, baseColorTexture, 0);
    const GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, &drawBuffers[0]);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        errorList->AddError("Framebuffer incomplete", ErrorList::MessageType::Error);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
}

void OpenGLWidget::paintGL(){
    printf("paintGL: %s\n", mouseDown ? "true" : "false");

    if(mouseDown){
        glViewport(0, 0, 512, 512);
        glBindFramebuffer(GL_FRAMEBUFFER, backFramebuffer);
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

        brushShader.UseProgram();
        brushShader.BindVAO();
        const float hitPointTemp[2] = {hitPoint.x(), hitPoint.y()};
        printf("OpenGLWidget::paintGL hitPointTemp: %f %f\n", hitPointTemp[0], hitPointTemp[1]);
        brushShader.ChangeUniform(0, &hitPointTemp[0], 2, GL_FALSE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brushColorTexture);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        const GLuint glError = glGetError();
        printf("glGetError: %i\n", glError);

        glDisable(GL_BLEND);
        glBindVertexArray(0);
        mouseDown = false;
    }

    glViewport(0, 0, viewWidth, viewHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    defaultShader.UseProgram();
    glClearColor(0.0f,1.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    defaultShader.BindVAO();

    QMatrix4x4 perspectiveMat(&perspectiveMatrix[0]);
    QMatrix4x4 viewMat(&transformMatrix[0]);
    QMatrix4x4 transformMat = perspectiveMat*viewMat;

    defaultShader.ChangeUniform(1, &perspectiveMatrix[0], 16, GL_FALSE);
    defaultShader.ChangeUniform(2, transformMat.data(), 16, GL_FALSE);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

bool OpenGLWidget::event(QEvent* event){
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);
        mousePosition[0] = 2.0f * (float)mouseEvent->pos().x()/viewWidth - 1.0f;
        mousePosition[1] = 1.0f - 2.0f * (float)mouseEvent->pos().y()/viewHeight;
//        printf("mousePosition: %f %f\n", mousePosition[0], mousePosition[1]);

        if(!(mouseEvent->modifiers() & Qt::ShiftModifier) && mouseEvent->buttons() == Qt::LeftButton){
//            printf("ViewportSize: %i %i\n", width, height);
//            printf("MouseButtonPress: %i %i\n", mouseEvent->pos().x(), mouseEvent->pos().y());

            mouseDown = true;
            float distance = 99999999.9f;
//            raycast(&mousePosition[0], &triangle[0], &triangleTextureCoordinates[0], hitPoint);
            for(int i = 0; i < 12; i++){
                const float triangleVertices[9] = {cubeVertices[i*9],cubeVertices[i*9+1],cubeVertices[i*9+2],
                                                   cubeVertices[i*9+3],cubeVertices[i*9+4],cubeVertices[i*9+5],
                                                   cubeVertices[i*9+6],cubeVertices[i*9+7],cubeVertices[i*9+8]};
                const float triangleTextureCoordinates[6] = {cubeTextureCoordinates[i*6],cubeTextureCoordinates[i*6+1],
                                                             cubeTextureCoordinates[i*6+2],cubeTextureCoordinates[i*6+3],
                                                             cubeTextureCoordinates[i*6+4],cubeTextureCoordinates[i*6+5]};
                // this needs quadtree optimization
                raycast(&mousePosition[0], &triangleVertices[0],&triangleTextureCoordinates[0], hitPoint, distance);
            }
            update();
            emit DrawChanged();
        }else if(mouseEvent->buttons() == Qt::MiddleButton){
            printf("MiddleButton\n");
        }

        return true;
    }else if(event->type() == QEvent::MouseMove){
        if(QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)){
            if(mouseEvent->buttons() == Qt::MiddleButton ||
                       (mouseEvent->modifiers() & Qt::ShiftModifier && mouseEvent->buttons() == Qt::LeftButton)){
//                printf("move MiddleButton\n");
                const float mousex = 2.0f * (float)mouseEvent->pos().x()/viewWidth - 1.0f;
                const float mousey = 1.0f - 2.0f * (float)mouseEvent->pos().y()/viewHeight;
//                printf("mousex: %f mousey: %f\n", mousex, mousey);
                const float moveDirection[2] = {mousex - mousePosition[0], mousey - mousePosition[1]};
                viewRotation[0] -= moveDirection[1];
                viewRotation[1] -= moveDirection[0];
                if(viewRotation[0] > 1.55334f){
                    viewRotation[0] = 1.55334f;
                }else if(viewRotation[0] < -1.55334f){
                    viewRotation[0] = -1.55334f;
                }
                if(viewRotation[1] > 6.28318530718f){
                    viewRotation[1] = viewRotation[1] - 6.28318530718f;
                }else if(viewRotation[1] < 6.28318530718f){
                    viewRotation[1] = viewRotation[1] + 6.28318530718f;
                }
                printf("viewRotation: %f %f\n", viewRotation[0], viewRotation[1]);

                viewPosition[0] = 0.0f + distance * sinf(viewRotation[1]) * cosf(viewRotation[0]);
                viewPosition[1] = 0.0f + distance * sinf(viewRotation[0]);
                viewPosition[2] = 0.0f + distance * cosf(viewRotation[1]) * cosf(viewRotation[0]);

                QVector3D viewPos(viewPosition[0], viewPosition[1], viewPosition[2]);
                QVector3D forwardDirection(-viewPosition[0], -viewPosition[1], -viewPosition[2]);
                forwardDirection.normalize();
                QVector3D worldUp(0.0f,1.0f,0.0f);
                QVector3D rightDirection(QVector3D::crossProduct(forwardDirection, worldUp));
                rightDirection.normalize();
                QVector3D upDirection(QVector3D::crossProduct(rightDirection, forwardDirection));

                printf("viewPosition: %f %f %f\n", viewPosition[0], viewPosition[1], viewPosition[2]);

                transformMatrix[0] = rightDirection.x();
                transformMatrix[1] = rightDirection.y();
                transformMatrix[2] = rightDirection.z();
                transformMatrix[3] = -QVector3D::dotProduct(rightDirection, viewPos);
                transformMatrix[4] = upDirection.x();
                transformMatrix[5] = upDirection.y();
                transformMatrix[6] = upDirection.z();
                transformMatrix[7] = -QVector3D::dotProduct(upDirection, viewPos);
                transformMatrix[8] = -forwardDirection.x();
                transformMatrix[9] = -forwardDirection.y();
                transformMatrix[10] = -forwardDirection.z();
                transformMatrix[11] = QVector3D::dotProduct(forwardDirection, viewPos);
                transformMatrix[12] = 0.0f;
                transformMatrix[13] = 0.0f;
                transformMatrix[14] = 0.0f;
                transformMatrix[15] = 1.0f;

//                printf("moveDirection: %f %f - %f\n", moveDirection[0], mousex, mousePosition[0]);
            }else if(!(mouseEvent->modifiers() & Qt::ShiftModifier) && mouseEvent->buttons() == Qt::LeftButton){
                mouseDown = true;
                float distance = 999999999.9f;
//                raycast(&mousePosition[0], &triangle[0], &triangleTextureCoordinates[0], hitPoint);
                for(int i = 0; i < 12; i++){
                    const float triangleVertices[9] = {cubeVertices[i*9],cubeVertices[i*9+1],cubeVertices[i*9+2],
                                                       cubeVertices[i*9+3],cubeVertices[i*9+4],cubeVertices[i*9+5],
                                                       cubeVertices[i*9+6],cubeVertices[i*9+7],cubeVertices[i*9+8]};
                    const float triangleTextureCoordinates[6] = {cubeTextureCoordinates[i*6],cubeTextureCoordinates[i*6+1],
                                                                 cubeTextureCoordinates[i*6+2],cubeTextureCoordinates[i*6+3],
                                                                 cubeTextureCoordinates[i*6+4],cubeTextureCoordinates[i*6+5]};
                    // this needs quadtree optimization
                    raycast(&mousePosition[0], &triangleVertices[0],&triangleTextureCoordinates[0], hitPoint, distance);
                }
                update();
                emit DrawChanged();
            }

            mousePosition[0] = 2.0f * (float)mouseEvent->pos().x()/viewWidth - 1.0f;
            mousePosition[1] = 1.0f - 2.0f * (float)mouseEvent->pos().y()/viewHeight;
            update();

            return true;
        }
    }else if(event->type() == QEvent::MouseButtonRelease){
        mouseDown = false;
        return true;
    }else if(event->type() == QEvent::Gesture){
        if(QGestureEvent* const gestureEvent = static_cast<QGestureEvent*>(event)){
            if(QPanGesture* const pan = static_cast<QPanGesture*>(gestureEvent->gesture(Qt::PanGesture))){
                viewRotation[0] += pan->delta().y();
                viewRotation[1] += pan->delta().x();
                update();
                return true;
            }else if(QPinchGesture* const pinch = static_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture))){
                distance = pinch->totalScaleFactor();

                viewPosition[0] = 0.0f + distance * sinf(viewRotation[1]) * cosf(viewRotation[0]);
                viewPosition[1] = 0.0f + distance * sinf(viewRotation[0]);
                viewPosition[2] = 0.0f + distance * cosf(viewRotation[1]) * cosf(viewRotation[0]);

                QVector3D viewPos(viewPosition[0], viewPosition[1], viewPosition[2]);
                QVector3D forwardDirection(-viewPosition[0], -viewPosition[1], -viewPosition[2]);
                forwardDirection.normalize();
                QVector3D worldUp(0.0f,1.0f,0.0f);
                QVector3D rightDirection(QVector3D::crossProduct(forwardDirection, worldUp));
                rightDirection.normalize();
                QVector3D upDirection(QVector3D::crossProduct(rightDirection, forwardDirection));

                printf("viewPosition: %f %f %f\n", viewPosition[0], viewPosition[1], viewPosition[2]);

                transformMatrix[0] = rightDirection.x();
                transformMatrix[1] = upDirection.x();
                transformMatrix[2] = -forwardDirection.x();
                transformMatrix[3] = -QVector3D::dotProduct(rightDirection, viewPos);
                transformMatrix[4] = rightDirection.y();
                transformMatrix[5] = upDirection.y();
                transformMatrix[6] = -forwardDirection.y();
                transformMatrix[7] = -QVector3D::dotProduct(upDirection, viewPos);
                transformMatrix[8] = rightDirection.z();
                transformMatrix[9] = upDirection.z();
                transformMatrix[10] = -forwardDirection.z();
                transformMatrix[11] = QVector3D::dotProduct(forwardDirection, viewPos);
                transformMatrix[12] = 0.0f;
                transformMatrix[13] = 0.0f;
                transformMatrix[14] = 0.0f;
                transformMatrix[15] = 1.0f;

                printf("pinch distance: %f\n", distance);
                update();
            }
        }
    }

    return QOpenGLWidget::event(event);
}

void OpenGLWidget::resizeGL(int w, int h){
    const float devicePixelRatio = qApp->devicePixelRatio();
    viewWidth = w * devicePixelRatio;
    viewHeight = h * devicePixelRatio;
    glViewport(0, 0, viewWidth, viewHeight);

    aspectScale = viewWidth/viewHeight;

    perspectiveMatrix[0] = fovScale / aspectScale;

    defaultShader.UseProgram();
//    defaultShader.ChangeUniform(1, &perspectiveMatrix[0], 16, GL_FALSE);
    QMatrix4x4 perspectiveMat(&perspectiveMatrix[0]);
    QMatrix4x4 viewMat(&transformMatrix[0]);
    QMatrix4x4 transformMat = perspectiveMat*viewMat;
    defaultShader.ChangeUniform(2, transformMat.data(), 16, GL_FALSE);
    printf("resizeGL\n");

//    for(int i = 0; i < 4; i++){
//        printf("transformMatrix: %f %f %f %f\n", transformMatrix[i*4], transformMatrix[i*4+1], transformMatrix[i*4+2], transformMatrix[i*4+3]);
//    }

//    for(int i = 0; i < 4; i++){
//        printf("perspectiveMatrix: %f %f %f %f\n", perspectiveMatrix[i*4], perspectiveMatrix[i*4+1], perspectiveMatrix[i*4+2], perspectiveMatrix[i*4+3]);
//    }

//    QMatrix4x4 viewMat(&transformMatrix[0]);
//    QMatrix4x4 perspectiveMat(&perspectiveMatrix[0]);
//    QMatrix4x4 transformMat = perspectiveMat * viewMat;

//    for(int i = 0; i < 3; i++){
//        QVector4D newPoint = transformMat.map(QVector4D(triangle[i*3],triangle[i*3+1],triangle[i*3+2],1.0f));
//        printf("newPoint: %f %f %f\n", newPoint.x(), newPoint.y(), newPoint.z());
//    }
}

void OpenGLWidget::dragEnterEvent(QDragEnterEvent* event){
    printf("drag enter event\n");

    event->acceptProposedAction();
}

void OpenGLWidget::dropEvent(QDropEvent* event){
    printf("drop Event\n");

    event->acceptProposedAction();
}

void OpenGLWidget::wheelEvent(QWheelEvent* event){
    if(event->angleDelta().y() > 0){
        if(distance < 0.1f)
            return;

        distance -= 0.5f;
    }else{
        if(distance > 10.0f)
            return;

        distance += 0.5f;
    }

    viewPosition[0] = 0.0f + distance * sinf(viewRotation[1]) * cosf(viewRotation[0]);
    viewPosition[1] = 0.0f + distance * sinf(viewRotation[0]);
    viewPosition[2] = 0.0f + distance * cosf(viewRotation[1]) * cosf(viewRotation[0]);

    QVector3D viewPos(viewPosition[0], viewPosition[1], viewPosition[2]);
    QVector3D forwardDirection(-viewPosition[0], -viewPosition[1], -viewPosition[2]);
    forwardDirection.normalize();
    QVector3D worldUp(0.0f,1.0f,0.0f);
    QVector3D rightDirection(QVector3D::crossProduct(forwardDirection, worldUp));
    rightDirection.normalize();
    QVector3D upDirection(QVector3D::crossProduct(rightDirection, forwardDirection));

    printf("viewPosition: %f %f %f\n", viewPosition[0], viewPosition[1], viewPosition[2]);

    transformMatrix[0] = rightDirection.x();
    transformMatrix[1] = upDirection.x();
    transformMatrix[2] = -forwardDirection.x();
    transformMatrix[3] = -QVector3D::dotProduct(rightDirection, viewPos);
    transformMatrix[4] = rightDirection.y();
    transformMatrix[5] = upDirection.y();
    transformMatrix[6] = -forwardDirection.y();
    transformMatrix[7] = -QVector3D::dotProduct(upDirection, viewPos);
    transformMatrix[8] = rightDirection.z();
    transformMatrix[9] = upDirection.z();
    transformMatrix[10] = -forwardDirection.z();
    transformMatrix[11] = QVector3D::dotProduct(forwardDirection, viewPos);
    transformMatrix[12] = 0.0f;
    transformMatrix[13] = 0.0f;
    transformMatrix[14] = 0.0f;
    transformMatrix[15] = 1.0f;

    printf("distance: %f\n", distance);
    update();
    event->accept();
}

bool OpenGLWidget::raycast(float* mousePosition, const float* triangle, const float* triangleTextureCoordinates, QVector3D &outHitPoint, float& distance){
//    printf("OpenGLWidget::raycast\n");

    const QMatrix4x4 perspectiveMat = QMatrix4x4(&perspectiveMatrix[0]);
//    const QMatrix4x4 perspectiveMat = QMatrix4x4(&perspectiveMatrix[0]);
//    const QMatrix4x4 viewMatrix = QMatrix4x4(&transformMatrix[0]);
    const QMatrix4x4 viewMat = QMatrix4x4(&transformMatrix[0]);
//    const QMatrix4x4 translateMat(&translate[0]);
//    const QMatrix4x4 rotateyMat(&rotationy[0]);
//    const QMatrix4x4 viewMatrix = translateMat*rotateyMat;
    const QMatrix4x4 invMatrix = QMatrix4x4(perspectiveMat*viewMat).inverted();
    const QVector4D mouseClickPosition(mousePosition[0], mousePosition[1], -1.0f, 1.0f);
    const QVector4D unprojectedMousePositionStartClip = invMatrix.map(mouseClickPosition);
//    printf("unprojectedMosuePositionStartClip: %f %f %f %f\n", mouseClickPosition.x(), mouseClickPosition.y(), mouseClickPosition.z(), mouseClickPosition.w());
//    printf("unprojectedMousePositionStartClip: %f %f %f %f\n", unprojectedMousePositionStartClip.x(), unprojectedMousePositionStartClip.y(), unprojectedMousePositionStartClip.z(), unprojectedMousePositionStartClip.w());
    const QVector3D unprojectedMousePositionStartView(unprojectedMousePositionStartClip.x()/unprojectedMousePositionStartClip.w(),
                                                        unprojectedMousePositionStartClip.y()/unprojectedMousePositionStartClip.w(),
                                                        unprojectedMousePositionStartClip.z()/unprojectedMousePositionStartClip.w());
//    printf("unprojectedMousePositionStart: %f %f %f\n", unprojectedMousePositionStartView.x(), unprojectedMousePositionStartView.y(), unprojectedMousePositionStartView.z());
    const QVector4D unprojectedMousePositionEndClip = invMatrix.map(QVector4D(mousePosition[0], mousePosition[1], 1.0f, 1.0f));
    const QVector3D unprojectedMousePositionEndView(unprojectedMousePositionEndClip.x()/unprojectedMousePositionEndClip.w(),
                                                      unprojectedMousePositionEndClip.y()/unprojectedMousePositionEndClip.w(),
                                                      unprojectedMousePositionEndClip.z()/unprojectedMousePositionEndClip.w());
//    printf("unprojectedMousePositionEnd: %f %f %f\n", unprojectedMousePositionEndView.x(), unprojectedMousePositionEndView.y(), unprojectedMousePositionEndView.z());

    const QVector3D origin = unprojectedMousePositionStartView;
//    printf("origin: %f %f %f\n", origin.x(), origin.y(), origin.z());
    QVector3D direction(unprojectedMousePositionEndView.x()-unprojectedMousePositionStartView.x(),
                              unprojectedMousePositionEndView.y()-unprojectedMousePositionStartView.y(),
                              unprojectedMousePositionEndView.z()-unprojectedMousePositionStartView.z());
    direction = direction.normalized();
//    printf("direction: %f %f %f\n", direction.x(), direction.y(), direction.z());

    const QVector3D edge1(triangle[3]-triangle[0], triangle[4]-triangle[1], triangle[5]-triangle[2]);
    const QVector3D edge2(triangle[6]-triangle[0], triangle[7]-triangle[1], triangle[8]-triangle[2]);

    const QVector3D P(QVector3D::crossProduct(direction, edge2));
    const float det(QVector3D::dotProduct(edge1, P));

    if(det == 0.0f){
        printf("det == 0.0f\n");
        return false;
    }

    const float invDet = 1.0f / det;

    const QVector3D T(origin.x() - triangle[0], origin.y() - triangle[1], origin.z() - triangle[2]);
    const float u = QVector3D::dotProduct(T, P) * invDet;
    if(u < 0.0f || u > 1.0f){
        printf("u outside\n");
        return false;
    }

    const QVector3D Q = QVector3D::crossProduct(T, edge1);
    const float v = QVector3D::dotProduct(direction, Q) * invDet;
    if(v < 0.0f || u + v > 1.0f){
        printf("v and u+v outside\n");
        return false;
    }

    const float t = QVector3D::dotProduct(edge2, Q) * invDet;
    if(t < 0.0f){
        printf("t behind\n");
        return false;
    }

    // hit point in world position
    const QVector3D worldHitPoint(origin[0] + t * direction[0],
                         origin[1] + t * direction[1],
                         origin[2] + t * direction[2]);
    printf("worldHitPoint: %f %f %f\n", worldHitPoint.x(), worldHitPoint.y(), worldHitPoint.z());

    QVector3D originToWorldHitPoint(worldHitPoint.x()-origin.x(),
                                   worldHitPoint.y()-origin.y(),
                                   worldHitPoint.z()-origin.z());

    const float newDistance = sqrt(originToWorldHitPoint.x()*originToWorldHitPoint.x()+
                                   originToWorldHitPoint.y()*originToWorldHitPoint.y()+
                                   originToWorldHitPoint.z()*originToWorldHitPoint.z());

    if(newDistance > distance){
        return false;
    }

    distance = newDistance;

    // hit point in uv space
    outHitPoint = QVector3D(2.0f * ((1-u-v)*triangleTextureCoordinates[0]+triangleTextureCoordinates[2]*u+triangleTextureCoordinates[4]*v) - 1.0f,
                           2.0f * ((1-u-v)*triangleTextureCoordinates[1]+triangleTextureCoordinates[3]*u+triangleTextureCoordinates[5]*v) - 1.0f, 0.0f);

//    printf("outHitPoint: %f %f %f\n", outHitPoint.x(), outHitPoint.y(), outHitPoint.z());

    return true;
}
