#include "dockwidget.h"

#include <QCloseEvent>

DockWidget::DockWidget(QWidget* const parent) :
QDockWidget(parent)
{
    printf("DockWidget\n");
}

void DockWidget::closeEvent(QCloseEvent* closeEvent){
    printf("closeEvent\n");
    closeEvent->ignore();

    Qt::WindowFlags flags = windowFlags();

    if(!isFloating()){
        flags &= ~Qt::WindowCloseButtonHint;
        show();
        return;
    }

    if(isFloating()){
        flags |= Qt::WindowCloseButtonHint;
        show();
        setFloating(false);
    }
}
