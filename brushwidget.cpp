#include "brushwidget.h"

#include <QVBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOption>
#include <QGraphicsBlurEffect>

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

    setFixedWidth(278);

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

bool ColorPicker::eventFilter(QObject* object, QEvent* event){
    if(object == &colorPaletteWrapper){
        if(event->type() == QEvent::MouseMove){
            if(QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event)){
                if(mouseEvent->buttons() == Qt::LeftButton){
                    const QPoint mousePosition = mouseEvent->pos();

                    if (!colorPaletteImage.rect().contains(mousePosition)) {
                        return true;
                    }

#ifdef __EMSCRIPTEN__
                    if (!rect().contains(mousePosition)) {
                        close();
                    }
#endif

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
colorPickerColor(){
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

        const QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
        const int colorPickerRightEdge = globalSpawnPosition.x() + colorPicker.rect().width();
        const int rightEdgeDistance = screenRect.right() - colorPickerRightEdge;
        if (rightEdgeDistance < 0) {
            globalSpawnPosition.setX(globalSpawnPosition.x() + rightEdgeDistance);
        }
        
        // needed for initialize for a different ColorPickerWidget
        //const QColor pickerColor = colorPickerColor.palette().color(QPalette::Normal, QPalette::Button);
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
sliderLabel(text, this),
sliderProxyStyle(),
slider(),
sliderSpinbox(),
minimum(minimum),
maximum(maximum),
value(value),
spinboxDrag({}) {
    mainLayout.setSpacing(0);
    slider.setStyle(&sliderProxyStyle);

    sliderLabel.setObjectName("NameLabel");
    mainLayout.addWidget(&sliderLabel);
    slider.setOrientation(Qt::Horizontal);
    slider.setRange(0, 100);
    slider.setValue(value * 100);
    mainLayout.addWidget(&slider);
    connect(&slider, &QSlider::valueChanged, this, &SliderWidget::OnSliderValueChanged);

    sliderSpinbox.setMinimumWidth(48);
    sliderSpinbox.setRange(minimum, maximum);
    sliderSpinbox.setValue(value);
    sliderSpinbox.setSingleStep((maximum-minimum)*0.1);
    mainLayout.addWidget(&sliderSpinbox);
    connect(&sliderSpinbox, &QDoubleSpinBox::valueChanged, this, &SliderWidget::OnSpinboxValueChanged);

    spinboxDrag.lineEdit = sliderSpinbox.findChild<QLineEdit*>();
    if (spinboxDrag.lineEdit != nullptr) {
        spinboxDrag.lineEdit->setMouseTracking(true);
        spinboxDrag.lineEdit->installEventFilter(this);
    }
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

bool SliderWidget::eventFilter(QObject* object, QEvent* event) {
    if (spinboxDrag.lineEdit != nullptr && object == spinboxDrag.lineEdit) {
        if (event->type() == QEvent::MouseButtonPress) {
            printf("sliderSpinbox MouseButtonPress\n");
            spinboxDrag.mouseDown = true;
            if (QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)) {
                spinboxDrag.mousePosition = mouseEvent->pos();
            }
        } else if (spinboxDrag.mouseDown && event->type() == QEvent::MouseMove) {
            if (QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)) {
                const QPoint mousePosition = mouseEvent->pos();
                const QPoint delta = mousePosition - spinboxDrag.mousePosition;
                if (spinboxDrag.mouseDown || delta.manhattanLength() > 5) {
                    // delta.y() down is 1, so reverse it
                    const int deltay = -delta.y();
                    if (mouseEvent->modifiers() & Qt::ShiftModifier) {
                        sliderSpinbox.setValue(sliderSpinbox.value() + (deltay * sliderSpinbox.singleStep()) / 10);
                    } else {
                        sliderSpinbox.setValue(sliderSpinbox.value() + deltay * sliderSpinbox.singleStep());
                    }
                    
                    spinboxDrag.mousePosition = mousePosition;
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            spinboxDrag.mouseDown = false;
        }
    }

    return QWidget::eventFilter(object, event);
}

void SliderWidget::resizeEvent(QResizeEvent* resizeEvent) {
    const int labelSpinboxWidth = sliderLabel.width() + sliderSpinbox.minimumWidth();
    const int sliderWidth = width() - labelSpinboxWidth;
    
    if (250 < sliderWidth) {
        slider.setVisible(true);
    } else {
        slider.setVisible(false);
    }

    QWidget::resizeEvent(resizeEvent);
}

SliderWidget::SliderProxyStyle::SliderProxyStyle(QStyle* style) : 
QProxyStyle(){

}

SliderWidget::SliderProxyStyle::~SliderProxyStyle() {

}

void SliderWidget::SliderProxyStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget) const{
    if (control != CC_Slider) {
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        return;
    }

    if (const QStyleOptionSlider* sliderOption = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        const QRect grooveRect = subControlRect(CC_Slider, sliderOption, SC_SliderGroove, widget);
        const QRect handleRect = subControlRect(CC_Slider, sliderOption, SC_SliderHandle, widget);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect customGroove = grooveRect;

        QLinearGradient linearBackgroundGradient(QPoint(0, 0), QPoint(100, 0));
        linearBackgroundGradient.setColorAt(0.0, QColor(10,10,10));
        linearBackgroundGradient.setColorAt(1.0, QColor(100, 100, 100));

        painter->setPen(Qt::NoPen);
        painter->setBrush(linearBackgroundGradient);
        painter->drawRoundedRect(customGroove, 4.0, 4.0);

        QRect filledRect = grooveRect;
        filledRect.setRight(handleRect.center().x());

        const int filledPercentage = ((sliderOption->sliderValue - sliderOption->minimum) * 100) / (sliderOption->maximum - sliderOption->minimum);
        QColor finishedColor;

        if (filledPercentage > 95) {
            finishedColor = Qt::red;
        } else if (filledPercentage > 75) {
            QColor color1(Qt::lightGray);
            QColor color2(Qt::red);

            const float invertThreshold = 1.0f / 85.0f;
            const float filledPercentageFloat = (filledPercentage - 75) * 4.0f / 100.0f;
            const float redChannel = color1.red() + (color2.red() - color1.red()) * filledPercentageFloat;
            const float greenChannel = color1.green() + (color2.green() - color1.green()) * filledPercentageFloat;
            const float blueChannel = color1.blue() + (color2.blue() - color1.blue()) * filledPercentageFloat;
            finishedColor = QColor(redChannel, greenChannel, blueChannel);
        } else {
            QColor color1(Qt::blue);
            QColor color2(Qt::lightGray);

            const float filledPercentageFloat = filledPercentage / 100.0f * (1.0f / 0.75f);
            const float redChannel = color1.red() + (color2.red() - color1.red()) * filledPercentageFloat;
            const float greenChannel = color1.green() + (color2.green() - color1.green()) * filledPercentageFloat;
            const float blueChannel = color1.blue() + (color2.blue() - color1.blue()) * filledPercentageFloat;
            finishedColor = QColor(redChannel, greenChannel, blueChannel);
        }

        QRadialGradient radialGradient(handleRect.center(), 200);
        radialGradient.setColorAt(0.0, QColor(~finishedColor.red(), ~finishedColor.green(), ~finishedColor.blue()));
        radialGradient.setColorAt(1.0, finishedColor);

        painter->setBrush(radialGradient);
        painter->drawRoundedRect(filledRect, 4.0, 4.0);

        QRect customHandleRect(handleRect.center().x() - 12, handleRect.y(), 24, handleRect.height());
        if (sliderOption->state & QStyle::State_MouseOver) {
            painter->setBrush(Qt::white);
        } else {
            painter->setBrush(Qt::lightGray);
        }

        painter->drawRect(customHandleRect);

        painter->restore();
    }
}

QRect SliderWidget::SliderProxyStyle::subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subcontrol, const QWidget* widget) const{
    if (control != CC_Slider) {
        return QProxyStyle::subControlRect(control, option, subcontrol, widget);
    }

    if (const QStyleOptionSlider* sliderOption = qstyleoption_cast<const QStyleOptionSlider*>(option)) {
        const int handleWidth = 24;

        if (subcontrol == SC_SliderHandle) {
            const int sliderPosition = sliderPositionFromValue(sliderOption->minimum, sliderOption->maximum, sliderOption->sliderValue, sliderOption->rect.width() - handleWidth);
            return QRect(sliderPosition, 0, handleWidth, sliderOption->rect.height());
        }

        if (subcontrol == SC_SliderGroove) {
            QRect grooveRect = sliderOption->rect;
            grooveRect.setLeft(grooveRect.left() + handleWidth / 2);
            grooveRect.setRight(grooveRect.right() - handleWidth / 2);
            return sliderOption->rect;
        }
    }

    return QProxyStyle::subControlRect(control, option, subcontrol, widget);
}

BrushWidget::BrushWidget(QWidget* parent) :
QWidget(parent),
mainLayout(this),
scrollArea(this),
scrollAreaWrapperWidget(&scrollArea),
scrollAreaWrapperWidgetLayout(&scrollAreaWrapperWidget),
brushPreview(128,128, QImage::Format_RGBA8888),
brushPreviewWrapper(this),
colorPickerWidget("Color", ColorPicker::GetInstance(), this),
smoothnessSlider("Smoothness", this),
sizeSlider("Size", this, 0.0, 1.0, 1.0),
brushProperties{ 0.0f, 1.0f } {
    const ColorPicker& colorPicker = ColorPicker::GetInstance();

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
    scrollAreaWrapperWidgetLayout.addWidget(&brushPreviewWrapper);
    connect(&colorPicker, &ColorPicker::colorChanged, this, &BrushWidget::onColorChanged);

    scrollAreaWrapperWidgetLayout.addWidget(&colorPickerWidget);

    scrollAreaWrapperWidgetLayout.addWidget(&smoothnessSlider);
    connect(&smoothnessSlider, &SliderWidget::changedValue, this, &BrushWidget::onValueChanged);

    scrollAreaWrapperWidgetLayout.addWidget(&sizeSlider);
    connect(&sizeSlider, &SliderWidget::changedValue, this, &BrushWidget::onSizeChanged);

    constexpr int widgetsCount = 3;
    QWidget* widgets[widgetsCount] = { &colorPickerWidget, &smoothnessSlider, &sizeSlider };
    SliderWidget::GetLongestNameplateWidth(widgets, widgetsCount);

    scrollArea.setWidgetResizable(true);
    scrollArea.setWidget(&scrollAreaWrapperWidget);
    mainLayout.addWidget(&scrollArea);
}

void BrushWidget::onValueChanged(double value, double maximum){
    printf("onSmoothnessChanged\n");
    brushProperties.Smoothness = value;
    const QImage previewImage = calculateBrushPreview();
    brushPreviewWrapper.setPixmap(QPixmap::fromImage(previewImage));

    emit BrushChanged(previewImage);
}

void BrushWidget::onColorChanged(QColor newColor){
    brushPreview.fill(Qt::transparent);

    colorPickerWidget.SetPickerColor(newColor);
    QImage previewImage = calculateBrushPreview();
    brushPreviewWrapper.setPixmap(QPixmap::fromImage(previewImage));

    emit BrushChanged(previewImage);
}

void BrushWidget::onSizeChanged(double value, double maximum) {
    printf("onSizeChanged\n");
    brushProperties.Size = value;
    const QImage previewImage = calculateBrushPreview();
    brushPreviewWrapper.setPixmap(QPixmap::fromImage(previewImage));

    emit BrushChanged(previewImage);
}

QImage BrushWidget::calculateBrushPreview() {
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setPen(Qt::NoPen);
    QColor pickerColor = colorPickerWidget.GetPickerColor();
    painter.setBrush(pickerColor);
    const int brushPreviewWidth = brushPreview.width();
    const int brushPreviewHeight = brushPreview.height();
    painter.drawEllipse(brushPreviewWidth / 2 * (1.0f - brushProperties.Size), brushPreviewHeight / 2 * (1.0f - brushProperties.Size), brushPreviewWidth * brushProperties.Size, brushPreviewHeight * brushProperties.Size);
    const QSize smallSize(brushPreviewWidth * (1.01f - brushProperties.Smoothness), brushPreviewHeight * (1.01f - brushProperties.Smoothness));
    const QImage smallImage = brushPreview.scaled(smallSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return smallImage.scaled(brushPreview.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}
