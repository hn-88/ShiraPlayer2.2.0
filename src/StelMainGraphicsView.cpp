/*
 * ShiraPlayer(TM)
 * Copyright (C) 2002 Fabien Chereau
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

#include "StelMainGraphicsView.hpp"
#include "StelAppGraphicsWidget.hpp"
#include "StelApp.hpp"
#include "StelCore.hpp"
#include "StelFileMgr.hpp"
#include "StelProjector.hpp"
#include "StelModuleMgr.hpp"
#include "StelScriptMgr.hpp"
#include "StelPainter.hpp"
#include "StelGuiBase.hpp"
#include "StelMainScriptAPIProxy.hpp"
#include "StelMainWindow.hpp"
#include "StelGui.hpp"
#include "StelNavigator.hpp"
#include "StelObjectMgr.hpp"

#include <QtOpenGL/QGLFormat>
#include <QPaintEngine>
#include <QGraphicsView>
#include <QtOpenGL/QGLWidget>
#include <QResizeEvent>
#include <QSettings>
#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsGridLayout>
#include <QGraphicsProxyWidget>
#include <stelqglwidget.h>

//ASAF
#include "StelMovementMgr.hpp"
//#include "shiraplayerform.hpp"

// Initialize static variables
StelMainGraphicsView* StelMainGraphicsView::singleton = NULL;

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

/*class StelQGLWidget : public QGLWidget
{
public:
    StelQGLWidget(const QGLFormat& format, QWidget* parent) : QGLWidget(format, parent)
    {
        qDebug()<<"StelQGLWidget created";
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_OpaquePaintEvent);
        //setAutoFillBackground(false);
        setBackgroundRole(QPalette::Window);
    }

protected:

    virtual void initializeGL()
    {
        qDebug()<<"initializeGL worked";
        QGLWidget::initializeGL();

        if (!format().stencil())
            qWarning("Could not get stencil buffer; results will be suboptimal");
        if (!format().depth())
            qWarning("Could not get depth buffer; results will be suboptimal");
        if (!format().doubleBuffer())
            qWarning("Could not get double buffer; results will be suboptimal");

    }

    virtual void paintGL()
    {
        QGLWidget::paintGL();
        qDebug()<<"paintGL worked";

    }
};
*/
StelMainGraphicsView::StelMainGraphicsView(QWidget* parent)
    : QGraphicsView(parent), backItem(NULL), gui(NULL), scriptAPIProxy(NULL), scriptMgr(NULL),
      wasDeinit(false),
      flagInvertScreenShotColors(false),
      screenShotPrefix("shiraplayer-"),
      screenShotDir(""),
      cursorTimeout(-1.f), flagCursorTimeout(false), minFpsTimer(NULL), maxfps(10000.f)
{
    StelApp::initStatic();

    // Can't create 2 StelMainGraphicsView instances
    Q_ASSERT(!singleton);
    singleton = this;

    setObjectName("Mainview");

    // Avoid white background at init
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Window);
    QPalette pal;
    pal.setColor(QPalette::Window, Qt::black);
    setPalette(pal);

    // Allows for precise FPS control
    setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    //setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(Qt::StrongFocus);
    connect(this, SIGNAL(screenshotRequested()), this, SLOT(doScreenshot()));

    lastEventTimeSec = 0;

    // Create an openGL viewport
    QGLFormat glFormat(QGL::StencilBuffer | QGL::DepthBuffer | QGL::DoubleBuffer );
    glWidget = new StelQGLWidget(glFormat, NULL);
    //glWidget->show();
    //glWidget->updateGL();
    setViewport(glWidget);

    setOptimizationFlags(QGraphicsView::DontClipPainter|QGraphicsView::DontSavePainterState|QGraphicsView::DontAdjustForAntialiasing);
    setScene(new QGraphicsScene());

    backItem = new QGraphicsWidget();
    backItem->setFocusPolicy(Qt::NoFocus);

    //ASAF record için
    b_save  = false;
    timer_record = new QTimer(this);
    connect(&rec_thread, SIGNAL(encodeImage(QImage)), this, SLOT(trigencodeImage(QImage)));
    connect(timer_record, SIGNAL(timeout()), this, SLOT(update_record()));

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    //  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
    //         this, SLOT(ShowContextMenu(const QPoint&)));

}

