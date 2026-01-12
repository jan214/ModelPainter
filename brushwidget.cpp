#include "brushwidget.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QPainter>

BrushWidget::BrushWidget(QWidget* parent) :
QWidget(parent),
layout(),
brushPreview(128,128, QImage::Format_ARGB32),
brushPreviewWrapper(this),
slider1(this),
slider2(this){
    setLayout(&layout);
    QToolButton button(this);
    button.setText(QString("Test Button"));
    QObject::connect(&button, &QToolButton::clicked, [](){
        qDebug() << "hello";
    });
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QRadialGradient radialGradient(brushPreview.width()/2, brushPreview.height()/2, brushPreview.width()/2);
    radialGradient.setColorAt(0.0, Qt::black);
    radialGradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(radialGradient);
    painter.drawEllipse(0.0, 0.0, brushPreview.width(), brushPreview.height());

    brushPreviewWrapper.setPixmap(QPixmap::fromImage(brushPreview));
    layout.addWidget(&brushPreviewWrapper);
    layout.addWidget(&button);
    slider1.setOrientation(Qt::Horizontal);

    QObject::connect(&slider1, &QSlider::valueChanged, this, &BrushWidget::onValueChanged);
    layout.addWidget(&slider1);
    slider2.setOrientation(Qt::Horizontal);
    layout.addWidget(&slider2);
}

void BrushWidget::onValueChanged(int value){
    printf("onValueChanged\n");
    const float sliderFloatValue = (float)value/(float)slider1.maximum();
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QRadialGradient radialGradient(brushPreview.width()/2, brushPreview.height()/2, brushPreview.width()/2);
    radialGradient.setColorAt(1.0-sliderFloatValue, Qt::black);
    radialGradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(radialGradient);
    painter.drawEllipse(0.0, 0.0, brushPreview.width(), brushPreview.height());

    brushPreviewWrapper.setPixmap(QPixmap::fromImage(brushPreview));

    emit BrushChanged(brushPreview);
}
