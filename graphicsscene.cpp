#include "graphicsscene.h"

#include <QPainter>

GraphicsScene::GraphicsScene(QObject* const parent) :
QGraphicsScene(parent)
{
    setSceneRect(0,0,512,512);
}

void GraphicsScene::drawBackground(QPainter* painter, const QRectF& rect){
    printf("GraphicsScene::drawBackground\n");
    QPen gridPen(Qt::lightGray);
    gridPen.setWidth(0);
    painter->setPen(gridPen);
    painter->save();
    painter->resetTransform();

    for(int count = 0; count < 10; count++){
        painter->drawLine(0,count/10.0*rect.height(),rect.width(),count/10.0*rect.height());
        painter->drawLine(count/10.0*rect.width(),0,count/10.0*rect.width(),rect.height());
    }

    painter->setPen(Qt::black);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->drawText(20,20,"Graphics View");
    painter->restore();
}

void GraphicsScene::drawForeground(QPainter* painter, const QRectF& rect){

}
