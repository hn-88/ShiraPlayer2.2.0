/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Fabien Chereau
 * Copyright (C) 2011 Asaf Yurdakul
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ShiraPlayer is a trademark of Sureyyasoft.
 */

#include <config.h>

#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelAppGraphicsWidget.hpp"
#include "StelPainter.hpp"
#include "StelGuiBase.hpp"
//#include "StelViewportEffect.hpp"
#include "StelViewportDistorter.hpp"

#include <QPainter>
#include <QPaintEngine>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QtOpenGL/QGLFramebufferObject>
#include <QSettings>

#include "StelMainGraphicsView.hpp"
#include "StelMovementMgr.hpp"
#include "StelNavigator.hpp"
#include "StelMainWindow.hpp"
#include "freehand/tupgraphicalgorithm.h"
#include <QtXml/QtXml>
#include <stdexcept>

//#include "glrc/include/glrc.h"
#include "multiprojector/channelhak.h"

#include "shiraprojector.h"

StelAppGraphicsWidget* StelAppGraphicsWidget::singleton = NULL;

StelAppGraphicsWidget::StelAppGraphicsWidget()
    : paintState(0), useBuffers(false), backgroundBuffer(0), foregroundBuffer(0), viewportEffect(NULL)
{

    if (StelMainWindow::getInstance().getIsServer())
    {
        if (StelMainWindow::getInstance().projdll->getUseBuffer() && StelMainWindow::getInstance().projdll->isEnabled())
        {
            StelMainWindow::getInstance().projdll->getPluginName();
            useBuffers = true;
        }
    }


    //    if(useBuffers)
    //    {
    //        if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
    //            qWarning() << "This system has no framebuffer object support";
    //        }
    //    }

    viewportEffect = StelViewportDistorter::create("none",800,600,StelProjectorP());

    previousPaintTime = StelApp::getTotalRunTime();
    setFocusPolicy(Qt::StrongFocus);
    stelApp = new StelApp();

    //ASAF
    Q_ASSERT(!singleton);
    singleton = this;

    pressed = false;

    backgroundPrvBuffer = 0;
    foregroundPrvBuffer = 0;

    changeLocByMouse = false;
    isDrawingPreview = false;
}

StelAppGraphicsWidget::~StelAppGraphicsWidget()
{
    delete stelApp;
    if (backgroundBuffer)
        delete backgroundBuffer;
    if (foregroundBuffer)
        delete foregroundBuffer;

    if (backgroundPrvBuffer)
        delete backgroundPrvBuffer;
    if (foregroundPrvBuffer)
        delete foregroundPrvBuffer;

    if (viewportEffect)
        delete viewportEffect;
}

void StelAppGraphicsWidget::init(QSettings* conf)
{
    stelApp->init(conf);
    Q_ASSERT(viewportEffect==NULL);
    if(StelMainWindow::getInstance().getIsServer())
        setViewportEffect("none");
    else
    {
        setViewportEffect("none");
        setViewportEffect(conf->value("video/viewport_effect", "none").toString());
        StelApp::getInstance().getCore()->currentProjectorParams.gravityLabels = StelApp::getInstance().confSettings->value("viewing/flag_gravity_labels").toBool();
    }

    //previousPaintTime needs to be updated after the time zone is set
    //in StelLocaleMgr::init(), otherwise this causes an invalid value of
    //deltaT the first time it is calculated in paintPartial(), which in
    //turn causes Stellarium to start with a wrong time.
    previousPaintTime = StelApp::getTotalRunTime();

    //#ifndef HAKONIWA
    //    if(useBuffers && !StelMainWindow::getInstance().getIsServer())
    //    {
    //        //viewportRes = conf->value("projection/viewport_res",2048 ).toDouble(); // Default deger 2048 kabul edildi.
    //        QString strPCName = conf->value("main/PC_Name","PC1").toString();

    //        /* initialize glrc lirbary */
    //        glrcInit();

    //        /* create a compositor */
    //        glrcGenCompositors(1, &compositorIndex);
    //        glrcBindCompositor(GLRC_COMPOSITOR_2D, compositorIndex);
    //        glrcSetupBuffer(GLRC_RTT_AUTO,
    //                        StelApp::getInstance().viewportRes,//this->geometry().width(),
    //                        StelApp::getInstance().viewportRes,//this->geometry().height(),
    //                        FALSE, FALSE);

    //        /* setup distortion and blendings */
    //        glrcSetDistortionMap(QString("%0").arg("DataSet/distort_"+strPCName+".png").toLatin1());
    //        glrcSetBlendMap(QString("%0").arg("DataSet/blend_"+strPCName+".png").toLatin1());
    //        glrcLoadMatrixFile(QString("%0").arg("DataSet/view_"+strPCName+".cfg").toLatin1());
    //        glrcSetEffectMode(GLRC_EFFECT_DISTORTION);
    //        //glrcDisable(GLRC_MESHDISTORTION);
    //        glrcEnable(GLRC_MESHDISTORTION);

    //    }
    //#endif


    if (StelMainWindow::getInstance().getIsServer())
    {
        if (StelMainWindow::getInstance().projdll->getPluginName()=="Multiprojector")
        {
            bool showprojectors = StelMainWindow::getInstance().projdll->isEnabled();
            StelMainWindow::getInstance().projdll->LoadProjectors(StelMainGraphicsView::getInstance().getOpenGLWin(),showprojectors);
            //StelMainWindow::getInstance().setIsMultiprojector(true);
            StelApp::getInstance().viewportRes = StelMainWindow::getInstance().projdll->getViewportRes();
        }
    }

}


