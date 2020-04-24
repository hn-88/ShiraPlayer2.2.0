#ifndef OPENGLSCENE_HPP
#define OPENGLSCENE_HPP

#include <QGraphicsScene>

class OpenGLScene : public QGraphicsScene
{
Q_OBJECT
public:
    OpenGLScene();
    void drawBackground(QPainter *painter, const QRectF &rect);

signals:

public slots:

};

#endif // OPENGLSCENE_HPP
