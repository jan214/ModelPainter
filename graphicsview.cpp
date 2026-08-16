#include "graphicsview.h"

#include <QToolButton>
#include <QOpenGLFunctions>
#include <QPainterPath>
#include <QMenu>
#include <QFileDialog>
#include <QBuffer>

#include <QLayout>
#include <QScrollBar>

#include "modelloader.h"

GraphicsView::GraphicsView(GLuint &baseColorTexture, QWidget * const parent) :
QGraphicsView(parent),
drawTextureShader(),
baseColorTexture(baseColorTexture),
graphicsScene(new GraphicsScene(this)),
openGLWidget(new ViewportOpenGLWidget(drawTextureShader, baseColorTexture, QMatrix4x4(transform()), this)),
modelLoader(ModelLoader::GetInstance()),
cubeTextureCoordinates{ 0.0001f, 0.3334f,
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
                       0.9999f, 0.6669f } {
    setObjectName("GraphicsView");

    // for some reason removing the scrollbars messed up the drawing when zooming in
    setViewportMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setParent(this);
    horizontalScrollBar()->setParent(this);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::viewMoved);
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &GraphicsView::viewMoved);

    setScene(graphicsScene);
    setViewport(openGLWidget);

    centerOn(mapFromScene(0.0, 0.0));
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);

    connect(this, &GraphicsView::TransformChanged, openGLWidget, &ViewportOpenGLWidget::OnTransformChanged);

    QToolButton settingsButton(this);

    modelLoader.Subscribe([&]() {  viewport()->update(); });

    scale(0.7, 0.7);
}

QByteArray GraphicsView::GetBaseColorTexture() {
    return openGLWidget->GetBaseColorTexture();
}

void GraphicsView::drawForeground(QPainter* painter, const QRectF& rect){
    painter->save();

    QPen pen(Qt::black);
    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->drawText(10,10,"Test");

    pen.setWidth(0);

    if (openGLWidget != nullptr) {
        printf("update DrawForeground\n");
        if (modelLoader.GetTextureCoordinatesSize() > 0) {
            QPainterPath path;
            const float textureWidth = 512.0f;
            const float textureHeight = 512.0f;
            for (int counter = 0; counter < modelLoader.GetTextureCoordinatesSize(); counter+=2) {
                if (counter == 0) {
                    const QPointF point(modelLoader.GetTextureCoordinates()[counter] * textureWidth, textureHeight - modelLoader.GetTextureCoordinates()[counter + 1] * textureHeight);
                    path.moveTo(point);
                    continue;
                }
                if (counter % 6 == 0) {
                    const QPointF oldPoint(modelLoader.GetTextureCoordinates()[counter - 6] * textureWidth, textureHeight - modelLoader.GetTextureCoordinates()[counter - 6 + 1] * textureHeight);
                    path.lineTo(oldPoint);
                    const QPointF point(modelLoader.GetTextureCoordinates()[counter] * textureWidth, textureHeight - modelLoader.GetTextureCoordinates()[counter + 1] * textureHeight);
                    path.moveTo(point);
                    continue;
                }
                const QPointF point(modelLoader.GetTextureCoordinates()[counter] * textureWidth, textureHeight - modelLoader.GetTextureCoordinates()[counter + 1] * textureHeight);
                path.lineTo(point);
            }

            painter->drawPath(path);
        } else {
            QPainterPath path;
            const float textureWidth = 512.0f;
            const float textureHeight = 512.0f;
            for (int counter = 0; counter < 72; counter += 2) {
                if (counter == 0) {
                    const QPointF point(cubeTextureCoordinates[counter] * textureWidth, textureHeight - cubeTextureCoordinates[counter + 1] * textureHeight);
                    path.moveTo(point);
                    continue;
                }
                if (counter % 6 == 0) {
                    const QPointF oldPoint(cubeTextureCoordinates[counter - 6] * textureWidth, textureHeight - cubeTextureCoordinates[counter - 6 + 1] * textureHeight);
                    path.lineTo(oldPoint);
                    const QPointF point(cubeTextureCoordinates[counter] * textureWidth, textureHeight - cubeTextureCoordinates[counter + 1] * textureHeight);
                    path.moveTo(point);
                    continue;
                }
                const QPointF point(cubeTextureCoordinates[counter] * textureWidth, textureHeight - cubeTextureCoordinates[counter + 1] * textureHeight);
                path.lineTo(point);
            }

            painter->drawPath(path);
        }
    }

    painter->restore();

    QGraphicsView::drawForeground(painter, rect);
}

void GraphicsView::drawBackground(QPainter* painter, const QRectF& rect){
    painter->save();

    painter->beginNativePainting();
    if(openGLWidget != nullptr){
        QMatrix4x4 projection;
        projection.ortho(0.0, viewport()->width(), 0.0, viewport()->height(), -1.0, 1.0);

        QMatrix4x4 modelView(painter->transform());
        QMatrix4x4 finalMatrix = projection * modelView;

        emit TransformChanged(finalMatrix);

        openGLWidget->paintGL();
    }
    painter->endNativePainting();

    painter->restore();

    QGraphicsView::drawBackground(painter, rect);
}