StelMainGraphicsView::~StelMainGraphicsView()
{
}

void StelMainGraphicsView::swapBuffer()
{   
    Q_ASSERT(glWidget!=NULL);
    Q_ASSERT(glWidget->isValid());
    glWidget->swapBuffers();
}

void StelMainGraphicsView::makeGLContextCurrent()
{
    Q_ASSERT(glWidget!=NULL);
    Q_ASSERT(glWidget->isValid());
    glWidget->makeCurrent();
}

void StelMainGraphicsView::init(QSettings* conf)
{
    Q_ASSERT(glWidget->isValid());
    glWidget->makeCurrent();
    //ASAF
    m_bDesignWarp = false;
    pwarpMode = showMode;
    m_blendAreaTexIndex = 0;
    m_pSelectedChannel = NULL;
    isClientConf = false;
    //
    qDebug() << "GL_VENDOR     : "<< QLatin1String(reinterpret_cast<const char*>(glGetString( GL_VENDOR)));
    qDebug() << "GL_RENDERER   : "<< QLatin1String(reinterpret_cast<const char*>(glGetString( GL_RENDERER )));
    qDebug() << "GL_VERSION    : "<< QLatin1String(reinterpret_cast<const char*>(glGetString( GL_VERSION  )));
    qDebug() << "GL_SHADING_LANGUAGE_VERSION : "<< QLatin1String(reinterpret_cast<const char*>(glGetString ( GL_SHADING_LANGUAGE_VERSION )));

    // Create the main widget for stellarium, which in turn creates the main StelApp instance.
    mainSkyItem = new StelAppGraphicsWidget();
    mainSkyItem->setZValue(-10);
    QGraphicsGridLayout* l = new QGraphicsGridLayout(backItem);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    l->addItem(mainSkyItem, 0, 0);
    scene()->addItem(backItem);

    // Activate the resizing caused by the layout
    QCoreApplication::processEvents();

    mainSkyItem->setFocus();

    flagInvertScreenShotColors = conf->value("main/invert_screenshots_colors", false).toBool();
    setFlagCursorTimeout(conf->value("gui/flag_mouse_cursor_timeout", false).toBool());
    setCursorTimeout(conf->value("gui/mouse_cursor_timeout", 10.).toDouble());
    maxfps = conf->value("video/maximum_fps",10000.).toDouble();
    minfps = conf->value("video/minimum_fps",10000.).toDouble();

    QPainter qPainter(glWidget);
    StelPainter::setQPainter(&qPainter);

    // Initialize the core, including the StelApp instance.
    mainSkyItem->init(conf);
    // Prevent flickering on mac Leopard/Snow Leopard
    glWidget->setAutoFillBackground (false);

    scriptAPIProxy = new StelMainScriptAPIProxy(this);
    scriptMgr = new StelScriptMgr(this);

    // Look for a static GUI plugins.
    foreach (QObject *plugin, QPluginLoader::staticInstances())
    {
        StelGuiPluginInterface* pluginInterface = qobject_cast<StelGuiPluginInterface*>(plugin);
        if (pluginInterface)
        {
            gui = pluginInterface->getStelGuiBase();
        }
        break;
    }
    Q_ASSERT(gui);	// There was no GUI plugin found

    StelApp::getInstance().setGui(gui);
    gui->init(backItem, mainSkyItem);
    StelApp::getInstance().initPlugIns();

    // Force refreshing of button bars if plugins modified the GUI, e.g. added buttons.
    //gui->forceRefreshGui();

    const QString& startupScript = conf->value("scripts/startup_script", "startup.ssc").toString();
    scriptMgr->runScript(startupScript);

    QThread::currentThread()->setPriority(QThread::HighestPriority);
    //StelPainter::setQPainter(NULL);
    startMainLoop();

    //ASAF
    //#ifdef SHIRAPLAYER_PRE
    if(!StelMainWindow::getInstance().getIsMultiprojector())
    {
        //m_pSelectedChannel = new Channel();
        if(!StelMainWindow::getInstance().getIsServer())
            if (conf->value("video/apply_finetune",false).toBool())
                loadWarpSettings("warpsettings.xml");
    }
    //#endif

    StelApp::getInstance().updateI18n();


}

