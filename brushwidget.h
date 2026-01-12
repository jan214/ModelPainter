#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QSlider>
#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif

class BrushWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushWidget(QWidget* parent = nullptr);
    ~BrushWidget(){}

signals:
    void BrushChanged(const QImage& brushTexture);

protected:
    void onValueChanged(int value);

    QVBoxLayout layout;
    QImage brushPreview;
    QLabel brushPreviewWrapper;
    QSlider slider1;
    QSlider slider2;
};

#endif // BRUSHWIDGET_H