bool GraphicsView::event(QEvent* event){
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->buttons() == Qt::LeftButton) {
            const QPointF scenePosition = mapToScene(mouseEvent->pos());
            printf("scenePosition: %f %f\n", scenePosition.x(), scenePosition.y());
            const float textureSize = 512.0f;
            const QPointF mousePosition(2.0f * (scenePosition.x() / textureSize) - 1.0f, 1.0f - 2.0f * (scenePosition.y() / textureSize));
            printf("GraphicsView::event Qt::LeftButton mousePosition: %f %f\n", mousePosition.x(), mousePosition.y());

            emit drawChanged(mousePosition);
            return true;
        }
    }

    return QGraphicsView::event(event);
}

bool GraphicsView::eventFilter(QObject* object, QEvent* event){
    if(object == viewport()){
        if(event->type() == QEvent::MouseMove){
            if(QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)){
                if(!(mouseEvent->modifiers() & Qt::ShiftModifier) && mouseEvent->buttons() == Qt::LeftButton){
                    QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event);
                    const QPointF scenePosition = mapToScene(mouseEvent->pos());
                    const float textureSize = 512.0f;
                    const QPointF mousePosition(2.0f * (scenePosition.x() / textureSize) - 1.0f, 1.0f - 2.0f * (scenePosition.y() / textureSize));
                    printf("GraphicsView::eventFilter QEvent::MouseMove mousePosition: %f %f\n", mousePosition.x(), mousePosition.y());

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

    // I have to find a way to hide the scrollbars when they are non scrollable
    const int scrollBarHeight = 16;
    const int scrollBarWidth = 16;
    const bool isHorizontalScrollBarVisible = horizontalScrollBar()->maximum() != 0;
    const bool isVerticalScrollBarVisible = verticalScrollBar()->maximum() != 0;
    horizontalScrollBar()->setGeometry(0, height() - scrollBarHeight, isHorizontalScrollBarVisible ? width() - scrollBarWidth : 0, scrollBarHeight);
    verticalScrollBar()->setGeometry(width() - scrollBarWidth, 0, scrollBarWidth, isVerticalScrollBarVisible ? height() - scrollBarHeight : 0);
}

void GraphicsView::wheelEvent(QWheelEvent* event){
//    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    const int scrollBarHeight = 16;
    const int scrollBarWidth = 16;
    const bool isHorizontalScrollBarVisible = horizontalScrollBar()->maximum() != 0;
    const bool isVerticalScrollBarVisible = verticalScrollBar()->maximum() != 0;
    horizontalScrollBar()->setGeometry(0, height() - scrollBarHeight, isHorizontalScrollBarVisible ? width() - scrollBarWidth : 0, scrollBarHeight);
    verticalScrollBar()->setGeometry(width() - scrollBarWidth, 0, scrollBarWidth, isVerticalScrollBarVisible ? height() - scrollBarHeight : 0);

    if(event->angleDelta().y() > 0){
        if(transform().m11() > 55.1 || transform().m22() > 55.1){
        }else{
            scale(1.01, 1.01);
        }
    }else{
        if(transform().m11() < 0.1 || transform().m22() < 0.1){
        }else{
            scale(1.0/1.01, 1.0/1.01);
        }
    }

    const QPointF sceneCenter = mapToScene(viewport()->rect().center());
    centerOn(sceneCenter);

    event->accept();
}

void GraphicsView::mouseReleaseEvent(QMouseEvent* mouseEvent) {
    if (mouseEvent->button() == Qt::RightButton) {
        printf("Right Mouse Button Released\n");

        QMenu* saveMenu = new QMenu(this);
        saveMenu->setAttribute(Qt::WA_DeleteOnClose);
        QAction* saveAction = saveMenu->addAction("Export Texture...");
        connect(saveAction, &QAction::triggered, this, [&]() {
            QByteArray saveTextureByteArray = openGLWidget->GetBaseColorTexture();
            QByteArray newByteArray;
            QImage saveImage(reinterpret_cast<const uchar*>(saveTextureByteArray.constData()), 512,512, QImage::Format_RGBA8888);
            QBuffer buffer(&newByteArray);
            buffer.open(QIODevice::WriteOnly);
            if (!saveImage.save(&buffer, "PNG")) {
                printf("image could not be saved in format\n");
            }

            QFileDialog::saveFileContent(newByteArray, "texture");
        });
        saveMenu->popup(mouseEvent->globalPosition().toPoint());

        mouseEvent->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(mouseEvent);
}

void GraphicsView::viewMoved(int value) {
    verticalScrollBar()->blockSignals(true);
    horizontalScrollBar()->blockSignals(true);
    //const QPointF viewCenter = mapToScene(viewport()->rect().center());
    //printf("%f %f\n", viewCenter.x(), viewCenter.y());
    //printf("1: %f %f %f\n", transform().m11(), transform().m12(), transform().m13());
    //printf("2: %f %f %f\n", transform().m21(), transform().m22(), transform().m23());
    //printf("3: %f %f %f\n", transform().m31(), transform().m32(), transform().m33());

    //QTransform newTransform;
    //newTransform.setMatrix(transform().m11(), transform().m12(), transform().m13(), transform().m21(), transform().m22(), transform().m23(), viewCenter.x(), viewCenter.y(), transform().m33());
    //setTransform(newTransform);

    //newTransform.setMatrix(512.0 / (qreal)viewport()->width(), transform().m12(), transform().m13(), transform().m21(), 512.0 / (qreal)viewport()->height(), transform().m23(), 2.0 * (viewCenter.x() / viewport()->width()) - 1.0, 2.0 * (viewCenter.y() / viewport()->height()) - 1.0f, transform().m33());

    //printf("1: %f %f %f\n", newTransform.m11(), newTransform.m12(), newTransform.m13());
    //printf("2: %f %f %f\n", newTransform.m21(), newTransform.m22(), newTransform.m23());
    //printf("3: %f %f %f\n", newTransform.m31(), newTransform.m32(), newTransform.m33());

    //emit TransformChanged(newTransform);

    //viewport()->update();

    verticalScrollBar()->blockSignals(false);
    horizontalScrollBar()->blockSignals(false);
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
                                        "vec4 transformedPosition = transformMatrix * vec4(position.x * 256.0 + 256.0, (position.y * 256.0 + 256.0), 0.0, 1.0);\n"
                                        "gl_Position = vec4(transformedPosition.xy, 0.0, 1.0);\n"
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
    drawTextureShader.AddAttribute(&quadTextureCoordinates[0], 12, "textureCoordinates", 2);

    drawTextureShader.AddUniform(nullptr, 1, "textureSampler", GL_FALSE);

    //for(int i = 0; i < 4; i++){
    //    printf("transformMatrix: %f %f %f %f\n", transformMatrix.data()[i*4], transformMatrix.data()[i*4+1], transformMatrix.data()[i*4+2], transformMatrix.data()[i*4+3]);
    //}
    drawTextureShader.AddUniform(transformMatrix.data(), 16, "transformMatrix", GL_FALSE);
    AspectRatio = ViewHeight/ViewWidth;
    drawTextureShader.AddUniform(&AspectRatio, 1, "aspectRatio", GL_FALSE);
    const float textureWidth = 512.0f;
    const float textureHeight = 512.0f;
    const float scaleFactor[2] = {textureWidth/ViewWidth, textureHeight/ViewHeight};
    drawTextureShader.AddUniform(&scaleFactor[0], 2, "scaleFactor", GL_FALSE);


    resizeGL(width(), height());
}

void GraphicsView::ViewportOpenGLWidget::paintGL(){
    printf("GraphicsView::ViewportOpenGLWidget::paintGL\n");
    //glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, width(), height());
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    drawTextureShader.UseProgram();
    drawTextureShader.BindVAO();

    drawTextureShader.ChangeUniform(1, transformMatrix.data(), 16, GL_FALSE);

    glBindTexture(GL_TEXTURE_2D, baseColorTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
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
    const float textureWidth = 512.0f;
    const float textureHeight = 512.0f;
    const float scaleFactor[2] = {textureWidth/ViewWidth, textureHeight/ViewHeight};
    drawTextureShader.ChangeUniform(3, &scaleFactor[0], 2, GL_FALSE);
}

void GraphicsView::ViewportOpenGLWidget::OnTransformChanged(const QMatrix4x4 transform){
    transformMatrix = QMatrix4x4(transform);

    //printf("transformMatrix\n");
    //printf("%f %f %f %f\n", transformMatrix.data()[0], transformMatrix.data()[1], transformMatrix.data()[2], transformMatrix.data()[3]);
    //printf("%f %f %f %f\n", transformMatrix.data()[4], transformMatrix.data()[5], transformMatrix.data()[6], transformMatrix.data()[7]);
    //printf("%f %f %f %f\n", transformMatrix.data()[8], transformMatrix.data()[9], transformMatrix.data()[10], transformMatrix.data()[11]);
    //printf("%f %f %f %f\n", transformMatrix.data()[12], transformMatrix.data()[13], transformMatrix.data()[14], transformMatrix.data()[15]);
}

QByteArray GraphicsView::ViewportOpenGLWidget::GetBaseColorTexture() {
    QByteArray textureByteArray;
    textureByteArray.resize(512 * 512 * 4);
    makeCurrent();
    GLuint tempFramebuffer = 0;
    glGenFramebuffers(1, &tempFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, tempFramebuffer);
    printf("baseColorTexture: %i\n", baseColorTexture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, baseColorTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, textureByteArray.data());
    } else {
        printf("temp framebuffer incomplete\n");
    }
    glDeleteFramebuffers(1, &tempFramebuffer);
    doneCurrent();
    return textureByteArray;
}
