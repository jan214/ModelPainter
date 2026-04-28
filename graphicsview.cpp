#include "graphicsview.h"

#include <QToolButton>
#include <QOpenGLFunctions>
#include <QPainterPath>

#include "modelloader.h"

GraphicsView::GraphicsView(GLuint &baseColorTexture, QWidget * const parent) :
QGraphicsView(parent),
drawTextureShader(),
baseColorTexture(baseColorTexture),
graphicsScene(new GraphicsScene(this)),
openGLWidget(new ViewportOpenGLWidget(drawTextureShader, baseColorTexture, this)),
currentScale(1.0),
modelLoader(ModelLoader::GetInstance())
{
    QToolButton settingsButton(this);
    setScene(graphicsScene);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setViewport(openGLWidget);
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect){
    QGraphicsView::drawForeground(painter, rect);

    painter->save();

    painter->resetTransform();

    QPen pen(Qt::black);
    painter->setPen(pen);
    painter->drawText(10,10,"Test");

    pen.setWidth(0.1f);

    if (openGLWidget != nullptr) {
        const float width = openGLWidget->ViewWidth;
        const float height = openGLWidget->ViewHeight;
        if (modelLoader.GetTextureCoordinatesSize() > 0) {
            QPainterPath path;
            for (int counter = 0; counter < modelLoader.GetTextureCoordinatesSize(); counter+=2) {
                if (counter == 0) {
                    const QPointF point(modelLoader.GetTextureCoordinates()[counter] * width, height - modelLoader.GetTextureCoordinates()[counter + 1] * height);
                    path.moveTo(point);
                    continue;
                }
                if (counter % 6 == 0) {
                    const QPointF oldPoint(modelLoader.GetTextureCoordinates()[counter - 6] * width, height - modelLoader.GetTextureCoordinates()[counter - 6 + 1] * height);
                    path.lineTo(oldPoint);
                    const QPointF point(modelLoader.GetTextureCoordinates()[counter] * width, height - modelLoader.GetTextureCoordinates()[counter + 1] * height);
                    path.moveTo(point);
                    continue;
                }
                const QPointF point(modelLoader.GetTextureCoordinates()[counter] * width, height - modelLoader.GetTextureCoordinates()[counter + 1] * height);
                path.lineTo(point);
            }

            painter->drawPath(path);
        }
    }

    painter->restore();
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect){
    painter->beginNativePainting();
    if(openGLWidget != nullptr){
        openGLWidget->paintGL();
    }
    painter->endNativePainting();

    QGraphicsView::drawBackground(painter, rect);
}

bool GraphicsView::event(QEvent* event){
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);
//        const float mousePosition[2] = {2.0f * (float)mouseEvent->pos().x()/openGLWidget->ViewWidth - 1.0f,
//                                        1.0f - 2.0f * (float)mouseEvent->pos().y()/openGLWidget->ViewHeight};

        const QVector2D mousePosition(2.0f * (float)mouseEvent->pos().x()/openGLWidget->ViewWidth - 1.0f,
                                      1.0f - 2.0f * (float)mouseEvent->pos().y()/openGLWidget->ViewHeight);

        printf("GraphicsView::event mousePosition: %f %f\n", mousePosition[0], mousePosition[1]);
        emit drawChanged(mousePosition);
        return true;
    }

    return QGraphicsView::event(event);
}

void GraphicsView::wheelEvent(QWheelEvent *event){
    const QPointF scenePosition = mapToScene(event->globalPosition().toPoint());
//    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    if(event->angleDelta().y() > 0){
        if(transform().m11() > 2.5 || transform().m22() > 2.5){
        }else{
            scale(1.1, 1.1);
        }
    }else{
        if(transform().m11() < 1.5 || transform().m22() < 1.5){
        }else{
            scale(1.0/1.1, 1.0/1.1);
        }
    }

    printf("scale: %f %f\n", transform().m11(), transform().m22());

//    setTransformationAnchor(QGraphicsView::NoAnchor);
    centerOn(scenePosition);

    event->accept();
}

GraphicsView::ViewportOpenGLWidget::ViewportOpenGLWidget(Shader& drawTextureShader, const GLuint &baseColorTexture, QWidget* parent) :
QOpenGLWidget(parent),
#if defined(__EMSCRIPTEN__)
QOpenGLFunctions(),
#else
QOpenGLFunctions_3_0(),
#endif
ViewWidth(0.0f),
ViewHeight(0.0f),
drawTextureShader(drawTextureShader),
baseColorTexture(baseColorTexture){

}

void GraphicsView::ViewportOpenGLWidget::initializeGL(){
    printf("ViewportOpenGLWidget::initializeGL\n");
    initializeOpenGLFunctions();

    const char* vertexShaderSource = "#version 300 es\n"
                                     "precision highp float;\n"
                                     "in vec2 position;\n"
                                     "in vec2 textureCoordinates;\n"
                                     "out vec2 aTextureCoordinates;\n"
                                     "void main(){\n"
                                        "aTextureCoordinates = textureCoordinates;\n"
                                        "gl_Position = vec4(position, 0.0, 1.0);\n"
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

    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void GraphicsView::ViewportOpenGLWidget::resizeGL(int w, int h){
    const float devicePixelRatio = qApp->devicePixelRatio();
    ViewWidth = (float)w * devicePixelRatio;
    ViewHeight = (float)h * devicePixelRatio;
}
