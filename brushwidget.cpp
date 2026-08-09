#include "brushwidget.h"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOption>
#include <QLineEdit>
#include <QMimeData>
#include <QToolButton>
#include <QImageReader>
#include <QScrollBar>

ColorPicker::ColorPicker(QWidget* parent) :
QDialog(parent),
mainLayout(this),
colorPaletteImage(256, 256, QImage::Format_RGBA8888_Premultiplied),
colorPaletteWrapper(this),
colorSliderWrapper(this),
colorSliderWrapperLayout(&colorSliderWrapper),
redSlider("Red", &colorSliderWrapper),
greenSlider("Green", &colorSliderWrapper),
blueSlider("Blue", &colorSliderWrapper),
brightnessSlider("Brightness", &colorSliderWrapper),
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

    constexpr int sliderCounter = 4;
    QWidget* colorSliders[sliderCounter] = { &redSlider, &greenSlider, &blueSlider, &brightnessSlider };
    SliderWidget::GetLongestNameplateWidth(colorSliders, sliderCounter);

    // I might change this later by adding a new ColorSliderWidget class that can connect to a single colorChanged function
    connect(&redSlider, &SliderWidget::changedValue, this, &ColorPicker::onRedColorChanged);
    connect(&greenSlider, &SliderWidget::changedValue, this, &ColorPicker::onGreenColorChanged);
    connect(&blueSlider, &SliderWidget::changedValue, this, &ColorPicker::onBlueColorChanged);
    connect(&brightnessSlider, &SliderWidget::changedValue, this, &ColorPicker::onBrightnessChanged);

    colorSliderWrapperLayout.setSpacing(0);
    for (QWidget* const slider : colorSliders) {
        colorSliderWrapperLayout.addWidget(slider);
    }

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
                    printf("mouseEvent->buttons() == Qt::LeftButton\n");
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

                    event->accept();
                    return true;
                }
            }
        }
    }

    return QDialog::eventFilter(object, event);
}

void ColorPicker::onRedColorChanged(double value, double maximum) {
    QColor selectedPixelColor(value * 255, greenSlider.GetValue() * 255, blueSlider.GetValue() * 255);
    emit colorChanged(selectedPixelColor);

    QPalette newBackgroundPalette;
    newBackgroundPalette.setColor(QPalette::Button, selectedPixelColor);
    closeButton.setPalette(newBackgroundPalette);
}

void ColorPicker::onGreenColorChanged(double value, double maximum) {
    QColor selectedPixelColor(redSlider.GetValue() * 255, value * 255, blueSlider.GetValue() * 255);
    emit colorChanged(selectedPixelColor);

    QPalette newBackgroundPalette;
    newBackgroundPalette.setColor(QPalette::Button, selectedPixelColor);
    closeButton.setPalette(newBackgroundPalette);
}

void ColorPicker::onBlueColorChanged(double value, double maximum) {
    QColor selectedPixelColor(redSlider.GetValue() * 255, greenSlider.GetValue() * 255, value * 255);
    emit colorChanged(selectedPixelColor);

    QPalette newBackgroundPalette;
    newBackgroundPalette.setColor(QPalette::Button, selectedPixelColor);
    closeButton.setPalette(newBackgroundPalette);
}

void ColorPicker::onBrightnessChanged(double value, double maximum) {

}