void StelMainGraphicsView::thereWasAnEvent()
{
    //ASAF
    //if(!StelApp::getInstance().isVideoMode)
    lastEventTimeSec = StelApp::getTotalRunTime();
    //else
    //    lastEventTimeSec = -100;
}

void StelMainGraphicsView::drawBackground(QPainter* painter, const QRectF& rect)
{
    //	if (painter->paintEngine()->type()!=QPaintEngine::OpenGL && painter->paintEngine()->type()!=QPaintEngine::OpenGL2)
    //	{
    //		qWarning("StelMainGraphicsView: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
    //		return;
    //	}
    //
    const double now = StelApp::getTotalRunTime();
    //
    //	// Determines when the next display will need to be triggered
    //	// The current policy is that after an event, the FPS is maximum for 2.5 seconds
    //	// after that, it switches back to the default minfps value to save power
    //	if (now-lastEventTimeSec<2.5)
    //	{
    //		double duration = 1./getMaxFps();
    //		int dur = (int)(duration*1000);
    //		QTimer::singleShot(dur<5 ? 5 : dur, scene(), SLOT(update()));
    //	}
    // Manage cursor timeout
    if (cursorTimeout>0.f && (now-lastEventTimeSec>cursorTimeout) && flagCursorTimeout)
    {
        if (QApplication::overrideCursor()==0)
            QApplication::setOverrideCursor(Qt::BlankCursor);
    }
    else
    {
        if (QApplication::overrideCursor()!=0)
            QApplication::restoreOverrideCursor();
    }
    //#ifndef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsMultiprojector())
    {
        if(!StelMainWindow::getInstance().getIsServer())
            if (QApplication::overrideCursor()==0)
                QApplication::setOverrideCursor(Qt::BlankCursor);
    }
    //#endif

    //QGraphicsView::drawBackground(painter, rect);
}

void StelMainGraphicsView::drawForeground(QPainter* painter, const QRectF &rect)
{
    //QGraphicsView::drawForeground(painter, rect);
}

void StelMainGraphicsView::startMainLoop()
{
    // Set a timer refreshing for every minfps frames
    minFpsChanged();
}

void StelMainGraphicsView::minFpsChanged()
{
    if (minFpsTimer!=NULL)
    {
        disconnect(minFpsTimer, SIGNAL(timeout()), 0, 0);
        delete minFpsTimer;
        minFpsTimer = NULL;
    }

    minFpsTimer = new QTimer(this);
    connect(minFpsTimer, SIGNAL(timeout()), scene(), SLOT(update()));
    minFpsTimer->start((int)(1./getMinFps()*1000.));
}

void StelMainGraphicsView::resizeEvent(QResizeEvent* event)
{
    scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
    backItem->setGeometry(0,0,event->size().width(),event->size().height());
    QGraphicsView::resizeEvent(event);
}

void StelMainGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::mouseMoveEvent(event);
}

void StelMainGraphicsView::mousePressEvent(QMouseEvent* event)
{
    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::mousePressEvent(event);
}

StelObjectP selectedObject = NULL;
void StelMainGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    //Front sky deðiþtirildi kabul edilecek
    StelApp::getInstance().getCore()->getMovementMgr()->frontDirection ="";
    //--

    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::mouseReleaseEvent(event);
    //#ifdef SHIRAPLAYER_PRE
    StelObjectP newSelectObject = NULL;
    if( (StelMainWindow::getInstance().getIsServer()) &&
            ( (!StelApp::getInstance().getAllowFreeHandDel()) &&
              (!StelApp::getInstance().getAllowFreeHand()) ) )
    {
        if (StelApp::getInstance().getStelObjectMgr().getSelectedObject().count()>0)
            newSelectObject = StelApp::getInstance().getStelObjectMgr().getSelectedObject()[0];

        if (selectedObject == newSelectObject)
        {
            //QMessageBox::information(0,"","ok",0,0);
            Sleep(100);
            const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
            double alt, azi;
            StelUtils::rectToSphe(&azi, &alt, current);
            alt = (alt)*180/M_PI; // convert to degrees from radians
            azi = std::fmod((((azi)*180/M_PI)*-1)+180., 360.);
            if (alt >89.9)
                alt = 89.9;
            StelApp::getInstance().addNetworkCommand("StelMovementMgr.setFlagTracking(false);core.moveToAltAzi("+QString("%0").arg(alt)+","+QString("%0").arg(azi)+",2);");
        }
    }
    selectedObject = newSelectObject;
    //#endif
}

