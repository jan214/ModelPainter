#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>

class DockWidget : public QDockWidget{
public:
    explicit DockWidget(QWidget* const parent = nullptr);
    ~DockWidget(){}

protected:
    virtual void closeEvent(QCloseEvent* closeEvent) override;
};

#endif // DOCKWIDGET_H