ColorPickerWidget::ColorPickerWidget(QString text, ColorPicker& colorPicker, QWidget* parent) :
QWidget(parent),
mainLayout(this),
colorPickerLabel(text),
colorPickerColor(){
    setFixedHeight(40);
    mainLayout.setSpacing(0);

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

void ColorPickerWidget::paintEvent(QPaintEvent* event) {
    QStyleOption styleOption;
    styleOption.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
}

SliderWidget::SliderWidget(QString text, QWidget* parent, double minimum, double maximum, double value) :
QWidget(parent),
mainLayout(QBoxLayout::Direction::LeftToRight, this),
sliderLabel(text, this),
sliderProxyStyle(),
slider(),
sliderSpinbox(),
minimum(minimum),
maximum(maximum),
value(value),
spinboxDrag({}) {
    setFixedHeight(40);
    mainLayout.setSpacing(0);

    slider.setStyle(&sliderProxyStyle);

    sliderLabel.setObjectName("NameLabel");
    mainLayout.addWidget(&sliderLabel);

    slider.setMouseTracking(true);
    slider.setOrientation(Qt::Horizontal);
    slider.setRange(0, 100);
    slider.setValue(value * 100);
    slider.installEventFilter(this);
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

double SliderWidget::GetValue() {
    return value;
}

void SliderWidget::OnSliderValueChanged(int sliderValue){
    value = minimum + (maximum - minimum) * (sliderValue / 100.0);
    QSignalBlocker sliderSpinboxSignalBlocker(sliderSpinbox);
    sliderSpinbox.setValue(value);

    emit changedValue(value, maximum);
}

void SliderWidget::OnSpinboxValueChanged(double spinboxValue){
    value = ((spinboxValue - minimum)/(maximum - minimum));
    QSignalBlocker sliderSignalBlocker(slider);
    slider.setValue(value * 100);

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
    const QEvent::Type& type = event->type();
    if (spinboxDrag.lineEdit != nullptr && object == spinboxDrag.lineEdit) {
        if (type == QEvent::MouseButtonPress) {
            printf("sliderSpinbox MouseButtonPress\n");
            spinboxDrag.mouseDown = true;
            if (QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)) {
                spinboxDrag.mousePosition = mouseEvent->pos();
            }
        } else if (spinboxDrag.mouseDown && type == QEvent::MouseMove) {
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
        } else if (type == QEvent::MouseButtonRelease) {
            spinboxDrag.mouseDown = false;
        }
    } else if (object == &slider && type == QEvent::MouseMove) {
        // call slider update on mouse move to activate gradient based on mouse position
        if (QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)) {
            slider.update();
        }
    }

    return QWidget::eventFilter(object, event);
}

void SliderWidget::resizeEvent(QResizeEvent* resizeEvent) {
    const int labelSpinboxWidth = sliderLabel.width() + sliderSpinbox.minimumWidth();
    const int sliderWidth = width() - labelSpinboxWidth;
    
    if (/*250*/100 < sliderWidth) {
        slider.setVisible(true);
    } else {
        slider.setVisible(false);
    }

    QWidget::resizeEvent(resizeEvent);
}