void StelMainGraphicsView::wheelEvent(QWheelEvent* event)
{
    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::wheelEvent(event);
    //#ifdef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsServer())
    {
        double fov = GETSTELMODULE(StelMovementMgr)->getAimFov();
        StelApp::getInstance().addNetworkCommand("StelMovementMgr.zoomTo("+QString("%0").arg(fov)+",3);");
    }
    //#endif

}

void StelMainGraphicsView::keyPressEvent(QKeyEvent* event)
{
    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::keyPressEvent(event);
}

void StelMainGraphicsView::keyReleaseEvent(QKeyEvent* event)
{
    thereWasAnEvent(); // Refresh screen ASAP
    QGraphicsView::keyReleaseEvent(event);
    //QMessageBox::information(0,"",QString("%1").arg( event->key()),0,0);
    //#ifdef SHIRAPLAYER_PRE
    if(StelMainWindow::getInstance().getIsServer())
    {
        if (((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt:: Key_Up))||
                ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt:: Key_Down))||
                (event->key() == Qt::Key_PageDown)||
                (event->key() == Qt::Key_PageUp))
        {
            double fov = GETSTELMODULE(StelMovementMgr)->getAimFov();
            StelApp::getInstance().addNetworkCommand("StelMovementMgr.zoomTo("+QString("%0").arg(fov)+",3);");
            //QMessageBox::information(0,"","ok",0,0);
        }
        else if ((event->key() == Qt::Key_Left) ||
                 (event->key() == Qt::Key_Right) ||
                 (event->key() == Qt::Key_Up) ||
                 (event->key() == Qt::Key_Down))
        {
            //QMessageBox::information(0,"","ok2",0,0);
            const Vec3d& current = StelApp::getInstance().getCore()->getNavigator()->j2000ToAltAz(GETSTELMODULE(StelMovementMgr)->getViewDirectionJ2000());
            double alt, azi;
            StelUtils::rectToSphe(&azi, &alt, current);
            alt = (alt)*180/M_PI; // convert to degrees from radians
            azi = std::fmod((((azi)*180/M_PI)*-1)+180., 360.);
            if (alt >89.9)
                alt = 89.9;
            Sleep(100);
            StelApp::getInstance().addNetworkCommand("StelMovementMgr.setFlagTracking(false);core.moveToAltAzi("+QString("%0").arg(alt)+","+QString("%0").arg(azi)+",2);");
        }
    }
    //#endif
}

void StelMainGraphicsView::enterEvent(QEvent *event)
{
    if (!StelMainWindow::getInstance().getIsServer()) return;
    if ( StelApp::getInstance().getAllowFreeHand() )
    {
        QImage image(":cursors/cursors/pencil.png");
        QCursor cursor = QCursor(QPixmap::fromImage(image), 0, 16);
        setCursor(cursor);
    }
    if ( StelApp::getInstance().getAllowFreeHandDel())
    {
        QImage image(":cursors/cursors/eraser.png");
        QCursor cursor = QCursor(QPixmap::fromImage(image), 0, 20);
        setCursor(cursor);
    }
}

void StelMainGraphicsView::leaveEvent(QEvent *event)
{
    if (!StelMainWindow::getInstance().getIsServer()) return;
    setCursor(QCursor(Qt::ArrowCursor));
}
void StelMainGraphicsView::focusOutEvent(QFocusEvent* event)
{
    //Bu satýrlar tam ekran olduðunda ekraný minimize ediyordu, iptal edildi
    //if (StelMainWindow::getInstance().isFullScreen())
    //   StelMainWindow::getInstance().showMinimized();

    QCoreApplication::processEvents();
    QGraphicsView::focusOutEvent(event);
}

