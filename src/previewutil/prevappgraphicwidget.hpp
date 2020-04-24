#ifndef PREVAPPGRAPHICWIDGET_HPP
#define PREVAPPGRAPHICWIDGET_HPP

#include <QGraphicsWidget>

#include "StelProjector.hpp"
#include "PrevApp.hpp"

class PrevAppGraphicWidget : public QGraphicsWidget
{
    Q_OBJECT
public:
    PrevAppGraphicWidget();
    ~PrevAppGraphicWidget();

    void init(class QSettings* conf);

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget=0);

    static PrevAppGraphicWidget& getInstance() {Q_ASSERT(singleton); return *singleton;}
    StelProjector::StelProjectorParams params;
protected:
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

private:

    class PrevApp* prevApp;

    int paintState;

    void initBuffers();

    void swapBuffers();

    bool paintPartial();

    class StelViewportDistorter* viewportEffect;
    static PrevAppGraphicWidget* singleton;

};

#endif // PREVAPPGRAPHICWIDGET_HPP