void SliderWidget::paintEvent(QPaintEvent* event) {
    QStyleOption styleOption;
    styleOption.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOption, &painter, this);
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

        QRect customGroove = grooveRect.adjusted(1,1,-1,-1);

        const QPoint& mousePosition = widget->cursor().pos();
        const QPoint widgetSpaceMousePosition = widget->mapFromGlobal(mousePosition);

        QLinearGradient linearBackgroundGradient(QPoint(0, 0), widgetSpaceMousePosition);
        linearBackgroundGradient.setColorAt(0.0, QColor(100,100,100));
        linearBackgroundGradient.setColorAt(1.0, QColor(50, 50, 50));

        painter->setPen(Qt::NoPen);
        painter->setBrush(linearBackgroundGradient);
        painter->drawRoundedRect(customGroove, 2.0, 2.0);

        QRect filledRect = grooveRect;
        filledRect.setRight(handleRect.center().x());

        const int filledPercentage = ((sliderOption->sliderValue - sliderOption->minimum) * 100) / (sliderOption->maximum - sliderOption->minimum);
        QColor gradientStartColor;
        QColor finishedColor;
        QColor midColor;

        if (filledPercentage > 95) {
            finishedColor = Qt::red;
        } else if (filledPercentage > 75) {
            const QColor colorStart(Qt::lightGray);
            const QColor colorEnd(Qt::red);

            const float filledPercentageFloat = (filledPercentage - 75) * 4.0f / 100.0f;
            const float redChannel = colorStart.red() + (colorEnd.red() - colorStart.red()) * filledPercentageFloat;
            const float greenChannel = colorStart.green() + (colorEnd.green() - colorStart.green()) * filledPercentageFloat;
            const float blueChannel = colorStart.blue() + (colorEnd.blue() - colorStart.blue()) * filledPercentageFloat;
            finishedColor = QColor(redChannel, greenChannel, blueChannel);
        } else {
            const QColor colorStart(Qt::blue);
            const QColor colorEnd(Qt::lightGray);

            const float filledPercentageFloat = filledPercentage / 100.0f * (1.0f / 0.75f);
            const float redChannel = colorStart.red() + (colorEnd.red() - colorStart.red()) * filledPercentageFloat;
            const float greenChannel = colorStart.green() + (colorEnd.green() - colorStart.green()) * filledPercentageFloat;
            const float blueChannel = colorStart.blue() + (colorEnd.blue() - colorStart.blue()) * filledPercentageFloat;
            finishedColor = QColor(redChannel, greenChannel, blueChannel);
        }

        QRadialGradient radialGradient(widgetSpaceMousePosition, 50);
        //radialGradient.setColorAt(0.0, Qt::white);
        //radialGradient.setColorAt(1.0, finishedColor);

        radialGradient.setColorAt(0.0, QColor("#E0F7FF"));
        radialGradient.setColorAt(0.5, QColor("#00D2FF"));
        radialGradient.setColorAt(1.0, QColor("#0A1128"));

        //radialGradient.setColorAt(0.0, QColor("#FFFFFF"));
        //radialGradient.setColorAt(0.5, QColor("#E0E0E0"));
        //radialGradient.setColorAt(1.0, QColor("#1A1A1A"));

        //radialGradient.setColorAt(0.0, QColor("#FFE5E5"));
        //radialGradient.setColorAt(0.5, QColor("#FF003C"));
        //radialGradient.setColorAt(1.0, QColor("#1F0006"));

        painter->setBrush(radialGradient);
        painter->drawRoundedRect(filledRect, 4.0, 4.0);

        QRect customGrooveFrame = grooveRect.adjusted(1,1,-1,-1);

        QRadialGradient radialFrameGradient(widgetSpaceMousePosition, 100);
        radialFrameGradient.setColorAt(0.0, Qt::white);
        radialFrameGradient.setColorAt(1.0, Qt::lightGray);

        //QLinearGradient linearFrameGradient(QPoint(0, 0), widgetSpaceMousePosition);
        //linearFrameGradient.setColorAt(0.0, Qt::white);
        //linearFrameGradient.setColorAt(1.0, Qt::lightGray);

        painter->setBrush(Qt::NoBrush);
        QPen gradientPen;
        gradientPen.setBrush(radialFrameGradient);
        //gradientPen.setBrush(linearFrameGradient);
        gradientPen.setWidth(2);
        painter->setPen(gradientPen);
        painter->drawRoundedRect(customGrooveFrame, 2.0, 2.0);

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

        if (subcontrol == CC_Slider || subcontrol == SC_SliderGroove) {
            if (sliderOption->orientation == Qt::Horizontal) {
                QRect newRect = QProxyStyle::subControlRect(control, option, subcontrol, widget);
                const int availableHeight = sliderOption->rect.height();
                newRect.setY(0);
                newRect.setHeight(availableHeight);

                return newRect;
            }
        } else if (subcontrol == SC_SliderHandle) {
            const int sliderPosition = sliderPositionFromValue(sliderOption->minimum, sliderOption->maximum, sliderOption->sliderValue, sliderOption->rect.width() - handleWidth);
            return QRect(sliderPosition, 0, handleWidth, sliderOption->rect.height());
        }
    }

    return QProxyStyle::subControlRect(control, option, subcontrol, widget);
}