void StelAppGraphicsWidget::setViewportEffect(const QString& name)
{
    /*if (viewportEffect)
    {
                if (viewportEffect->getType()==name)
            return;
        delete viewportEffect;
        viewportEffect=NULL;
    }
    if (name=="none")
    {
        useBuffers = false;
        return;
    }
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        qWarning() << "Don't support OpenGL framebuffer objects, can't use Viewport effect: " << name;
        useBuffers = false;
        return;
    }

    qDebug() << "Use OpenGL framebuffer objects for viewport effect: " << name;
    useBuffers = true;
    if (name == "framebufferOnly")
    {
                viewportEffect = new StelViewportDistorter();
    }
    else if (name == "sphericMirrorDistorter")
    {
                viewportEffect = new StelViewportDistorterFisheyeToSphericMirror(size().width(), size().height());
    }
    else
    {
        qWarning() << "Unknown viewport effect name: " << name;
        useBuffers=false;
        }*/


    if (name != getViewportEffect())
    {
        //        if (name == "none")
        //        {
        //            StelMainGraphicsView::getInstance().getOpenGLWin()->setMaximumSize((int)(size().width()),(int)(size().height()));
        //        }
        //        else if (name == "sphericMirrorDistorter")
        //        {

        //            StelMainGraphicsView::getInstance().getOpenGLWin()->setFixedSize((int)(size().width()), (int)(size().height()));
        //        }
        StelMainGraphicsView::getInstance().getOpenGLWin()->setFixedSize((int)(size().width()), (int)(size().height()));
    }

    if (viewportEffect)
    {
        delete viewportEffect;
        viewportEffect = NULL;
    }

    viewportEffect = StelViewportDistorter::create(name,(int)size().width(),(int)size().height(),StelApp::getInstance().getCore()->getProjection2d());
}

QString StelAppGraphicsWidget::getViewportEffect() const
{
    if (viewportEffect)
        return viewportEffect->getType();
    return "none";
}

void StelAppGraphicsWidget::distortPos(QPointF* pos)
{
    if (!viewportEffect)
        return;
    int x = pos->x();
    int y = pos->y();
    y = size().height() - 1 - y;
    viewportEffect->distortXY(x,y);
    pos->setX(x);
    pos->setY(size().height() - 1 - y);
}

