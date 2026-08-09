#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QImage>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QScrollArea>
#include <QProxyStyle>
#if defined(__EMSCRIPTEN__)
#include <QOpenGLFunctions>
#else
#include <QOpenGLFunctions_3_0>
#endif

// SliderWidget holds a name label, qslider and qspinbox in a horizontal layout
class SliderWidget : public QWidget{
    Q_OBJECT
public:
    // SliderProxyStyle alters the style how the slider is being rendered
    class SliderProxyStyle : public QProxyStyle {
    public:
        SliderProxyStyle(QStyle* style = nullptr);
        ~SliderProxyStyle();

        void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option, QPainter* painter, const QWidget* widget = nullptr) const override;
        QRect subControlRect(ComplexControl control, const QStyleOptionComplex* option, SubControl subcontrol, const QWidget* widget = nullptr) const override;
    };

    explicit SliderWidget(QString text, QWidget* parent = nullptr, double minimum = 0.0, double maximum = 1.0, double value = 0.0);
    ~SliderWidget();

    inline double GetValue();

    void OnSliderValueChanged(int sliderValue);
    void OnSpinboxValueChanged(double spinboxValue);

    // calculates the longest nameplate that all slider in the widets array have the same space for the nameplate
    static void GetLongestNameplateWidth(QWidget* widgets[], const int size);

signals:
    void changedValue(double value, double maximum);

protected:
    // used to make the qspinbox value change on drag and to render the gradient at the mouse in the slider
    bool eventFilter(QObject* object, QEvent* event) override;
    void resizeEvent(QResizeEvent* resizeEvent) override;

    virtual void paintEvent(QPaintEvent* event) override;

    QBoxLayout mainLayout;
    QLabel sliderLabel;
    SliderProxyStyle sliderProxyStyle;
    QSlider slider;
    QDoubleSpinBox sliderSpinbox;

    double minimum;
    double maximum;
    double value;

    // struct to hold data about the drag on the spinbox
    struct {
        class QLineEdit* lineEdit;
        bool mouseDown;
        QPoint mousePosition;
    } spinboxDrag;
};

// ColorPicker class shows a dialog with a color wheel image and sliders to change the color of the brush
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

    // sets the selected color based on the mouse click in the color wheel image
    bool eventFilter(QObject* object, QEvent* event);

    // slider interaction
    void onRedColorChanged(double value, double maximum);
    void onGreenColorChanged(double value, double maximum);
    void onBlueColorChanged(double value, double maximum);
    void onBrightnessChanged(double value, double maximum);

    QVBoxLayout mainLayout;

    QImage colorPaletteImage;               // the image shown in the color picker to pick a color
    QLabel colorPaletteWrapper;             // holds the colorPaletteImage
    QWidget colorSliderWrapper;             // holds the red, green and blue sliders
    QVBoxLayout colorSliderWrapperLayout;   // the layout for the image and the slider wrapper
    SliderWidget redSlider;
    SliderWidget greenSlider;
    SliderWidget blueSlider;
    SliderWidget brightnessSlider;
    QPushButton closeButton;                // close button that shows the selected color
};

// ColorPickerWidget is added the brush widget list and opens the ColorPicker on click
class ColorPickerWidget : public QWidget{
    Q_OBJECT
public:
    explicit ColorPickerWidget(QString text, ColorPicker& colorPicker, QWidget* parent = nullptr);
    virtual ~ColorPickerWidget(){}

    inline QColor GetPickerColor();
    void SetPickerColor(const QColor newColor);
protected:
    virtual void paintEvent(QPaintEvent* event) override;

    QHBoxLayout mainLayout;
    QLabel colorPickerLabel;
    QPushButton colorPickerColor;
};

// Shows a preview of the brush
class BrushPreviewWidget : public QWidget {
public:
    BrushPreviewWidget(QWidget* parent = nullptr);
    ~BrushPreviewWidget();

    void UpdatePreview(); // recalculates and sets the image in the label
    void ClearPreview();  // clears the file that was used for the brush, when clicking the x button for example

    struct {
        double Smoothness; // the strength of a blur effect, by scaling the iamge smaller and then scaling it back up
        double Size;       // the size of the image
        QColor Color;      // the color that is being multiplied to the image
    } BrushProperties;

    QImage BrushImage;  // the image that is used, can be changed by dropping a file into the file label

protected:
    // used for loading image files that were dropped into the file label and clearing the image file when the x button was pressed
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    QImage calculateBrushPreview(); // recalculates the brush preview based on the current brush properties

    QHBoxLayout mainLayout;
    QLabel brushPreview;        // shows an image of the brush
    QLineEdit fileLabel;        // shows empty or the name of the image file currently loaded
    QToolButton* clearButton;   // the button in the file label to clear the image file
    QImage fileImage;           // the file image that is used or empty
};

// BrushWidget shows a preview of the brush on top and has a list that gives option to change its appearance
class BrushWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BrushWidget(QWidget* parent = nullptr);
    virtual ~BrushWidget(){}

    void InitializeBrushImage();

signals:
    void BrushChanged(const QImage& brushTexture);

protected:
    void onValueChanged(double value, double maximum);
    void onColorChanged(QColor newColor);
    void onSizeChanged(double value, double maximum);

    QVBoxLayout mainLayout;
    QScrollArea scrollArea;
    QWidget scrollAreaWrapperWidget;
    QVBoxLayout scrollAreaWrapperWidgetLayout;
    BrushPreviewWidget brushPreviewWidget;
    QHBoxLayout brushPreviewWidgetLayout;
    ColorPickerWidget colorPickerWidget;
    SliderWidget smoothnessSlider;
    SliderWidget sizeSlider;
};

#endif // BRUSHWIDGET_H
