#include "graphicsview.h"

#include <QToolButton>
#include <QOpenGLFunctions>

GraphicsView::GraphicsView(GLuint &baseColorTexture, QWidget * const parent) :
QGraphicsView(parent),
drawTextureShader(),
baseColorTexture(baseColorTexture),
graphicsScene(new GraphicsScene(this)),
openGLWidget(new ViewportOpenGLWidget(drawTextureShader, baseColorTexture, QMatrix4x4(transform()), this))/*,
currentScale(1.0)*/
{
    QToolButton settingsButton(this);
    setScene(graphicsScene);

    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setViewport(openGLWidget);

    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);

    connect(this, &GraphicsView::TransformChanged, openGLWidget, &ViewportOpenGLWidget::OnTransformChanged);
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect){
    QGraphicsView::drawForeground(painter, rect);

    painter->save();

    painter->resetTransform();

    QPen pen(Qt::black);
    painter->setPen(pen);
    painter->drawText(10,10,"Test");

    painter->restore();
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect){
    painter->save();
    painter->resetTransform();
    painter->beginNativePainting();
    if(openGLWidget != nullptr){
        openGLWidget->paintGL();
    }
    painter->endNativePainting();
    painter->restore();

    QGraphicsView::drawBackground(painter, rect);
}

bool GraphicsView::event(QEvent* event){
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

        const float viewportWidth = openGLWidget->ViewWidth;
        const float viewportHeight = openGLWidget->ViewHeight;
        const float mousex = ((float)mouseEvent->pos().x()+0.5f)/viewportWidth;
        const float mousey = ((float)mouseEvent->pos().y()+0.5f)/viewportHeight;

        const QVector2D mousePosition((2.0f * mousex - 1.0f)/(512.0f*openGLWidget->CurrentScale/viewportWidth),
                                      (1.0f - 2.0f * mousey)/(512.0f*openGLWidget->CurrentScale/viewportHeight));

        emit drawChanged(mousePosition);
        return true;
    }

    return QGraphicsView::event(event);
}

bool GraphicsView::eventFilter(QObject* object, QEvent* event){
    if(object == viewport()){
        if(event->type() == QEvent::MouseMove){
            if(QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)){
                if(!(mouseEvent->modifiers() & Qt::ShiftModifier) && mouseEvent->buttons() == Qt::LeftButton){
                    QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);

                    const float viewportWidth = openGLWidget->ViewWidth;
                    const float viewportHeight = openGLWidget->ViewHeight;
                    const float mousex = ((float)mouseEvent->pos().x()+0.5f)/viewportWidth;
                    const float mousey = ((float)mouseEvent->pos().y()+0.5f)/viewportHeight;

                    const QVector2D mousePosition((2.0f * mousex - 1.0f)/(512.0f*openGLWidget->CurrentScale/viewportWidth),
                                                  (1.0f - 2.0f * mousey)/(512.0f*openGLWidget->CurrentScale/viewportHeight));

                    emit drawChanged(mousePosition);
                    return true;
                }
            }
        }
    }

    return QGraphicsView::eventFilter(object, event);
}

void GraphicsView::resizeEvent(QResizeEvent* event){
    QGraphicsView::resizeEvent(event);

    if(openGLWidget != nullptr && openGLWidget->isValid()){
        openGLWidget->resizeGL(event->size().width(), event->size().height());
    }
}

void GraphicsView::wheelEvent(QWheelEvent* event){
//    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if(event->angleDelta().y() > 0){
        if(transform().m11() > 5.1 || transform().m22() > 5.1){
        }else{
            scale(1.1, 1.1);
        }
    }else{
        if(transform().m11() < 1.1 || transform().m22() < 1.1){
        }else{
            scale(1.0/1.1, 1.0/1.1);
        }
    }

    openGLWidget->CurrentScale = transform().m11();

    printf("scale: %f %f\n", transform().m11(), transform().m22());

    emit TransformChanged(transform());

    viewport()->update();

    event->accept();
}

GraphicsView::ViewportOpenGLWidget::ViewportOpenGLWidget(Shader& drawTextureShader, const GLuint &baseColorTexture, const QMatrix4x4 transformMatrix, QWidget* parent) :
QOpenGLWidget(parent),
#if defined(__EMSCRIPTEN__)
QOpenGLFunctions(),
#else
QOpenGLFunctions_3_0(),
#endif
ViewWidth(0.0f),
ViewHeight(0.0f),
AspectRatio(1.0f),
CurrentScale(1.0f),
drawTextureShader(drawTextureShader),
baseColorTexture(baseColorTexture),
transformMatrix(transformMatrix){

}