//! Iterate through the drawing sequence.
bool StelAppGraphicsWidget::paintPartial()
{
    // qDebug() << "paintPartial" << paintState;
    if (paintState == 0)
    {
        //if(!stelApp->isVideoMode && !stelApp->isVideoLandscape)
        {
            const double now = StelApp::getTotalRunTime();
            double dt = now-previousPaintTime;
            previousPaintTime = now;
            if (dt<0)       // This fix the star scale bug!!
                return false;

            // Update the core and all modules
            stelApp->update(dt);
        }
        paintState = 1;
        return true;
    }
    if (paintState == 1)
    {
        // And draw them
        //viewportEffect->prepare();
        if (stelApp->drawPartial())
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

void StelAppGraphicsWidget::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // Don't even try to draw if we don't have a core yet (fix a bug during splash screen)
    if (!stelApp || !stelApp->getCore())
        return;

    StelPainter::setQPainter(painter);

    if (useBuffers)
    {

        stelApp->makeMainGLContextCurrent();
        initBuffers();
        stelApp->getCore()->windowHasBeenResized(0,0,
                                                 stelApp->viewportRes,
                                                 stelApp->viewportRes);
        if (StelMainWindow::getInstance().projdll->needFlip())
        {
            //StelApp::getInstance().core->setFlipHorz(true);
            stelApp->getCore()->setFlipVert(true);

        }
        backgroundBuffer->bind();
        QPainter* pa = new QPainter(backgroundBuffer);
        StelPainter::setQPainter(pa);

        // If we are using the gui, then we try to have the best reactivity, even if we need to lower the fps for that.
        int minFps = stelApp->getGui()->isCurrentlyUsed() ? 16 : 2;

        //SkyWriter için projektörlere çizim
        stelApp->getCore()->prevScreendrawing = false;
        //--
        while (true)
        {
            bool keep = paintPartial();
            if (!keep) // The paint is done
            {
                delete pa;
                backgroundBuffer->release();
                swapBuffers();
                break;
            }
            double spentTime = StelApp::getTotalRunTime() - previousPaintFrameTime;
            if (1. / spentTime <= minFps) // we spent too much time
            {
                // We stop the painting operation for now
                delete pa;
                backgroundBuffer->release();
                break;
            }
        }


        Q_ASSERT(!backgroundBuffer->isBound());
        Q_ASSERT(!foregroundBuffer->isBound());


        if (StelMainWindow::getInstance().projdll->needFlip())
        {
            //StelApp::getInstance().core->setFlipHorz(false);
            stelApp->getCore()->setFlipVert(false);

        }


        //---Preview ekraný için prevbuffer a çiziliyor
        //--
        stelApp->getCore()->windowHasBeenResized(0,0,
                                                 scene()->sceneRect().size().toSize().width(),
                                                 scene()->sceneRect().size().toSize().height());

        if(!stelApp->isVideoMode)
        {
            isDrawingPreview = true;
            backgroundPrvBuffer->bind();
            pa = new QPainter(backgroundPrvBuffer);
            StelPainter::setQPainter(pa);
            //SkyWriter için preview ekrana çizim
            stelApp->getCore()->prevScreendrawing = true;
            //--

            while (true)
            {
                bool keep = paintPartial();
                if (!keep) // The paint is done
                {
                    delete pa;
                    backgroundPrvBuffer->release();
                    swapPrvBuffers();
                    break;
                }
                double spentTime = StelApp::getTotalRunTime() - previousPaintFrameTime;
                if (1. / spentTime <= minFps) // we spent too much time
                {
                    // We stop the painting operation for now
                    delete pa;
                    backgroundPrvBuffer->release();
                    break;
                }
            }
            isDrawingPreview = false;
        }

        Q_ASSERT(!backgroundPrvBuffer->isBound());
        Q_ASSERT(!foregroundPrvBuffer->isBound());
        //---


        // Paint the last completed painted buffer
        StelPainter::setQPainter(painter);
        //viewportEffect->distort();

        //Multiprojector Distorter
        if (StelMainWindow::getInstance().getIsMultiprojector())
        {
            //Preview Frame Buffer ekrana çiziliyor
            //----

            StelPainter sPainter(StelApp::getInstance().getCore()->getProjection2d());
            glEnable(GL_TEXTURE_2D);
            glMatrixMode(GL_PROJECTION);        // projection matrix mode
            glLoadIdentity();
            glOrtho(0,scene()->sceneRect().size().toSize().width() ,
                    0,scene()->sceneRect().size().toSize().height(), -1, 1); // set a 2D orthographic projection
            glMatrixMode(GL_MODELVIEW);         // modelview matrix mode
            glLoadIdentity();

            glBindTexture(GL_TEXTURE_2D, foregroundPrvBuffer->texture());
            //int fishSize = std::min(scene()->sceneRect().width(),scene()->sceneRect().height());

            sPainter.drawRect2d(0,
                                0,
                                scene()->sceneRect().size().toSize().width(),
                                scene()->sceneRect().size().toSize().height());

            glDisable(GL_TEXTURE_2D);
            //-----

            //Frame Buffer projektörlere gönderiliyor
            //--

            QGLFramebufferObject* buffer = NULL;
            if (foregroundBuffer != NULL)
                buffer = foregroundBuffer;
            if (buffer == NULL)
                buffer = backgroundBuffer;

            if (buffer != NULL)
            {


                if (buffer->isValid())
                {
                    /*QImage originalImage(buffer->toImage());
                                                QImage captureImage(originalImage.constBits(),
                                                                    originalImage.width(),
                                                                    originalImage.height(),
                                                                    QImage::Format_ARGB32);
                                               captureImage.save("test.png");*/
                    StelMainWindow::getInstance().projdll->setprjTex(buffer->texture());
                }

            }
            //----

            stelApp->makeMainGLContextCurrent();
            glEnable(GL_TEXTURE_2D);
        }
        else
        {
            if( (!StelMainWindow::getInstance().getIsServer()) && StelApp::getInstance().isVideoMode )
                if (StelApp::getInstance().getCore()->showFPS)
                    StelApp::getInstance().getCore()->displayFramerate();

        }
    }
    else
    {

        while (paintPartial()) {;}
        viewportEffect->distort();
        //#ifdef SHIRAPLAYER_PRE
        if (!StelMainWindow::getInstance().getIsMultiprojector())
        {
            //stelApp->getCore()->drawDistortBlend();
            if( (!StelMainWindow::getInstance().getIsServer()) && stelApp->isVideoMode )
                if (stelApp->getCore()->showFPS)
                    stelApp->getCore()->displayFramerate();
        }
        //#endif
    }

    StelPainter::setQPainter(NULL);
    previousPaintFrameTime = StelApp::getTotalRunTime();
}

//! Swap the buffers
//! this should be called after we finish the paint
void StelAppGraphicsWidget::swapBuffers()
{
    Q_ASSERT(useBuffers);
    QGLFramebufferObject* tmp = backgroundBuffer;
    backgroundBuffer = foregroundBuffer;
    foregroundBuffer = tmp;
}

void StelAppGraphicsWidget::swapPrvBuffers()
{
    Q_ASSERT(useBuffers);
    QGLFramebufferObject* tmp = backgroundPrvBuffer;
    backgroundPrvBuffer = foregroundPrvBuffer;
    foregroundPrvBuffer = tmp;
}

//! Initialize the opengl buffer objects.
void StelAppGraphicsWidget::initBuffers()
{
    Q_ASSERT(useBuffers);
    Q_ASSERT(QGLFramebufferObject::hasOpenGLFramebufferObjects());
    if (!backgroundBuffer)
    {
        backgroundBuffer = new QGLFramebufferObject(StelApp::getInstance().viewportRes,//scene()->sceneRect().size().toSize().width(),
                                                    StelApp::getInstance().viewportRes,//scene()->sceneRect().size().toSize().height(),
                                                    QGLFramebufferObject::CombinedDepthStencil);
        foregroundBuffer = new QGLFramebufferObject(StelApp::getInstance().viewportRes,//scene()->sceneRect().size().toSize().width(),
                                                    StelApp::getInstance().viewportRes,//scene()->sceneRect().size().toSize().height(),
                                                    QGLFramebufferObject::CombinedDepthStencil);

        backgroundPrvBuffer = new QGLFramebufferObject(scene()->sceneRect().size().toSize().width(),
                                                       scene()->sceneRect().size().toSize().height(),
                                                       QGLFramebufferObject::CombinedDepthStencil);
        foregroundPrvBuffer = new QGLFramebufferObject(scene()->sceneRect().size().toSize().width(),
                                                       scene()->sceneRect().size().toSize().height(),
                                                       QGLFramebufferObject::CombinedDepthStencil);


        qDebug()<<"Buffer Width:"<<foregroundBuffer->width();
        qDebug()<<"Buffer Height:"<<foregroundBuffer->height();

        qDebug()<<"Preview Buffer Width:"<<scene()->sceneRect().size().toSize().width();
        qDebug()<<"Preview Buffer Height:"<<scene()->sceneRect().size().toSize().height();

        Q_ASSERT(backgroundBuffer->isValid());
        Q_ASSERT(foregroundBuffer->isValid());

        //glTexImage2D(GL_TEXTURE_2D, 0, 3, foregroundBuffer->size().width(),
        //             foregroundBuffer->size().height(),
        //             0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//create our image
    }
    /*#ifndef SHIRAPLAYER_PRE
   if (!StelMainWindow::getInstance().getIsServer())
   {
       StelMainGraphicsView::getInstance().setGeometry(QRect(0,0,1920,1080));
   }
#endif*/
}

void StelAppGraphicsWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (changeLocByMouse)
    {
        return;
    }

    QPointF pos = event->scenePos();
    QPointF posPrev = event->scenePos();

    if ((StelMainWindow::getInstance().getIsMultiprojector())
            && (stelApp->getAllowFreeHand()))
    {
        rateX = stelApp->viewportRes / qMin(scene()->sceneRect().width(),scene()->sceneRect().height());
        rateY = stelApp->viewportRes / qMin(scene()->sceneRect().width(),scene()->sceneRect().height());

        qreal fark = qAbs(scene()->sceneRect().width() - scene()->sceneRect().height());

        if (scene()->sceneRect().width() > scene()->sceneRect().height())
        {
            pos.setX((pos.x()- fark / 2.0)*rateX);
            pos.setY(scene()->sceneRect().height() - pos.y()*rateY);
        }
        else
        {
            pos.setX(pos.x()*rateX);
            pos.setY(scene()->sceneRect().height() - (pos.y()- fark / 2.0)*rateY);
        }

        //qDebug()<<"Move Pos:"<< pos.x()<<","<<pos.y();
    }

    if(!StelMainGraphicsView::getInstance().m_bDesignWarp)
    {
        // Apply distortion on the mouse position.
        distortPos(&pos);
        pos.setY(scene()->height() - 1 - pos.y());
        stelApp->handleMove(pos.x(), pos.y(), event->buttons());

        updateClientData();
    }
    //        else //Distortion design Mode
    //        {
    //            if (event->buttons() == Qt::LeftButton)
    //            {
    //                if (StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->drag(pos.x(), pos.y()))
    //                {
    //                    if (StelMainWindow::getInstance().getIsServer())
    //                    {
    //    #ifndef SHIRAPLAYER_PRE
    //                        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    //    #endif
    //                    }
    //                }
    //            }
    //        }

    if ( StelApp::getInstance().getAllowFreeHand() )
    {
        if (pressed)
        {
            stelApp->getCore()->path.moveTo(StelApp::getInstance().getCore()->oldPathPos);
            stelApp->getCore()->pathPrevScreen.moveTo(StelApp::getInstance().getCore()->oldPathPosPrevScreen);

            if (StelMainWindow::getInstance().getIsMultiprojector())
            {
                stelApp->getCore()->path.lineTo( pos);
                stelApp->getCore()->oldPathPos = pos;

                stelApp->getCore()->pathPrevScreen.lineTo( posPrev);
                stelApp->getCore()->oldPathPosPrevScreen = posPrev;
            }
            else
            {
                stelApp->getCore()->path.lineTo( event->scenePos());
                stelApp->getCore()->oldPathPos = event->scenePos();
            }
        }
    }

}

