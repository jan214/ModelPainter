#include "brushwidget.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

ColorPicker::ColorPicker(QWidget* parent) :
QDialog(parent),
mainLayout(this),
colorPaletteImage(256, 256, QImage::Format_RGBA8888),
colorPaletteWrapper(this),
colorSliderWrapper(this),
colorSliderWrapperLayout(&colorSliderWrapper),
redSlider("Red", &colorSliderWrapper),
greenSlider("Green", &colorSliderWrapper),
blueSlider("Blue", &colorSliderWrapper),
closeButton("Ok"){
    setWindowFlags(Qt::Popup);
    mainLayout.setSpacing(0);

    // color wheel calculations
    for(int x = 0; x < 256; x++){
        for(int y = 0; y < 256; y++){
            const int dx = x - 128;
            const int dy = y - 128;
            const double angle = std::atan2(dy, dx);
            double hue = (angle * 180.0 / M_PI);
            if (hue < 0){
                hue += 360.0;
            }

            const double dist = std::sqrt(dx*dx + dy*dy);
            const double maxRadius = 256.0 / 2.0;
            const double saturation = qBound(0.0, dist / maxRadius, 1.0);

            if(dist > maxRadius){
                colorPaletteImage.setPixelColor(x, y, Qt::transparent);
                continue;
            }

            colorPaletteImage.setPixelColor(x, y, QColor::fromHsvF(hue / 360.0, saturation, 1.0));
        }
    }


    colorPaletteWrapper.setAlignment(Qt::AlignCenter);
    colorPaletteWrapper.setPixmap(QPixmap::fromImage(colorPaletteImage));
    mainLayout.addWidget(&colorPaletteWrapper);

    colorPaletteWrapper.installEventFilter(this);

    QWidget* colorSliders[3] = { &redSlider, &greenSlider, &blueSlider };
    SliderWidget::GetLongestNameplateWidth(colorSliders, 3);

    colorSliderWrapperLayout.setSpacing(0);
    colorSliderWrapperLayout.addWidget(&redSlider);
    colorSliderWrapperLayout.addWidget(&greenSlider);
    colorSliderWrapperLayout.addWidget(&blueSlider);

    mainLayout.addWidget(&colorSliderWrapper);

    closeButton.setFlat(true);
    closeButton.setAutoFillBackground(true);
    mainLayout.addWidget(&closeButton);
    connect(&closeButton, &QPushButton::clicked, this, &QDialog::close);
}

void ColorPicker::Initialize(int red, int green, int blue){
    redSlider.OnSliderValueChanged(red);
    greenSlider.OnSliderValueChanged(green);
    blueSlider.OnSliderValueChanged(blue);
}

bool ColorPicker::eventFilter(QObject* object, QEvent* event){
    if(object == &colorPaletteWrapper){
        if(event->type() == QEvent::MouseMove){
            if(QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event)){
                if(mouseEvent->buttons() == Qt::LeftButton){
                    const QPoint mousePosition = mouseEvent->pos();
                    QColor selectedPixelColor = colorPaletteImage.pixelColor(mousePosition.x(), mousePosition.y());
                    printf("event on color picker: %i %i %i\n", selectedPixelColor.red(), selectedPixelColor.green(), selectedPixelColor.blue());
                    if(selectedPixelColor.alpha() == 0){
                        return true;
                    }
                    const float redPortion = selectedPixelColor.redF();
                    const float greenPortion = selectedPixelColor.greenF();
                    const float bluePortion = selectedPixelColor.blueF();
                    redSlider.OnSpinboxValueChanged((double)redPortion);
                    redSlider.OnSliderValueChanged(100*redPortion);
                    greenSlider.OnSpinboxValueChanged((double)greenPortion);
                    greenSlider.OnSliderValueChanged(100*greenPortion);
                    blueSlider.OnSpinboxValueChanged((double)bluePortion);
                    blueSlider.OnSliderValueChanged(100*bluePortion);

                    emit colorChanged(selectedPixelColor);

                    QPalette newBackgroundPalette;
                    newBackgroundPalette.setColor(QPalette::Button, selectedPixelColor);
                    closeButton.setPalette(newBackgroundPalette);

                    return true;
                }
            }
        }
    }

    return QDialog::eventFilter(object, event);
}

ColorPickerWidget::ColorPickerWidget(QString text, ColorPicker& colorPicker, QWidget* parent) :
QWidget(parent),
mainLayout(this),
colorPickerLabel(text),
colorPickerColor(),
colorPicker(colorPicker){
    colorPickerLabel.setObjectName("NameLabel");
    mainLayout.addWidget(&colorPickerLabel);
    mainLayout.addWidget(&colorPickerColor);
    colorPickerColor.setFlat(true);

    QPalette initialPalette;
    initialPalette.setColor(QPalette::Button, Qt::green);
    colorPickerColor.setPalette(initialPalette);
    colorPickerColor.setAutoFillBackground(true);

    connect(&colorPickerColor, &QPushButton::pressed, [this, &colorPicker](){
        QPoint globalSpawnPosition = colorPickerColor.mapToGlobal(colorPickerColor.rect().topLeft());
        const QColor pickerColor = colorPickerColor.palette().color(QPalette::Normal, QPalette::Button);
        colorPicker.Initialize(pickerColor.red(), pickerColor.green(), pickerColor.blue());
        colorPicker.move(globalSpawnPosition);
        colorPicker.open();
    });
}

QColor ColorPickerWidget::GetPickerColor(){
    return colorPickerColor.palette().color(QPalette::Normal, QPalette::Button);
}