BrushPreviewWidget::BrushPreviewWidget(QWidget* parent) :
QWidget(parent),
BrushProperties{ 0.0f,1.0f,Qt::white },
BrushImage(128,128, QImage::Format_RGBA8888_Premultiplied),
mainLayout(this),
brushPreview(this),
fileLabel("None", this),
fileImage(QImage()) {
    setAcceptDrops(true);

    BrushImage = calculateBrushPreview();
    brushPreview.setPixmap(QPixmap::fromImage(BrushImage));

    mainLayout.addWidget(&brushPreview);

    fileLabel.setReadOnly(true);
    fileLabel.setClearButtonEnabled(true);
    fileLabel.installEventFilter(this);
    mainLayout.addWidget(&fileLabel);

    if (clearButton = fileLabel.findChild<QToolButton*>()) {
        printf("clearButton found\n");
        clearButton->installEventFilter(this);
    }
}

BrushPreviewWidget::~BrushPreviewWidget() {

}

void BrushPreviewWidget::UpdatePreview() {
    BrushImage = calculateBrushPreview();
    brushPreview.setPixmap(QPixmap::fromImage(BrushImage));
}

void BrushPreviewWidget::ClearPreview() {
    fileImage = QImage();
}

bool BrushPreviewWidget::eventFilter(QObject* watched, QEvent* event) {
    const QEvent::Type& type = event->type();

    if (QLineEdit* const lineEdit = qobject_cast<QLineEdit*>(watched)) {
        if (type == QEvent::MouseButtonPress) {
            // open file dialog
            printf("mouse button press\n");
        } else if (type == QEvent::DragEnter) {
            event->accept();
            return true;
        } else if (type == QEvent::Drop) {
            //lineEdit->setText("file added");

            if (QDropEvent* const dropEvent = static_cast<QDropEvent*>(event)) {
                printf("dropEvent\n");
                if (dropEvent->mimeData()->hasImage()) {
                    printf("image file\n");
                }
                if (dropEvent->mimeData()->hasUrls()) {
                    printf("url founds\n");
                    for (const QUrl& fileUrl : dropEvent->mimeData()->urls()) {
                        printf("url: %s\n", fileUrl.toString().toStdString().c_str());
                        QString localFileUrl;
#if defined(__EMSCRIPTEN__)
                        localFileUrl = QString("/qt/tmp/%1").arg(fileUrl.fileName());
#else
                        localFileUrl = fileUrl.toLocalFile();
#endif
                        const QByteArray format = QImageReader::imageFormat(localFileUrl);
                        if (!format.isEmpty()) {
                            fileImage = QImage(localFileUrl);
                            fileImage = fileImage.convertToFormat(QImage::Format_RGBA8888_Premultiplied);

                            lineEdit->setText(localFileUrl);
                            UpdatePreview();
                            break;
                        }
                    }
                }
            }

            event->accept();
            return true;
        }
    }

    if (watched == clearButton) {
        if (type == QEvent::MouseButtonRelease) {
            if (QMouseEvent* const mouseEvent = static_cast<QMouseEvent*>(event)) {
                printf("mouseButtonRelease\n");
                if (mouseEvent->button() == Qt::LeftButton) {
                    printf("clear text\n");
                    fileLabel.setText("None");
                    ClearPreview();
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

QImage BrushPreviewWidget::calculateBrushPreview() {
    const int previewSize = 128;
    QImage brushPreview(previewSize, previewSize, QImage::Format_RGBA8888_Premultiplied);
    brushPreview.fill(Qt::transparent);
    QPainter painter(&brushPreview);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    const int brushPreviewSize = 128;
    if (fileImage.isNull()) {
        painter.setBrush(BrushProperties.Color);
        painter.drawEllipse(brushPreviewSize / 2 * (1.0f - BrushProperties.Size), brushPreviewSize / 2 * (1.0f - BrushProperties.Size), brushPreviewSize * BrushProperties.Size, brushPreviewSize * BrushProperties.Size);
    } else {
        painter.setBrush(Qt::NoBrush);
        QImage customFile = fileImage.scaled(brushPreviewSize * BrushProperties.Size, brushPreviewSize * BrushProperties.Size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        const int targetX = brushPreviewSize / 2 * (1.0f - BrushProperties.Size);
        const int targetY = brushPreviewSize / 2 * (1.0f - BrushProperties.Size);
        painter.drawImage(targetX, targetY, customFile);
        painter.setCompositionMode(QPainter::CompositionMode_Multiply);
        painter.fillRect(targetX, targetY, customFile.width(), customFile.height(), BrushProperties.Color);
    }
    const QSize smallSize(brushPreviewSize * (1.01f - BrushProperties.Smoothness), brushPreviewSize * (1.01f - BrushProperties.Smoothness));
    const QImage smallImage = brushPreview.scaled(smallSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return smallImage.scaled(previewSize, previewSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

BrushWidget::BrushWidget(QWidget* parent) :
QWidget(parent),
mainLayout(this),
scrollArea(this),
scrollAreaWrapperWidget(&scrollArea),
scrollAreaWrapperWidgetLayout(&scrollAreaWrapperWidget),
brushPreviewWidget(this),
colorPickerWidget("Color", ColorPicker::GetInstance(), this),
smoothnessSlider("Smoothness", this),
sizeSlider("Size", this, 0.0, 1.0, 1.0) {
    setObjectName("BrushWidget");

    scrollAreaWrapperWidgetLayout.setSpacing(0);

    const ColorPicker& colorPicker = ColorPicker::GetInstance();

    const QImage brushPreviewImage = brushPreviewWidget.BrushImage;

    scrollAreaWrapperWidgetLayout.addWidget(&brushPreviewWidget);
    connect(&colorPicker, &ColorPicker::colorChanged, this, &BrushWidget::onColorChanged);
    scrollAreaWrapperWidgetLayout.addWidget(&colorPickerWidget);

    connect(&smoothnessSlider, &SliderWidget::changedValue, this, &BrushWidget::onValueChanged);
    scrollAreaWrapperWidgetLayout.addWidget(&smoothnessSlider);

    connect(&sizeSlider, &SliderWidget::changedValue, this, &BrushWidget::onSizeChanged);
    scrollAreaWrapperWidgetLayout.addWidget(&sizeSlider);

    constexpr int widgetsCount = 3;
    QWidget* widgets[widgetsCount] = { &colorPickerWidget, &smoothnessSlider, &sizeSlider };
    SliderWidget::GetLongestNameplateWidth(widgets, widgetsCount);
    scrollAreaWrapperWidgetLayout.addStretch();

    scrollArea.verticalScrollBar()->setObjectName("BrushWidgetVerticalScrollBar");
    scrollArea.setWidgetResizable(true);
    scrollArea.setWidget(&scrollAreaWrapperWidget);
    mainLayout.addWidget(&scrollArea);
}

void BrushWidget::InitializeBrushImage() {
    emit BrushChanged(brushPreviewWidget.BrushImage);
}

void BrushWidget::onValueChanged(double value, double maximum){
    printf("onSmoothnessChanged\n");
    brushPreviewWidget.BrushProperties.Smoothness = value;
    brushPreviewWidget.UpdatePreview();
    const QImage previewImage = brushPreviewWidget.BrushImage;

    emit BrushChanged(previewImage);
}

void BrushWidget::onColorChanged(QColor newColor){
    colorPickerWidget.SetPickerColor(newColor);
    brushPreviewWidget.BrushProperties.Color = newColor;
    brushPreviewWidget.UpdatePreview();
    QImage previewImage = brushPreviewWidget.BrushImage;

    emit BrushChanged(previewImage);
}

void BrushWidget::onSizeChanged(double value, double maximum) {
    printf("onSizeChanged\n");
    brushPreviewWidget.BrushProperties.Size = value;
    brushPreviewWidget.UpdatePreview();
    const QImage previewImage = brushPreviewWidget.BrushImage;

    emit BrushChanged(previewImage);
}