void StelAppGraphicsWidget::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if ( (event->modifiers() & Qt::CTRL ) && ( StelApp::getInstance().getstartedFlyby()) )
    {
        changeLocByMouse = true;
        locPosNew = event->pos();
        return;
    }

    QPointF pos = event->scenePos();
    QPointF posPrev = event->scenePos();

    if ((StelMainWindow::getInstance().getIsMultiprojector())
            && (stelApp->getAllowFreeHand()))
    {
        /*rateX = stelApp->viewportRes / scene()->sceneRect().width();
        rateY = stelApp->viewportRes / scene()->sceneRect().height();
        pos.setX(pos.x()*rateX);
        pos.setY(scene()->sceneRect().height() - pos.y()*rateY);*/

        rateX = stelApp->viewportRes / qMin(scene()->sceneRect().width(),scene()->sceneRect().height());
        rateY = stelApp->viewportRes / qMin(scene()->sceneRect().width(),scene()->sceneRect().height());

        qreal fark = qAbs(scene()->sceneRect().width() - scene()->sceneRect().height());

        if (scene()->sceneRect().width() > scene()->sceneRect().height())
        {
            pos.setX((pos.x()- fark / 2.0)*rateX);
            pos.setY(scene()->sceneRect().height() - pos.y()*rateY);
        }
        else
        {
            pos.setX(pos.x()*rateX);
            pos.setY(scene()->sceneRect().height() - (pos.y()- fark / 2.0)*rateY);
        }
    }


    if(!StelMainGraphicsView::getInstance().m_bDesignWarp)
    {
        // Apply distortion on the mouse position.
        distortPos(&pos);
        pos.setY(scene()->height() - 1 - pos.y());
        QMouseEvent newEvent(QEvent::MouseButtonPress, QPoint(pos.x(),pos.y()), event->button(), event->buttons(), event->modifiers());
        stelApp->handleClick(&newEvent);
    }
    else
    {
        if (StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->pick(pos.x(), pos.y()))
        {
        }
    }

    if (StelApp::getInstance().getAllowFreeHand())
    {
        pressed = true;
        //int h =  scene()->height() ;
        stelApp->getCore()->path = QPainterPath();
        stelApp->getCore()->pathPrevScreen = QPainterPath();
        if (StelMainWindow::getInstance().getIsMultiprojector())
        {
            stelApp->getCore()->path.moveTo( pos );
            stelApp->getCore()->oldPathPos = pos;

            stelApp->getCore()->pathPrevScreen.moveTo( posPrev );
            stelApp->getCore()->oldPathPosPrevScreen = posPrev;

        }
        else
        {
            stelApp->getCore()->path.moveTo( event->scenePos() );
            stelApp->getCore()->oldPathPos = event->scenePos();
        }

        //StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_DRAWFREE_START,
        //                                               QString("%0@%1").arg(pos.x()).arg(pos.y()));

    }


}

void StelAppGraphicsWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if(changeLocByMouse)
    {
        double xf = locPosNew.x()-event->pos().x();
        double yf = locPosNew.y()-event->pos().y();
        locPosNew.setX(xf);
        locPosNew.setY(yf);

        StelLocation loc = stelApp->getCore()->getNavigator()->getCurrentLocation();

        loc.latitude = loc.latitude - locPosNew.x()/10.0;
        loc.longitude = loc.longitude - locPosNew.y()/10.0;

        stelApp->getCore()->getNavigator()->moveObserverTo(loc, 0.);
        //StelApp::getInstance().addNetworkCommand(QString("core.setObserverLocation(%0,%1,%2,1,'','").arg(locPosNew.x()).arg(locPosNew.y()).arg(loc.altitude)+loc.planetName+"');");
        StelApp::getInstance().getRsync()->sendChanges(RSYNC_COMMAND_FLYBY_SETLOC,
                                                       QString("%1@%2")
                                                       .arg(loc.latitude)
                                                       .arg(loc.longitude));

        changeLocByMouse = false;
        return;
    }

    // Apply distortion on the mouse position.
    QPointF pos = event->scenePos();
    QPointF posPrev = event->scenePos();

    /* if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        rateX = StelApp::getInstance().viewportRes / scene()->sceneRect().width();
        rateY = StelApp::getInstance().viewportRes / scene()->sceneRect().height();
        pos.setX(pos.x()*rateX);
        pos.setY(scene()->sceneRect().height() - pos.y()*rateY);
    }*/

    if ((!StelApp::getInstance().getAllowFreeHandDel())
            && (!StelApp::getInstance().getAllowFreeHand()))
    {
        distortPos(&pos);
        pos.setY(scene()->height() - 1 - pos.y());
        QMouseEvent newEvent(QEvent::MouseButtonRelease, QPoint(pos.x(),pos.y()), event->button(), event->buttons(), event->modifiers());
        if (StelMainWindow::getInstance().getIsServer())
            stelApp->handleClick(&newEvent);
    }

    if (StelApp::getInstance().getAllowFreeHand())
    {
        pressed = false;
        //--Prepare to Send client
        QList<QPolygonF> polygons = stelApp->getCore()->path.toSubpathPolygons();
        //--
        smoothPath(stelApp->getCore()->path, 4);
        stelApp->getCore()->freehandItems.append(freehandItemPtr(new freehandItem(StelApp::getInstance().getCore()->path,
                                                                                  StelApp::getInstance().getCore()->freeHandPen,
                                                                                  StelApp::getInstance().getCore()->freeHandSize,
                                                                                  StelApp::getInstance().getCore()->freeHandOpacity)));
        if(StelMainWindow::getInstance().getIsMultiprojector())
        {
            smoothPath(stelApp->getCore()->pathPrevScreen, 4);
            stelApp->getCore()->freehandPrevScreenItems.append(freehandItemPtr(new freehandItem(StelApp::getInstance().getCore()->pathPrevScreen,
                                                                                                StelApp::getInstance().getCore()->freeHandPen,
                                                                                                StelApp::getInstance().getCore()->freeHandSize,
                                                                                                StelApp::getInstance().getCore()->freeHandOpacity)));

        }

        //--Send client
        QString strPoints = "";
        //qDebug()<<"Polygons"<<polygons.count();
        for (int i = 0; i< polygons.count() ; i++)
        {
            for (int k = 0; k < polygons[i].count() ; k++ )
            {
                QPointF p = QPointF(polygons[i].at(k));
                strPoints = strPoints + QString("%1-%2").arg(p.x()).arg(p.y())+";";
            }
        }
        QColor m_color= stelApp->getCore()->freeHandPen.color();
        stelApp->getRsync()->sendChanges(RSYNC_COMMAND_DRAWFREE_ADD,
                                         QString("%1@%2@%3@%4-%5-%6@%7@%8")
                                         .arg(strPoints)
                                         .arg(StelMainGraphicsView::getInstance().viewport()->width())
                                         .arg(StelMainGraphicsView::getInstance().viewport()->height())
                                         .arg(m_color.red())
                                         .arg(m_color.green())
                                         .arg(m_color.blue())
                                         .arg(StelApp::getInstance().getCore()->freeHandSize)
                                         .arg(StelApp::getInstance().getCore()->freeHandOpacity));
        //----

        stelApp->getCore()->path = QPainterPath();
        stelApp->getCore()->pathPrevScreen = QPainterPath();
    }

    if (StelApp::getInstance().getAllowFreeHandDel())
    {
        if (StelMainWindow::getInstance().getIsMultiprojector())
            pos.setY(scene()->height() - 1 - pos.y());
        else
            rateX = 1;

        for (int m = 0 ; m < stelApp->getCore()->freehandItems.count() ; m++ )
        {
            if ( IncludePathPoint(stelApp->getCore()->freehandItems[m]->getPath(), pos,
                                  stelApp->getCore()->freehandItems[m]->getSize()*rateX) )
            {
                stelApp->getCore()->freehandItems.erase(&stelApp->getCore()->freehandItems[m] );
                stelApp->getCore()->freehandPrevScreenItems.erase(&StelApp::getInstance().getCore()->freehandPrevScreenItems[m] );
                stelApp->getRsync()->sendChanges(RSYNC_COMMAND_DRAWFREE_DELETE,QString("%1").arg(m));
                break;
            }
        }
        for (int m = 0 ; m < stelApp->getCore()->freehandPrevScreenItems.count() ; m++ )
        {
            if ( IncludePathPoint(stelApp->getCore()->freehandPrevScreenItems[m]->getPath(), posPrev,
                                  stelApp->getCore()->freehandPrevScreenItems[m]->getSize()) )
            {
                stelApp->getCore()->freehandPrevScreenItems.erase(&StelApp::getInstance().getCore()->freehandPrevScreenItems[m] );
                stelApp->getCore()->freehandItems.erase(&StelApp::getInstance().getCore()->freehandItems[m] );
                break;
            }
        }
    }




}