void StelMainGraphicsView::focusInEvent(QFocusEvent* event)
{
    //TODO: Test if this is really necessary
    StelMainWindow::getInstance().activateWindow();
    QCoreApplication::processEvents();
    QGraphicsView::focusInEvent(event);
}

//! Delete openGL textures (to call before the GLContext disappears)
void StelMainGraphicsView::deinitGL()
{
    // Can be called only once
    if (wasDeinit==true)
        return;
    wasDeinit = true;
    if (scriptMgr->scriptIsRunning())
        scriptMgr->stopScript();
    StelApp::getInstance().getModuleMgr().unloadAllPlugins();
    QCoreApplication::processEvents();
    delete mainSkyItem;
}

void StelMainGraphicsView::saveScreenShot(const QString& filePrefix, const QString& saveDir)
{
    screenShotPrefix = filePrefix;
    screenShotDir = saveDir;
    emit(screenshotRequested());
}

void StelMainGraphicsView::doScreenshot(void)
{
    QFileInfo shotDir;
    imagescreen = glWidget->grabFrameBuffer();
    //	if (flagInvertScreenShotColors)
    //		im.invertPixels();
    //
    //	if (screenShotDir == "")
    //		shotDir = QFileInfo(StelFileMgr::getScreenshotDir());
    //	else
    //		shotDir = QFileInfo(screenShotDir);
    //
    //	if (!shotDir.isDir())
    //	{
    //		qWarning() << "ERROR requested screenshot directory is not a directory: " << shotDir.filePath();
    //		return;
    //	}
    //	else if (!shotDir.isWritable())
    //	{
    //		qWarning() << "ERROR requested screenshot directory is not writable: " << shotDir.filePath();
    //		return;
    //	}
    //
    //	QFileInfo shotPath;
    //	for (int j=0; j<100000; ++j)
    //	{
    //		shotPath = QFileInfo(shotDir.filePath() + "/" + screenShotPrefix + QString("%1").arg(j, 3, 10, QLatin1Char('0')) + ".png");
    //		if (!shotPath.exists())
    //			break;
    //	}
    //
    //	qDebug() << "INFO Saving screenshot in file: " << shotPath.filePath();
    //	if (!im.save(shotPath.filePath())) {
    //		qWarning() << "WARNING failed to write screenshot to: " << shotPath.filePath();
    //	}
    //
    //ASAF
    //imagescreen = im;
}

QImage StelMainGraphicsView::getScreenasImage()
{
    QFileInfo shotDir;
    QImage im = glWidget->grabFrameBuffer();
    if (flagInvertScreenShotColors)
        im.invertPixels();

    screenShotDir = "c:/";

    screenShotPrefix = "";

    emit(screenshotRequested());
    doScreenshot();
    return im;
}

void StelMainGraphicsView::setwarpMode(const QString& smode)
{
    if(smode == "Distortion Warp Mode")
    {
        m_bDesignWarp = true;
        pwarpMode = distortionMode;
    }
    else if(smode == "Blend Warp Mode")
    {
        m_bDesignWarp = true;
        pwarpMode = blendMode;
    }
    else if(smode == "Show Mode")
    {
        m_bDesignWarp = false;
        pwarpMode = showMode;
    }
}

void StelMainGraphicsView::saveWarpSettings(char* filename)
{
    //    QFile file(filename);
    //    if (file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    //    {
    //        QTextStream out(&file);
    //        QDomDocument doc;
    //
    //        QDomElement main = doc.createElement("Distortion");
    //        main.setAttribute("version", 1.0);
    //        doc.appendChild(main);
    //
    //        for (unsigned int i = 0; i < m_pChannels.size(); ++i)
    //            main.appendChild(m_pChannels[i]->domElement(m_pChannels[i]->getName(),doc));
    //
    ////        main.appendChild(StelMainGraphicsView::getInstance().m_pDistWarp->domElement("Distorter",doc));
    ////        main.appendChild(StelMainGraphicsView::getInstance().m_pBlendWarp->domElement("Blend",doc));
    //
    //        doc.save(out, 4);
    //        file.flush();
    //        file.close();
    //    }
}

