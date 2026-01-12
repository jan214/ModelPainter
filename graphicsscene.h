#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>

class GraphicsScene : public QGraphicsScene{
public:
    explicit GraphicsScene(QObject* const parent = nullptr);
    ~GraphicsScene(){}

protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void drawForeground(QPainter* painter, const QRectF& rect) override;
};

#endif // GRAPHICSSCENE_H