bool StelAppGraphicsWidget::IncludePathPoint(QPainterPath path, QPointF pos, int size)
{
    //int h = StelMainGraphicsView::getInstance().viewport()->height();
    QList<QPolygonF> polygons = path.toSubpathPolygons();
    //qDebug() << "Polygons" << polygons.count();
    for (int i = 0; i< polygons.count() ; i++)
    {
        for (int k = 0; k < polygons[i].count() ; k++ )
        {
            QPointF p = QPointF(polygons[i].at(k));
            //qDebug() << pos.x()<<  "-" <<pos.y() << " ----- " << QPointF(polygons[i].at(k)).x() <<"-" << QPointF(polygons[i].at(k)).y();
            if (( pos.x() <= p.x() + size ) &&
                    ( pos.x() >= p.x() - size ) &&
                    ( pos.y() <= p.y() + size ) &&
                    ( pos.y() >= p.y() - size ))
                return true;
        }
    }
    return false;
}


void StelAppGraphicsWidget::smoothPath(QPainterPath &path, double smoothness, int from, int to)
{
    QPolygonF pol;
    QList<QPolygonF> polygons = path.toSubpathPolygons();

    QList<QPolygonF>::iterator it = polygons.begin();

    QPolygonF::iterator pointIt;

    while (it != polygons.end()) {
        pointIt = (*it).begin();

        while (pointIt <= (*it).end()-2) {
            pol << (*pointIt);
            pointIt += 2;
        }
        ++it;
    }

    if (smoothness > 0) {
        path = TupGraphicalAlgorithm::bezierFit(pol, smoothness, from, to);
    } else {
        path = QPainterPath();
        path.addPolygon(pol);
    }
}

