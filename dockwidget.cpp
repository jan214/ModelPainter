#include "dockwidget.h"

#include <QCloseEvent>

DockWidget::DockWidget(QWidget* const parent) :
QDockWidget(parent)
{
    printf("DockWidget\n");
#if defined(__EMSCRIPTEN__)
    setFeatures(QDockWidget::DockWidgetClosable);
#endif
}

void DockWidget::closeEvent(QCloseEvent* closeEvent) {
    printf("closeEvent\n");
    setVisible(false);

    closeEvent->accept();
}