void ColorPickerWidget::SetPickerColor(const QColor newColor){
    QPalette initialPalette;
    initialPalette.setColor(QPalette::Button, newColor);
    colorPickerColor.setPalette(initialPalette);
}

SliderWidget::SliderWidget(QString text, QWidget* parent, double minimum, double maximum, double value) :
QWidget(parent),
mainLayout(this),
sliderLabel(text),
slider(),
sliderSpinbox(),
minimum(minimum),
maximum(maximum),
value(value){
    mainLayout.setSpacing(0);
    sliderLabel.setObjectName("NameLabel");
    mainLayout.addWidget(&sliderLabel);
    slider.setOrientation(Qt::Horizontal);
    slider.setRange(0, 100);
    slider.setValue(0);
    mainLayout.addWidget(&slider);
    connect(&slider, &QSlider::valueChanged, this, &SliderWidget::OnSliderValueChanged);

    sliderSpinbox.setRange(minimum, maximum);
    sliderSpinbox.setValue(minimum);
    sliderSpinbox.setSingleStep((maximum-minimum)*0.1);
    mainLayout.addWidget(&sliderSpinbox);
    connect(&sliderSpinbox, &QDoubleSpinBox::valueChanged, this, &SliderWidget::OnSpinboxValueChanged);
}

SliderWidget::~SliderWidget(){

}

void SliderWidget::OnSliderValueChanged(int sliderValue){
    value = minimum + (maximum - minimum) * (sliderValue / 100.0);
    QSignalBlocker sliderSpinboxSignalBlocker(sliderSpinbox);
    sliderSpinbox.setValue(value);

    emit changedValue(value, maximum);
}

void SliderWidget::OnSpinboxValueChanged(double value){
    const int newValue = ((value - minimum)/(maximum - minimum))*100;
    QSignalBlocker sliderSignalBlocker(slider);
    slider.setValue(newValue);

    emit changedValue(value, maximum);
}

void SliderWidget::GetLongestNameplateWidth(QWidget* widgets[], const int size){
    int longestNameplate = 0;
    for(int counter = 0; counter < size; counter++){
        if(QLabel* nameLabel = widgets[counter]->findChild<QLabel*>("NameLabel")){
            QFontMetrics fontMetrics = nameLabel->fontMetrics();
            int namePlateWidth = fontMetrics.horizontalAdvance(nameLabel->text());
            if(namePlateWidth > longestNameplate){
                longestNameplate = namePlateWidth;
            }
        }
    }

    for(int counter = 0; counter < size; counter++){
        if(QLabel* nameLabel = widgets[counter]->findChild<QLabel*>("NameLabel")){
            nameLabel->setFixedWidth(longestNameplate);
        }
    }
}

BrushWidget::BrushWidget(QWidget* parent) :
QWidget(parent),
mainLayout(),
brushPreview(128,128, QImage::Format_RGBA8888),
brushPreviewWrapper(this),
colorPicker(this),
colorPickerWidget("Color", colorPicker, this),
smoothnessSlider("Smoothness", this, 0.1){
    setLayout(&mainLayout);

    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QRadialGradient radialGradient(brushPreview.width()/2, brushPreview.height()/2, brushPreview.width()/2);
    QColor pickerColor = colorPickerWidget.GetPickerColor();
    radialGradient.setColorAt(0.0, pickerColor);
    radialGradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(radialGradient);
    painter.drawEllipse(0.0, 0.0, brushPreview.width(), brushPreview.height());

    brushPreviewWrapper.setPixmap(QPixmap::fromImage(brushPreview));
    mainLayout.addWidget(&brushPreviewWrapper);
    connect(&colorPicker, &ColorPicker::colorChanged, this, &BrushWidget::onColorChanged);

    mainLayout.addWidget(&colorPickerWidget);

    mainLayout.addWidget(&smoothnessSlider);
    connect(&smoothnessSlider, &SliderWidget::changedValue, this, &BrushWidget::onValueChanged);

    QWidget* widgets[2] = { &colorPickerWidget, &smoothnessSlider };
    SliderWidget::GetLongestNameplateWidth(widgets, 2);
}

void BrushWidget::onValueChanged(double value, double maximum){
    printf("onValueChanged\n");
    const double sliderDoubleValue = value/maximum;
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QRadialGradient radialGradient(brushPreview.width()/2, brushPreview.height()/2, brushPreview.width()/2);
    QColor pickerColor = colorPickerWidget.GetPickerColor();
    radialGradient.setColorAt(1.0-sliderDoubleValue, pickerColor);
    radialGradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(radialGradient);
    painter.drawEllipse(0.0, 0.0, brushPreview.width(), brushPreview.height());

    brushPreviewWrapper.setPixmap(QPixmap::fromImage(brushPreview));

    emit BrushChanged(brushPreview);
}

void BrushWidget::onColorChanged(QColor newColor){
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QRadialGradient radialGradient(brushPreview.width()/2, brushPreview.height()/2, brushPreview.width()/2);
    colorPickerWidget.SetPickerColor(newColor);
    radialGradient.setColorAt(0.0, newColor);
    radialGradient.setColorAt(1.0, Qt::transparent);
    painter.setBrush(radialGradient);
    painter.drawEllipse(0.0, 0.0, brushPreview.width(), brushPreview.height());

    brushPreviewWrapper.setPixmap(QPixmap::fromImage(brushPreview));

    emit BrushChanged(brushPreview);
}