void StelAppGraphicsWidget::wheelEvent(QGraphicsSceneWheelEvent* event)
{

    QPointF pos = event->scenePos();
    if(!StelMainGraphicsView::getInstance().m_bDesignWarp)
    {
        // Apply distortion on the mouse position.
        distortPos(&pos);
        pos.setY(scene()->height() - 1 - pos.y());
        QWheelEvent newEvent(QPoint(pos.x(),pos.y()), event->delta(), event->buttons(), event->modifiers(), event->orientation());
        stelApp->handleWheel(&newEvent);

        updateClientData(stelApp->getCore()->getMovementMgr()->fov_changed);
    }
    else
    {
        if (event->delta() > 0)
            StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->setZoom(StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->getZoom()/1.2f);
        else
            StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->setZoom(StelMainGraphicsView::getInstance().m_pSelectedChannel->getCurrentWarp()->getZoom()*1.2f);
    }

}

void StelAppGraphicsWidget::keyPressEvent(QKeyEvent* event)
{
    stelApp->handleKeys(event);
}

void StelAppGraphicsWidget::keyReleaseEvent(QKeyEvent* event)
{
    stelApp->handleKeys(event);
}

void StelAppGraphicsWidget::resizeEvent(QGraphicsSceneResizeEvent* event)
{
    QGraphicsWidget::resizeEvent(event);

    if (StelMainWindow::getInstance().getIsMultiprojector())
    {
        /*stelApp->glWindowHasBeenResized(0,
                                        0,
                                        StelApp::getInstance().viewportRes,
                                        StelApp::getInstance().viewportRes);
                                        */
    }
    else
    {
        if ((geometry().width() > 0) && (geometry().height() > 0) )
            stelApp->glWindowHasBeenResized(scenePos().x(),
                                            scene()->sceneRect().height()-(scenePos().y()+geometry().height()),
                                            geometry().width(),
                                            geometry().height());
    }

    if (backgroundBuffer)
    {
        delete backgroundBuffer;
        backgroundBuffer = NULL;
    }
    if (foregroundBuffer)
    {
        delete foregroundBuffer;
        foregroundBuffer = NULL;
    }
    if (backgroundPrvBuffer)
    {
        delete backgroundPrvBuffer;
        backgroundPrvBuffer = NULL;
    }
    if (foregroundPrvBuffer)
    {
        delete foregroundPrvBuffer;
        foregroundPrvBuffer = NULL;
    }
}

void StelAppGraphicsWidget::updateClientData()
{
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitFov(StelApp::getInstance().getCore()->getMovementMgr()->getCurrentFov());
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitPos(StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(StelApp::getInstance().getCore()->getMovementMgr()->getViewDirectionJ2000()));
        }
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}

void StelAppGraphicsWidget::updateClientData(double fov)
{
    if (StelMainWindow::getInstance().getIsServer() && StelMainGraphicsView::getInstance().isClientConf)
    {
        if(StelMainGraphicsView::getInstance().m_pSelectedChannel)
        {
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitFov(fov);
            StelMainGraphicsView::getInstance().m_pSelectedChannel->setinitPos(StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(StelApp::getInstance().getCore()->getMovementMgr()->getViewDirectionJ2000()));
        }
        StelMainGraphicsView::getInstance().m_pSelectedChannel->updateClientData();
    }
}


