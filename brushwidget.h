#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QImage>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QScrollArea>
#include <QLineEdit>
#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif

class SliderWidget : public QWidget{
    Q_OBJECT
public:
    explicit SliderWidget(QString text, QWidget* parent = nullptr, double minimum = 0.0, double maximum = 1.0, double value = 0.0);
    ~SliderWidget();

    void OnSliderValueChanged(int sliderValue);
    void OnSpinboxValueChanged(double spinboxValue);

    static void GetLongestNameplateWidth(QWidget* widgets[], const int size);

signals:
    void changedValue(double value, double maximum);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void resizeEvent(QResizeEvent* resizeEvent) override;

    QHBoxLayout mainLayout;
    QLabel sliderLabel;
    QSlider slider;
    QDoubleSpinBox sliderSpinbox;

    double minimum;
    double maximum;
    double value;

    struct {
        QLineEdit* lineEdit;
        bool mouseDown;
        QPoint mousePosition;
    } spinboxDrag;
};

class ColorPicker : public QDialog{
    Q_OBJECT
public:
    static ColorPicker& GetInstance() {
        static ColorPicker instance;
        return instance;
    }

    ColorPicker(const ColorPicker&) = delete;
    ColorPicker& operator=(const ColorPicker&) = delete;
signals:
    void colorChanged(QColor newColor);

protected:
    explicit ColorPicker(QWidget* parent = nullptr);
    ~ColorPicker() {}

    bool eventFilter(QObject* object, QEvent* event);

    QVBoxLayout mainLayout;

    QImage colorPaletteImage;
    QLabel colorPaletteWrapper;
    QWidget colorSliderWrapper;
    QVBoxLayout colorSliderWrapperLayout;
    SliderWidget redSlider;
    SliderWidget greenSlider;
    SliderWidget blueSlider;
    QPushButton closeButton;
};

class ColorPickerWidget : public QWidget{
    Q_OBJECT
public:
    explicit ColorPickerWidget(QString text, ColorPicker& colorPicker, QWidget* parent = nullptr);
    ~ColorPickerWidget(){}

    inline QColor GetPickerColor();
    void SetPickerColor(const QColor newColor);
protected:
    QHBoxLayout mainLayout;
    QLabel colorPickerLabel;
    QPushButton colorPickerColor;
};

class BrushWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushWidget(QWidget* parent = nullptr);
    ~BrushWidget(){}

signals:
    void BrushChanged(const QImage& brushTexture);

protected:
    void onValueChanged(double value, double maximum);
    void onColorChanged(QColor newColor);

    QVBoxLayout mainLayout;
    QScrollArea scrollArea;
    QWidget scrollAreaWrapperWidget;
    QVBoxLayout scrollAreaWrapperWidgetLayout;
    QImage brushPreview;
    QLabel brushPreviewWrapper;
    ColorPickerWidget colorPickerWidget;
    SliderWidget smoothnessSlider;
};

#endif // BRUSHWIDGET_H