void StelMainGraphicsView::loadWarpSettings(QString filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(&file);
        file.close();

        QDomElement main;
        main = doc.documentElement();

        // Initialising channels
        QDomElement channelelement = main.firstChildElement("Channel");
        while (!channelelement.isNull())
        {
            Channel* pChannel = new Channel();
            pChannel->initFromDOMElement(channelelement);
            m_pChannels.push_back(pChannel);
            channelelement = channelelement.nextSiblingElement("Channel");
            //#ifdef SHIRAPLAYER_PRE
            if(!StelMainWindow::getInstance().getIsMultiprojector())
            {
                setwarpMode("Show Mode");
                StelApp::getInstance().getCore()->initWarpGL();
                m_pSelectedChannel = pChannel;
            }
            //#endif
        }
    }
}

unsigned int StelMainGraphicsView::getBlendingAreaTexture()
{
    bool m_bSoftEdge = true;
    int m_blendEdgeWidth = 16;
    float m_blendEdgeExponent = 1.0f;

    if (m_blendAreaTexIndex == 0)
    {
        int blendBaseValue = 255;
        QPixmap pixmap(128, 128);
        pixmap.fill(QColor(blendBaseValue, blendBaseValue, blendBaseValue));
        if (m_bSoftEdge)
        {
            QPainter painter(&pixmap);
            QPen pen;
            for (int w=0; w<m_blendEdgeWidth; w++) {
                int value = int(pow(1.0f/m_blendEdgeWidth*(m_blendEdgeWidth-w-1), m_blendEdgeExponent) * blendBaseValue);
                pen.setColor(QColor(value, value, value));
                pen.setWidth(m_blendEdgeWidth-w);
                painter.setPen(pen);
                painter.drawRect(0, 0, 127, 127);
            }
        }
        m_blendAreaTexIndex = this->getOpenGLWin()->bindTexture(pixmap);
        pixmap.detach();
    }
    return m_blendAreaTexIndex;
}

void StelMainGraphicsView::removeChannel(int index)
{
    if (index < 0)
        return;

    Channel* pChannel = m_pChannels.takeAt(index);
    if (m_pSelectedChannel == pChannel) {
        m_pSelectedChannel = NULL;
    }

}

void StelMainGraphicsView::startLoopTimer(bool status)
{
    if (status)
        minFpsTimer->start();
    else
        minFpsTimer->stop();
}

void StelMainGraphicsView::update_record()
{
    if(b_save)
    {
        if (StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->getViewportEffect() == "sphericMirrorDistorter")
            rec_thread.appendQIMGList(glWidget->grabFrameBuffer());
        else
            rec_thread.appendQIMGList(glWidget->grabFrameBuffer().copy(glWidget->width()/2- glWidget->height()/2,0,glWidget->height(),glWidget->height() )); //Fisheye

        rec_thread.start(QThread::NormalPriority);
    }

}

void StelMainGraphicsView::startRecordFile(QString filename, int FPS)
{
    int bitrate = 53.75*1024*1024; // 53.75 Mbit - High quality
    if (StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->getViewportEffect() == "sphericMirrorDistorter")
        v_encode.createFile(filename,glWidget->width(),glWidget->height(),bitrate ,0,FPS);
    else
        v_encode.createFile(filename,glWidget->height(),glWidget->height(),bitrate ,0,FPS); //width = height ,Fisheye

    timer_record->start(1000/FPS);

    //connect(&rec_thread, SIGNAL(encodeImage(QImage)), this, SLOT(trigencodeImage(QImage)));

    b_save = true;
}
void StelMainGraphicsView::trigencodeImage(QImage img)
{
    v_encode.encodeImage(img);
}

void StelMainGraphicsView::stopRecordFile()
{
    b_save = false;
    //rec_thread.disconnect(SIGNAL(encodeImage(QImage)));
    timer_record->stop();
    v_encode.close();
}

bool StelMainGraphicsView::pauseRecord(bool state)
{
    if (timer_record->isActive())
    {
        if(state)
            b_save = false;
        else
            b_save = true;
        return true;
    }
    else
        return false;

}