void StelAppGraphicsWidget::CalcUV(double px, double py, double pz, double *u, double *v)
{
    double r,phi;

    r = atan2(sqrt(px*px+pz*pz),py);
    r /= M_PI;  /* -0.5 .. 0.5 */
    phi = atan2(pz,px);

    *u = r * cos(phi) + 0.5;
    *v = r * sin(phi) + 0.5;
}
void StelAppGraphicsWidget::DomeScreen(double radius)
{
    double m_radius;                //!< Radius.
    unsigned int m_elevResolution;  //!< Elevation resolution.
    unsigned int m_azimResolution;  //!< Azimuth resolution.
    int m_subdiv;                   //!< Subdivision steps.
    bool m_bFullDome;               //!< True is a full dome, False is a half dome.

    m_radius = radius;
    m_elevResolution = 16;
    m_azimResolution = 16;
    m_subdiv = 4;
    m_bFullDome = true;

    // draw as a polygon model
    double azimDelta = M_PI / m_azimResolution / m_subdiv;
    double elevDelta = M_PI / 2.0 / m_elevResolution / m_subdiv;
    if (m_bFullDome) azimDelta *= 2.0;
    for (unsigned int elevCount=0; elevCount<m_elevResolution*m_subdiv; ++elevCount)
    {
        glBegin(GL_QUAD_STRIP);
        for (unsigned int azimCount=0; azimCount<=m_azimResolution*m_subdiv; ++azimCount)
        {
            double nx, ny, nz;
            double tu,tv;

            nx = cos(-azimDelta*azimCount) * cos(elevDelta*elevCount);
            ny = sin(elevDelta*elevCount);
            nz = sin(-azimDelta*azimCount) * cos(elevDelta*elevCount);
            CalcUV(m_radius * nx,m_radius * ny, m_radius * nz,&tu,&tv);
            glTexCoord2d(tu,tv);
            glNormal3d(-nx, -ny, -nz);
            glVertex3d(m_radius * nx, m_radius * ny, m_radius * nz);

            nx = cos(-azimDelta*azimCount) * cos(elevDelta*(elevCount+1));
            ny = sin(elevDelta*(elevCount+1));
            nz = sin(-azimDelta*azimCount) * cos(elevDelta*(elevCount+1));
            CalcUV(m_radius * nx,m_radius * ny, m_radius * nz,&tu,&tv);
            glTexCoord2d(tu,tv);
            glNormal3d(-nx, -ny, -nz);
            glVertex3d(m_radius * nx, m_radius * ny, m_radius * nz);
        }
        glEnd();
    }

}

void StelAppGraphicsWidget::DrawDomeDistortion(QGLFramebufferObject *buf)
{
    //    glPopAttrib();
    //    glMatrixMode(GL_PROJECTION);
    //    glPopMatrix();
    //    glMatrixMode(GL_MODELVIEW);

    //    glDisable(GL_CULL_FACE);

    //    glEnableClientState(GL_VERTEX_ARRAY);
    //    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glEnable(GL_BLEND);
    //    glEnable(GL_TEXTURE_2D);
    //    glEnable(GL_DEPTH_TEST);

    //    glEnable(GL_SMOOTH);
    //    glDisable(GL_LIGHTING);
    //    glEnable(GL_COLOR_MATERIAL);

    //    //    QGLFramebufferObject target(buf->size());
    //    //    QGLFramebufferObject::blitFramebuffer(&target,QRect(0, 0, buf->width(), buf->height()),
    //    //                                          buf,    QRect((buf->width()-buf->height())/2,
    //    //                                                        0,
    //    //                                                        buf->height(),
    //    //                                                        buf->height()),
    //    //                                                        GL_COLOR_BUFFER_BIT,
    //    //                                                        GL_NEAREST );
    //    glBindTexture(GL_TEXTURE_2D, buf->texture());

    //    glrcBindCompositor(GLRC_COMPOSITOR_2D, compositorIndex);
    //    /* begin to render a scene into off-screen buffer */
    //    glrcBeginRender();
    //    glClearColor(0.0, 0.0, 0.0, 1.0);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    //glrcSetScale(0.5f,0.5f);
    //    //glViewport(0, 0, 1920, 1080);

    //    //float frustum[6] = {-0.0988428, 0.0988428, -0.0661886 ,0.0661886, 0.1 ,100};
    //    //glrcGetProjectionFrustum(frustum);
    //    //qDebug()<<frustum[0]<<frustum[1]<<frustum[2]<<frustum[3]<<frustum[4]<<frustum[5];
    //    //glrcSetProjectionFrustum(frustum);
    //    glrcViewport(0.0f,0.0f,
    //                 geometry().width()/StelApp::getInstance().viewportRes,
    //                 geometry().height()/StelApp::getInstance().viewportRes);
    //    DomeScreen(1.0);

    //    glPopMatrix();
    //    glMatrixMode(GL_MODELVIEW);
    //    /* now use the off-screen rendered scene image to display */
    //    glrcEndRender();

}

