#include <config.h>

#include "PrevPainter.hpp"
#include "prevappgraphicwidget.hpp"
#include "StelViewportDistorter.hpp"
#include "StelCore.hpp"
#include "PrevApp.hpp"

#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGLFramebufferObject>
#include <QSettings>

PrevAppGraphicWidget* PrevAppGraphicWidget::singleton = NULL;

PrevAppGraphicWidget::PrevAppGraphicWidget(): paintState(0), viewportEffect(NULL)
{
    viewportEffect = StelViewportDistorter::create("none",800,600,StelProjectorP());

    prevApp = new PrevApp();

    Q_ASSERT(!singleton);
    singleton = this;
}

PrevAppGraphicWidget::~PrevAppGraphicWidget()
{
    delete prevApp;

    if (viewportEffect)
        delete viewportEffect;
}
void PrevAppGraphicWidget::init(QSettings* conf)
{
    prevApp->init(conf);
    Q_ASSERT(viewportEffect==NULL);

    if (viewportEffect)
    {
        delete viewportEffect;
        viewportEffect = NULL;
    }

    viewportEffect = StelViewportDistorter::create("none",(int)size().width(),(int)size().height(),PrevApp::getInstance().getCore()->getProjection2d());

}

bool PrevAppGraphicWidget::paintPartial()
{
    if (paintState == 0)
    {
        // Update the core and all modules

        paintState = 1;
        return true;
    }
    if (paintState == 1)
    {
        // And draw them
        //viewportEffect->prepare();
        if (prevApp->drawPartial())
        {
            //viewportEffect->distort();
            return true;
        }

        paintState = 0;
        return false;
    }
    Q_ASSERT(false);
    return false;
}

void PrevAppGraphicWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (!prevApp || !prevApp->getCore())
        return;

    if (painter->paintEngine()->type() != QPaintEngine::OpenGL && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("PrevAppGraphicWidget: paint needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }

    PrevPainter::setQPainter(painter);
    //
    //    if (useBuffers)
    //    {
    //        //            stelApp->makeMainGLContextCurrent();
    //        //            initBuffers();
    //        //            backgroundBuffer->bind();
    //        //            QPainter* pa = new QPainter(backgroundBuffer);
    //        //            StelPainter::setQPainter(pa);
    //        //
    //        //            // If we are using the gui, then we try to have the best reactivity, even if we need to lower the fps for that.
    //        //            int minFps = StelApp::getInstance().getGui()->isCurrentlyUsed() ? 16 : 2;
    //        //            while (true)
    //        //            {
    //        //                bool keep = paintPartial();
    //        //                if (!keep) // The paint is done
    //        //                {
    //        //                    delete pa;
    //        //                    backgroundBuffer->release();
    //        //                    //if(!StelApp::getInstance().isVideoMode)
    //        //                    swapBuffers();
    //        //                    break;
    //        //                }
    //        //                double spentTime = StelApp::getTotalRunTime() - previousPaintFrameTime;
    //        //                if (1. / spentTime <= minFps) // we spent too much time
    //        //                {
    //        //                    // We stop the painting operation for now
    //        //                    delete pa;
    //        //                    backgroundBuffer->release();
    //        //                    break;
    //        //                }
    //        //            }
    //        //
    //        //            Q_ASSERT(!backgroundBuffer->isBound());
    //        //            Q_ASSERT(!foregroundBuffer->isBound());
    //        //            // Paint the last completed painted buffer
    //        //            StelPainter::setQPainter(painter);
    //        //            viewportEffect->distort();//paintViewportBuffer(foregroundBuffer);
    //    }
    //    else
    {
        while (paintPartial()) {;}
        // viewportEffect->distort();
    }

    PrevPainter::setQPainter(NULL);
}

void PrevAppGraphicWidget::swapBuffers()
{
    //    Q_ASSERT(useBuffers);
    //    QGLFramebufferObject* tmp = backgroundBuffer;
    //    backgroundBuffer = foregroundBuffer;
    //    foregroundBuffer = tmp;

}

void PrevAppGraphicWidget::initBuffers()
{
    Q_ASSERT(useBuffers);
    Q_ASSERT(QGLFramebufferObject::hasOpenGLFramebufferObjects());
    //    if (!backgroundBuffer)
    //    {
    //        backgroundBuffer = new QGLFramebufferObject(scene()->sceneRect().size().toSize(), QGLFramebufferObject::CombinedDepthStencil);
    //        foregroundBuffer = new QGLFramebufferObject(scene()->sceneRect().size().toSize(), QGLFramebufferObject::CombinedDepthStencil);
    //        Q_ASSERT(backgroundBuffer->isValid());
    //        Q_ASSERT(foregroundBuffer->isValid());
    //    }
}

void PrevAppGraphicWidget::keyPressEvent(QKeyEvent* event)
{
    prevApp->handleKeys(event);
}

void PrevAppGraphicWidget::keyReleaseEvent(QKeyEvent* event)
{
    prevApp->handleKeys(event);
}