void GraphicsView::ViewportOpenGLWidget::initializeGL(){
    printf("ViewportOpenGLWidget::initializeGL\n");
    initializeOpenGLFunctions();

    const char* vertexShaderSource = "#version 300 es\n"
                                     "precision highp float;\n"
                                     "in vec2 position;\n"
                                     "in vec2 textureCoordinates;\n"
                                     "uniform mat4 transformMatrix;\n"
                                     "uniform float aspectRatio;\n"
                                     "uniform vec2 scaleFactor;\n"
                                     "out vec2 aTextureCoordinates;\n"
                                     "void main(){\n"
                                        "aTextureCoordinates = textureCoordinates;\n"
                                        "vec4 transformedPosition = transformMatrix * vec4(position.x*scaleFactor.x, position.y*scaleFactor.y, 0.0, 1.0);\n"
                                        "gl_Position = vec4(transformedPosition.xyz, 1.0);\n"
                                     "}";

    const char* fragmentShaderSource = "#version 300 es\n"
                                       "precision highp float;\n"
                                       "in vec2 aTextureCoordinates;\n"
                                       "uniform sampler2D textureSampler;\n"
                                       "out vec4 fragColor;\n"
                                       "void main(){\n"
                                            "vec4 color = texture(textureSampler, aTextureCoordinates);\n"
                                            "fragColor = vec4(color.rgb, 1.0);\n"
                                       "}";

    drawTextureShader.InitializeGLFunctions(context());
    drawTextureShader.CreateProgram(vertexShaderSource, fragmentShaderSource);
    drawTextureShader.UseProgram();

    const float quadVertices[12] = {-1.0f,-1.0f, 1.0f,1.0f, 1.0f,-1.0f,
                                    -1.0f,-1.0f, -1.0f,1.0f, 1.0f,1.0f};
    const float quadTextureCoordinates[12] = {0.0f,0.0f, 1.0f,1.0f, 1.0f,0.0f,
                                              0.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f};

    drawTextureShader.AddAttribute(&quadVertices[0], 12, "position", 2);
    // the actual texture coordinates should be from 0,..,1 so currently they are drawn four times.
    drawTextureShader.AddAttribute(&quadTextureCoordinates[0], 12, "textureCoordinates", 2);

    drawTextureShader.AddUniform(nullptr, 1, "textureSampler", GL_FALSE);

    for(int i = 0; i < 4; i++){
        printf("transformMatrix: %f %f %f %f\n", transformMatrix.data()[i*4], transformMatrix.data()[i*4+1], transformMatrix.data()[i*4+2], transformMatrix.data()[i*4+3]);
    }
    drawTextureShader.AddUniform(transformMatrix.data(), 16, "transformMatrix", GL_FALSE);
    AspectRatio = ViewHeight/ViewWidth;
    drawTextureShader.AddUniform(&AspectRatio, 1, "aspectRatio", GL_FALSE);
    const float scaleFactor[2] = {512.0f*CurrentScale/width(), 512.0f*CurrentScale/height()};
    drawTextureShader.AddUniform(&scaleFactor[0], 2, "scaleFactor", GL_FALSE);


    resizeGL(width(), height());
}

void GraphicsView::ViewportOpenGLWidget::paintGL(){
    printf("GraphicsView::ViewportOpenGLWidget::paintGL\n");
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, ViewWidth, ViewHeight);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawTextureShader.UseProgram();
    drawTextureShader.BindVAO();

    drawTextureShader.ChangeUniform(1, transformMatrix.data(), 16, GL_FALSE);

    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GraphicsView::ViewportOpenGLWidget::resizeGL(int w, int h){
    printf("GraphicsView::ViewportOpenGLWidget::resizeGL\n");
    const float devicePixelRatio = qApp->devicePixelRatio();
    ViewWidth = (float)w * devicePixelRatio;
    ViewHeight = (float)h * devicePixelRatio;

    if(!drawTextureShader.UseProgram())
        return;

    AspectRatio = ViewHeight/ViewWidth;
    drawTextureShader.ChangeUniform(2, &AspectRatio, 1, GL_FALSE);
    const float scaleFactor[2] = {512.0f*CurrentScale/ViewWidth, 512.0f*CurrentScale/ViewHeight};
    drawTextureShader.ChangeUniform(3, &scaleFactor[0], 2, GL_FALSE);
}

void GraphicsView::ViewportOpenGLWidget::OnTransformChanged(const QTransform transform){
    transformMatrix = QMatrix4x4(transform);

    for(int i = 0; i < 4; i++){
        printf("transformMatrix: %f %f %f %f\n", transformMatrix.data()[i*4], transformMatrix.data()[i*4+1], transformMatrix.data()[i*4+2], transformMatrix.data()[i*4+3]);
    }
}
