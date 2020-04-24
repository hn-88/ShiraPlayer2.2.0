/*
 * ShiraPlayer(TM)
 * Copyright (C) 2006 Fabien Chereau
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

#include "StelApp.hpp"

#include "StelCore.hpp"
#include "StelUtils.hpp"
#include "StelTextureMgr.hpp"
#include "StelLoadingBar.hpp"
#include "StelObjectMgr.hpp"

#include "TelescopeMgr.hpp"
#include "ConstellationMgr.hpp"
#include "NebulaMgr.hpp"
#include "LandscapeMgr.hpp"
#include "GridLinesMgr.hpp"
#include "MilkyWay.hpp"
#include "MeteorMgr.hpp"
#include "LabelMgr.hpp"
#include "ScreenImageMgr.hpp"
#include "StarMgr.hpp"
#include "SolarSystem.hpp"
#include "StelIniParser.hpp"
#include "StelProjector.hpp"
#include "StelLocationMgr.hpp"

#include "StelModuleMgr.hpp"
#include "StelLocaleMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "StelMovementMgr.hpp"
#include "StelFileMgr.hpp"
#include "StelScriptMgr.hpp"
#include "StelMainScriptAPIProxy.hpp"
#include "StelJsonParser.hpp"
#include "StelSkyLayerMgr.hpp"
#include "StelAudioMgr.hpp"
#include "StelStyle.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelGuiBase.hpp"
#include "StelPainter.hpp"
//ASAF
#include "socketutils/mysocket.h"
#include "socketutils/rsync.h"
#include "StelFontMgr.hpp"
#include "StelNavigator.hpp"
#include "shiraplayerform.hpp"
#include "StelMainWindow.hpp"
#include "StelPresentMgr.hpp"
#include "StelMainScriptAPI.hpp"
#include "StelObserver.hpp"
#include "StelAppGraphicsWidget.hpp"
//ASAF
#include <iostream>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMouseEvent>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QSysInfo>
#include <QtNetwork/QNetworkProxy>
#include <QMessageBox>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkReply>
#include <QtPlugin>

#include "catalogpages/messierimage.h"

// The static plugins need to be imported here so that they belong to the
// libStelMain required on win32.
//Q_IMPORT_PLUGIN(StelGui)
Q_IMPORT_PLUGIN(StelStandardGuiPluginInterface)

#ifdef USE_STATIC_PLUGIN_VIRGO
Q_IMPORT_PLUGIN(VirGO)
#endif

#ifdef USE_STATIC_PLUGIN_SVMT
Q_IMPORT_PLUGIN(SVMT)
#endif

#ifdef USE_STATIC_PLUGIN_HELLOSTELMODULE
Q_IMPORT_PLUGIN(HelloStelModule)
#endif

#ifdef USE_STATIC_PLUGIN_ANGLEMEASURE
Q_IMPORT_PLUGIN(AngleMeasureStelPluginInterface)
#endif

#ifdef USE_STATIC_PLUGIN_COMPASSMARKS
Q_IMPORT_PLUGIN(CompassMarksStelPluginInterface)
#endif

#ifdef USE_STATIC_PLUGIN_SATELLITES
Q_IMPORT_PLUGIN(SatellitesStelPluginInterface)
#endif

#ifdef USE_STATIC_PLUGIN_TEXTUSERINTERFACE
Q_IMPORT_PLUGIN(TextUserInterfaceStelPluginInterface)
#endif

#ifdef USE_STATIC_PLUGIN_LOGBOOK
Q_IMPORT_PLUGIN(LogBook)
#endif

#ifdef USE_STATIC_PLUGIN_OCULARS
Q_IMPORT_PLUGIN(OcularsStelPluginInterface)
#endif

#ifdef USE_STATIC_PLUGIN_TELESCOPECONTROL
Q_IMPORT_PLUGIN(TelescopeControlStelPluginInterface)
#endif

// Initialize static variables
StelApp* StelApp::singleton = NULL;
QTime* StelApp::qtime = NULL;

void StelApp::initStatic()
{
    StelApp::qtime = new QTime();
    StelApp::qtime->start();
}

/*************************************************************************
 Create and initialize the main Stellarium application.
*************************************************************************/
StelApp::StelApp(QObject* parent)
    : QObject(parent), core(NULL), stelGui(NULL), fps(0),
      frame(0), timefr(0.), timeBase(0.), flagNightVision(false),
      confSettings(NULL), initialized(false), saveProjW(-1), saveProjH(-1), drawState(0)
{
    // Stat variables
    nbDownloadedFiles=0;
    totalDownloadedSize=0;
    nbUsedCache=0;
    totalUsedCacheSize=0;

    setObjectName("StelApp");

    skyCultureMgr=NULL;
    localeMgr=NULL;
    stelObjectMgr=NULL;
    textureMgr=NULL;
    moduleMgr=NULL;
    loadingBar=NULL;
    networkAccessManager=NULL;

    // Can't create 2 StelApp instances
    Q_ASSERT(!singleton);
    singleton = this;

    moduleMgr = new StelModuleMgr();

    // Init a default StelStyle, before loading modules, it will be overrided
    currentStelStyle = NULL;
    setColorScheme("color");

    //ASAF
    //mysocket* m_mysocket=new mysocket(this); //Socket start
    isLiveMode = false;
    isTimeCommand = false;
    isNetCom = false;
    isScriptConsole = false;
    isVideoMode = false;
    isVideoLandscape = false;
    isDiscreteTimeSteps = false;
    m_timedirection = 1.0;

    //ASAF
    fov_height_rate = 1.0;
    fov_width_rate = 1.0;

    //        //ASAF
    //        b_startedRecord = false;

    //#ifdef SHIRAPLAYER_PRE
    if (!StelMainWindow::getInstance().getIsMultiprojector())
    {
        if (StelMainWindow::getInstance().isServer)
        {
            //QMessageBox::information(0,"server","server",0,0);
            QString program ="";
#ifdef Q_OS_WIN
            program = "./shiraplayer";
#else
            program = QCoreApplication::applicationDirPath()+"/shiraplayer";
#endif
            QStringList arguments;
            arguments << "client" ;

            QProcess *shiraClientProcess = new QProcess(this);
            shiraClientProcess->start(program, arguments);
        }
    }
    //#endif

    allowFreeHand = false;
    allowFreeHandDel = false;
}

/*************************************************************************
 Deinitialize and destroy the main Stellarium application.
*************************************************************************/
StelApp::~StelApp()
{
    qDebug() << qPrintable(QString("Downloaded %1 files (%2 kbytes) in a session of %3 sec (average of %4 kB/s + %5 files from cache (%6 kB)).").arg(nbDownloadedFiles).arg(totalDownloadedSize/1024).arg(getTotalRunTime()).arg((double)(totalDownloadedSize/1024)/getTotalRunTime()).arg(nbUsedCache).arg(totalUsedCacheSize/1024));

    stelObjectMgr->unSelect();
    moduleMgr->unloadModule("StelSkyLayerMgr", false);  // We need to delete it afterward
    moduleMgr->unloadModule("StelObjectMgr", false);// We need to delete it afterward
    StelModuleMgr* tmp = moduleMgr;
    moduleMgr = new StelModuleMgr(); // Create a secondary instance to avoid crashes at other deinit
    delete tmp; tmp=NULL;
    delete core; core=NULL;
    delete skyCultureMgr; skyCultureMgr=NULL;
    delete localeMgr; localeMgr=NULL;
    delete audioMgr; audioMgr=NULL;
    delete stelObjectMgr; stelObjectMgr=NULL; // Delete the module by hand afterward
    delete textureMgr; textureMgr=NULL;
    delete planetLocationMgr; planetLocationMgr=NULL;
    delete moduleMgr; moduleMgr=NULL; // Delete the secondary instance
    delete currentStelStyle;
    delete networkAccessManager;

    Q_ASSERT(singleton);
    singleton = NULL;
}


void StelApp::init(QSettings* conf)
{
    confSettings = conf;
    core = new StelCore();
    if (saveProjW!=-1 && saveProjH!=-1)
        core->windowHasBeenResized(0, 0, saveProjW, saveProjH);

    fontManager = new StelFontMgr();

#ifndef USE_OPENGL_ES2
    // Avoid using GL Shaders by default since it causes so many problems with broken drivers.
    useGLShaders = confSettings->value("main/use_glshaders", false).toBool();
    useGLShaders = useGLShaders && QGLShaderProgram::hasOpenGLShaderPrograms();

    // We use OpenGL 2.1 features in our shaders
    useGLShaders = useGLShaders && (QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_2_1) || QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_ES_Version_2_0));
#else    
    useGLShaders = true;
#endif

    StelPainter::initSystemGLInfo();

    // Initialize AFTER creation of openGL context
    textureMgr = new StelTextureMgr();
    textureMgr->init();

    //#ifdef SVN_REVISION
    //    loadingBar = new StelLoadingBar(12., "textures/logo24bits.png", QString("SVN r%1").arg(SVN_REVISION), 25, 320, 101);
    //#else
    //    //ASAF
    //    //loadingBar = new StelLoadingBar(12., "textures/logo24bits.png", PACKAGE_VERSION, 45, 320, 121);
    //    loadingBar = new StelLoadingBar(12., "textures/logo24bits.png", PACKAGE_VERSION, 45, 320, 121);

    //#endif // SVN_RELEASE
    //    loadingBar->Draw(0);
    //#ifdef Q_OS_WIN
    //    Sleep(2000);
    //#elif defined Q_OS_LINUX
    //    sleep(2);
    //#endif

    networkAccessManager = new QNetworkAccessManager(this);
    // Activate http cache if Qt version >= 4.5
    QNetworkDiskCache* cache = new QNetworkDiskCache(networkAccessManager);
    QString cachePath = StelFileMgr::getCacheDir();

    qDebug() << "Cache directory is: " << QDir::toNativeSeparators(cachePath);
    cache->setCacheDirectory(cachePath);
    networkAccessManager->setCache(cache);
    connect(networkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(reportFileDownloadFinished(QNetworkReply*)));

    // Stel Object Data Base manager
    stelObjectMgr = new StelObjectMgr();
    stelObjectMgr->init();
    getModuleMgr().registerModule(stelObjectMgr);

    localeMgr = new StelLocaleMgr();
    skyCultureMgr = new StelSkyCultureMgr();
    planetLocationMgr = new StelLocationMgr();

    localeMgr->init();

    // Init the solar system first
    SolarSystem* ssystem = new SolarSystem();
    ssystem->init();
    getModuleMgr().registerModule(ssystem);

    // Load hipparcos stars & names
    StarMgr* hip_stars = new StarMgr();
    hip_stars->init();
    getModuleMgr().registerModule(hip_stars);

    core->init();

    // Init nebulas
    NebulaMgr* nebulas = new NebulaMgr();
    nebulas->init();
    getModuleMgr().registerModule(nebulas);

    // Init milky way
    MilkyWay* milky_way = new MilkyWay();
    milky_way->init();
    getModuleMgr().registerModule(milky_way);

    // Init sky image manager
    skyImageMgr = new StelSkyLayerMgr();
    skyImageMgr->init();
    getModuleMgr().registerModule(skyImageMgr);

    // Init audio manager
    audioMgr = new StelAudioMgr();

    // Telescope manager
    TelescopeMgr* telescope_mgr = new TelescopeMgr();
    telescope_mgr->init();
    getModuleMgr().registerModule(telescope_mgr);

    // Constellations
    ConstellationMgr* asterisms = new ConstellationMgr(hip_stars);
    asterisms->init();
    getModuleMgr().registerModule(asterisms);

    // Landscape, atmosphere & cardinal points section
    LandscapeMgr* landscape = new LandscapeMgr();
    landscape->init();
    getModuleMgr().registerModule(landscape);

    GridLinesMgr* gridLines = new GridLinesMgr();
    gridLines->init();
    getModuleMgr().registerModule(gridLines);

    // Meteors
    MeteorMgr* meteors = new MeteorMgr(10, 60);
    meteors->init();
    getModuleMgr().registerModule(meteors);

    // User labels
    LabelMgr* skyLabels = new LabelMgr();
    skyLabels->init();
    getModuleMgr().registerModule(skyLabels);

    // Scripting images
    ScreenImageMgr* scriptImages = new ScreenImageMgr();
    scriptImages->init();
    getModuleMgr().registerModule(scriptImages);

    //Presenting image and videos - ASAF
    StelPresentMgr* skyPresents= new StelPresentMgr();
    skyPresents->init();
    getModuleMgr().registerModule(skyPresents);

    skyCultureMgr->init();

    // Initialisation of the color scheme
    flagNightVision=true;  // fool caching
    setVisionModeNight(false);
    setVisionModeNight(confSettings->value("viewing/flag_night").toBool());

    // Proxy Initialisation
    QString proxyName = confSettings->value("proxy/host_name").toString();
    QString proxyUser = confSettings->value("proxy/user").toString();
    QString proxyPassword = confSettings->value("proxy/password").toString();
    QVariant proxyPort = confSettings->value("proxy/port");

    if (proxyName!="" && !proxyPort.isNull()){

        QNetworkProxy proxy(QNetworkProxy::HttpProxy);
        proxy.setHostName(proxyName);
        proxy.setPort(proxyPort.toUInt());
        if(proxyUser!="" && proxyPassword!="") {
            proxy.setUser(proxyUser);
            proxy.setPassword(proxyPassword);
        }
        QNetworkProxy::setApplicationProxy(proxy);
    }

    //updateI18n();

    initialized = true;
    delete loadingBar;
    loadingBar=NULL;
    StelMainGraphicsView::getInstance().getOpenGLWin()->updateGL();

    m_rsync = new rsync();
    if(StelMainWindow::getInstance().getIsServer())
        m_rsync->initServer();
    else
        m_rsync->initClient();

    showPropGui = confSettings->value("tui/flag_properties_gui",false).toBool();
    show_only_this_propgui = confSettings->value("tui/flag_only_this_propgui",false).toBool();

    isAdvancedMode = confSettings->value("main/flag_advanced",false).toBool();

    if (StelMainWindow::getInstance().getIsServer())
    {
        int console_h = 1;
        if (conf->value("video/console_h", -1).toInt() == -1)
            console_h = conf->value("video/screen_h", 720).toInt()-1;// -1, çünkü fullscreen oluyor
        else
            console_h = conf->value("video/console_h", 720).toInt()-1;

        if (!StelMainWindow::getInstance().getIsMultiprojector())
        {    StelMainWindow::getInstance().setGeometry(0,
                                                       0,
                                                       conf->value("video/screen_l", 800).toInt(),
                                                       console_h);
        }
        else
        {
            QDesktopWidget widget;
            QRect mainScreenSize  = QApplication::desktop()->availableGeometry(widget.primaryScreen());
            StelMainWindow::getInstance().setGeometry(mainScreenSize);
        }

        StelMainGraphicsView::getInstance().setGeometry(0,  // Preview Kontrol Ekraný Left
                                                        0,    // Preview Kontrol ekraný Top
                                                        512,  // Preview Kontrol ekraný Width
                                                        512); // Preview Kontrol ekraný Height
        if (!StelMainWindow::getInstance().getIsMultiprojector())
            ShiraPlayerForm::getInstance().setGeometry(0,
                                                       0,
                                                       conf->value("video/screen_l", 800).toInt(),
                                                       console_h);
        else
        {
            QDesktopWidget widget2;
            QRect mainScreenSize2  = QApplication::desktop()->availableGeometry(widget2.primaryScreen());
            ShiraPlayerForm::getInstance().setGeometry(mainScreenSize2);
        }
    }
    else
    {
        StelMainGraphicsView::getInstance().setGeometry(0,0,
                                                        conf->value("video/screen_w", 800).toInt(),
                                                        conf->value("video/screen_h", 600).toInt());

        StelMainWindow::getInstance().setWindowTitle("ShiraPlayer Projector");
    }

    if (!StelMainWindow::getInstance().getIsMultiprojector())
    {
        //Spheric mirror set
        setspUseCustomdata(confSettings->value("spheric_mirror/use_custom_data",false).toBool());
        setcustomDistortDataFile(confSettings->value("spheric_mirror/custom_distortion_file","").toString());
        setDistortHorzMirror(confSettings->value("spheric_mirror/custom_dist_horz_mirror",false).toBool());
        setDistortVertMirror(confSettings->value("spheric_mirror/custom_dist_vert_mirror",false).toBool());
    }

    //Present iþlemleri
    presentID = 0;

}

// Load and initialize external modules (plugins)
void StelApp::initPlugIns()
{
    // Load dynamically all the modules found in the modules/ directories
    // which are configured to be loaded at startup
    foreach (StelModuleMgr::PluginDescriptor i, moduleMgr->getPluginsList())
    {
        if (i.loadAtStartup==false)
            continue;
        StelModule* m = moduleMgr->loadPlugin(i.info.id);
        if (m!=NULL)
        {
            moduleMgr->registerModule(m, true);
            m->init();
        }
    }
}

void StelApp::update(double deltaTime)
{
    //ASAF

    if(isDiscreteTimeSteps)
    {
        this->getCore()->getNavigator()->setZeroTimeSpeed();
        switch (this->m_discrete)
        {
        case DiscreteTimeStepsSeconds:
        {
            this->getCore()->getNavigator()->addSolarDays(m_timedirection* 1.0/24.0/3600.0);
            break;
        }
        case DiscreteTimeStepsMinutes:
        {
            this->getCore()->getNavigator()->addSolarDays(m_timedirection*1.0/24/60);
            break;
        }

        case DiscreteTimeStepsHours:
        {
            this->getCore()->getNavigator()->addSolarDays(m_timedirection*1.0/24);
            break;
        }
        case DiscreteTimeStepsDays:
        {
            this->getCore()->getNavigator()->addSolarDays(m_timedirection*1.0);
            break;
        }
        case DiscreteTimeStepsYears:
        {
            this->getCore()->getNavigator()->addSolarDays(m_timedirection*365.25);
            break;
        }
        }
    }


    //ASAF

    if (!initialized)
        return;

    ++frame;
    timefr+=deltaTime;
    if (timefr-timeBase > 1.)
    {
        // Calc the FPS rate every seconds
        fps=(double)frame/(timefr-timeBase);
        frame = 0;
        timeBase+=1.;
    }

    core->update(deltaTime);

    moduleMgr->update();

    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionUpdate))
    {
        if(isVideoMode)
        {
            if(i->inherits("LandscapeMgr"))
            {
                i->update(deltaTime);
                //return;
            }
        }
        else
            i->update(deltaTime);
    }
    if(!isVideoMode )
        stelObjectMgr->update(deltaTime);
}

//! Iterate through the drawing sequence.
bool StelApp::drawPartial()
{

    if (drawState == 0)
    {
        if (!initialized)
            return false;

        core->preDraw();
        drawState = 1;
        return true;
    }

    const QList<StelModule*> modules = moduleMgr->getCallOrders(StelModule::ActionDraw);
    int index = drawState - 1;
    if (index < modules.size())
    {
        //ASAF
        if(!isVideoMode )
        {
            if (modules[index]->drawPartial(core))
                return true;
            else
                StelApp::getInstance().getCore()->drawPaintFree();

        }
        else
        {
            if (modules[index]->inherits("LandscapeMgr"))
            {
                //drawState = modules.size()+1;
                drawState++;
                if (modules[index]->drawPartial(core))
                    return true;
            }
        }
        drawState++;
        return true;
    }

    core->postDraw();

    drawState = 0;
    return false;
}

//! Main drawing function called at each frame
void StelApp::draw()
{
    Q_ASSERT(drawState == 0);
    while (drawPartial()) {}
    Q_ASSERT(drawState == 0);
}

/*************************************************************************
 Call this when the size of the GL window has changed
*************************************************************************/
void StelApp::glWindowHasBeenResized(float x, float y, float w, float h)
{

    if (core)
        core->windowHasBeenResized(x, y, w, h);
    else
    {
        saveProjW = w;
        saveProjH = h;
    }
}

// Handle mouse clics
void StelApp::handleClick(QMouseEvent* event)
{
    event->setAccepted(false);
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleMouseClicks))
    {
        if (StelMainWindow::getInstance().getIsMultiprojector())
        {
            double hScreen = StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->geometry().height();
            double wScreen = StelMainGraphicsView::getInstance().getStelAppGraphicsWidget()->geometry().width();

            double oranX = 1;
            double oranY = 1;
            if (wScreen / hScreen<1) oranY = 1 / (wScreen / hScreen);
            if (wScreen / hScreen>1) oranX = 1 / (wScreen / hScreen);

            double farkX = 0;
            double farkY = 0;
            double fark = wScreen - hScreen;

            double katsayi = 1;
            if (wScreen>hScreen) katsayi= viewportRes / hScreen /2.0 ;
            else katsayi = viewportRes / wScreen /2.0;

            if (fark<0) farkY = fark *katsayi;
            if (fark>0) farkX = fark *katsayi;

            QMouseEvent myEvent = QMouseEvent(event->type(),
                                              QPointF((event->localPos().x())/oranX - farkX,
                                                      (viewportRes-event->localPos().y())*oranY+farkY),
                                              event->screenPos(),
                                              event->button(),
                                              event->buttons(),
                                              event->modifiers());
            myEvent.setAccepted(false);
            i->handleMouseClicks(&myEvent);
            if (myEvent.isAccepted())
                return;
        }
        else
        {
            i->handleMouseClicks(event);
            if (event->isAccepted())
                return;
        }
    }
}

// Handle mouse wheel.
void StelApp::handleWheel(QWheelEvent* event)
{
    event->setAccepted(false);
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleMouseClicks))
    {
        i->handleMouseWheel(event);
        if (event->isAccepted())
            return;
    }
}

// Handle mouse move
void StelApp::handleMove(int x, int y, Qt::MouseButtons b)
{
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleMouseMoves))
    {
        if (StelMainWindow::getInstance().getIsMultiprojector())
        {
            if (i->handleMouseMoves(x, viewportRes-y, b))
                return;
        }
        else
        {
            if (i->handleMouseMoves(x, y, b))
                return;
        }
    }
}

// Handle key press and release
void StelApp::handleKeys(QKeyEvent* event)
{
    event->setAccepted(false);
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleKeys))
    {
        i->handleKeys(event);
        if (event->isAccepted())
            return;
    }
}


// Set the colorscheme for all the modules
void StelApp::setColorScheme(const QString& section)
{
    if (!currentStelStyle)
        currentStelStyle = new StelStyle;

    currentStelStyle->confSectionName = section;

    QString qtStyleFileName;
    QString htmlStyleFileName;

    if (section=="night_color")
    {
        qtStyleFileName = "data/gui/nightStyle.css";
        htmlStyleFileName = "data/gui/nightHtml.css";
    }
    else if (section=="color")
    {
        qtStyleFileName = "data/gui/normalStyle.css";
        htmlStyleFileName = "data/gui/normalHtml.css";
    }

    // Load Qt style sheet
    QString styleFilePath;
    try
    {
        styleFilePath = StelFileMgr::findFile(qtStyleFileName);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: can't find Qt style sheet:" << qtStyleFileName;
    }
    QFile styleFile(styleFilePath);
    styleFile.open(QIODevice::ReadOnly);
    currentStelStyle->qtStyleSheet = styleFile.readAll();

    // Load HTML style sheet
    try
    {
        styleFilePath = StelFileMgr::findFile(htmlStyleFileName);
    }
    catch (std::runtime_error& e)
    {
        qWarning() << "WARNING: can't find css:" << htmlStyleFileName;
    }
    QFile htmlStyleFile(styleFilePath);
    htmlStyleFile.open(QIODevice::ReadOnly);
    currentStelStyle->htmlStyleSheet = htmlStyleFile.readAll();

    // Send the event to every StelModule
    foreach (StelModule* iter, moduleMgr->getAllModules())
    {
        iter->setStelStyle(*currentStelStyle);
    }
    if (getGui())
        getGui()->setStelStyle(*currentStelStyle);
}

//! Set flag for activating night vision mode
void StelApp::setVisionModeNight(bool b)
{
    if (flagNightVision!=b)
    {
        flagNightVision=b;
        setColorScheme(b ? "night_color" : "color");
    }
}

// Update translations and font for sky everywhere in the program
void StelApp::updateI18n()
{
    // Send the event to every StelModule
    foreach (StelModule* iter, moduleMgr->getAllModules())
    {
        iter->updateI18n();
    }
    if (getGui())
    {
        getGui()->updateI18n();
    }
}

// Update and reload sky culture informations everywhere in the program
void StelApp::updateSkyCulture()
{
    QString skyCultureDir = getSkyCultureMgr().getCurrentSkyCultureID();
    // Send the event to every StelModule
    foreach (StelModule* iter, moduleMgr->getAllModules())
    {
        iter->updateSkyCulture(skyCultureDir);
    }
}

// Return the time since when stellarium is running in second.
double StelApp::getTotalRunTime()
{
    return (double)(StelApp::qtime->elapsed())/1000.;
}


void StelApp::reportFileDownloadFinished(QNetworkReply* reply)
{
    bool fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool();
    if (fromCache)
    {
        ++nbUsedCache;
        totalUsedCacheSize+=reply->bytesAvailable();
    }
    else
    {
        ++nbDownloadedFiles;
        totalDownloadedSize+=reply->bytesAvailable();
    }
}

void StelApp::makeMainGLContextCurrent()
{
    StelMainGraphicsView::getInstance().makeGLContextCurrent();
}

//ASAF
void StelApp::addNetworkCommand(QString cmd)
{
    //    QMessageBox::critical(0,"server",cmd+
    //                          "-"+QString::number(isScriptConsole)+
    //                          "-"+QString::number(isLiveMode)+
    //                          "-"+QString::number(isNetCom)+
    //                          "-"+QString::number(StelMainWindow::getInstance().getIsServer()),0,0);
    if(!isScriptConsole)
    {
        if((isLiveMode) || (isTimeCommand))
        {
            StelScriptMgr& scriptMgr = StelMainGraphicsView::getInstance().getScriptMgr();
            if (!scriptMgr.scriptIsRunning())
                getRsync()->sendChanges(RSYNC_COMMAND_LIVE_SCRIPT,cmd);
        }
        else
        {
            if((isNetCom)&&(StelMainWindow::getInstance().getIsServer()))
            {
                nc.addCommand(cmd);
            }
        }
    }
}


void StelApp::setFlyBy(const QString &planetFlyByname, double fark, bool isInner)
{
    if (isSettingFlyBy) return;

    isSettingFlyBy = true;
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);

    ssmgr->setFlagPlanetsScale(false);
    Sleep(100);

    Vec3d aim= Vec3d();
    if (planetFlyByname == "FLYBY-SS")
    {
        if (!isFlybySS)
        {
            StelUtils::spheToRect(M_PI_2,-M_PI_4,aim);
            mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 0);
        }
    }
    else
    {
        StelUtils::spheToRect(M_PI_2,0,aim);
        mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 0);
    }
    StelLocation loc = StelApp::getInstance().getCore()->getNavigator()->getCurrentLocation();
    loc.planetName = planetFlyByname;

    isFlybySS = (planetFlyByname == "FLYBY-SS");
    if (planetFlyByname == "FLYBY-SS")
    {
        // Ýç gezegenlerin alçak uçuþ görünümü için Güneþ merkezinden olan uzaklýk AU cinsinden
        //Mars ýn güneþe uzaklýðý referans alýnýyor
        const double innerFlybyfactor = 358;
        //Plutonun güneþe olan uzaklýðý referans alýnýyor
        const double outerFlybyFactor = 10532;

        PlanetP p = ssmgr->searchByEnglishNameForFlyby(planetFlyByname);
        PlanetP pmain = p->getFlybyPlanet();

        if (isInner)
        {
            p->setRadiusforFader(pmain->getRadius() * innerFlybyfactor);
            p->setRadiusFader(true);
            p->setflybFactor(innerFlybyfactor);
        }
        else
        {
            p->setRadiusforFader(pmain->getRadius() * outerFlybyFactor);
            p->setRadiusFader(true);
            p->setflybFactor(outerFlybyFactor);
        }
    }
    StelApp::getInstance().getCore()->getNavigator()->moveObserverTo(loc, 4.0, 4.0,true);

    //-----

    PlanetP p = ssmgr->searchByEnglishNameForFlyby(planetFlyByname);
    PlanetP pmain = p->getFlybyPlanet();

    fark = (p->getRadius()- pmain->getRadius()) * AU;
    fark = fark - pmain->getRadius()*(p->getflybyFactor()-1)*AU ;

    isSettingFlyBy = false;
}

void StelApp::prepareFlyBy()
{
    StelMainGraphicsView::getInstance().getScriptMgr().getMainApi()->clear("spaceship");
    startedFlyby = true;
}

void StelApp::setAltitudebyConsole(QString pname, double slidervalue, double slidermaxvalue)
{
    SolarSystem* ssmgr = GETSTELMODULE(SolarSystem);
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);

    PlanetP p = ssmgr->searchByEnglishNameForFlyby(pname);
    PlanetP pmain = p->getFlybyPlanet();

    double newRadius = p.data()->getflybyFactor() * pmain.data()->getRadius() +
            slidervalue / AU ;

    if (p != NULL)
    {
        p.data()->setRadiusforFader(newRadius);
        p.data()->setRadiusFader(true);
        //ui->lblAltitude->setText(QString("Altitude: %0 km").arg( ((p.data()->getflybyFactor() -1) * pmain.data()->getRadius() * AU )+ ui->altitudeSlider->value()));

        double slope = 1.;
        if(p->getEnglishName()=="FLYBY-SS")
            slope = (- M_PI_4) +( -1 * slidervalue * M_PI_4 * (44.0 / 45.0)  / slidermaxvalue);
        else
            slope = -1 * slidervalue * M_PI_2 * (88.0 / 90.0)  / slidermaxvalue;

        Vec3d aim;
        StelUtils::spheToRect(M_PI_2,slope,aim);

        mvmgr->moveToJ2000(StelApp::getInstance().getCore()->getNavigator()->altAzToJ2000(aim), 1);
    }
}


void StelApp::goHome()
{
    isFlybySS = false;
    getCore()->getNavigator()->setTimeNow();
    double s = m_timedirection*JD_SECOND;
    getCore()->getNavigator()->setTimeRate(s);

    //getCore()->getNavigator()->init();
    //----
    QSettings* conf = StelApp::getInstance().getSettings();
    QString defaultLocationID = conf->value("init_location/location","Bursa, Turkey").toString();
    getCore()->getNavigator()->setStelObserver(new StelObserver(StelApp::getInstance().getLocationMgr().locationForSmallString(defaultLocationID)));
    // Compute transform matrices between coordinates systems
    getCore()->getNavigator()->updateTransformMatrices();
    //----

    StelMainGraphicsView::getInstance().getScriptMgr().getMainApi()->clear("returnback");

    Vec3d init = core->movementMgr->getInitViewingDirection();
    core->movementMgr->moveToJ2000(core->getNavigator()->altAzToJ2000(init), 1, 0);
    LandscapeMgr* lmgr = GETSTELMODULE(LandscapeMgr);
    lmgr->setCurrentLandscapeID(lmgr->defaultLandscapeID, true);

    startedFlyby = false;
}


//--Messier Catalog region
messierimage *StelApp::getimageItem(QString id)
{
    foreach(messierimagePtr m, messierList)
    {
        if (m.data()->id == id) return m.data();
    }
}

messierimage *StelApp::getimageItemByIndex(int index)
{
    if (index < 0) return NULL;
    if (index > messierList.count()-1) return NULL;
    return messierList[index].data();
}

int StelApp::getimageIndex(messierimage *image)
{
    for (int i = 0 ; i < messierList.count();i++)
    {
        if (messierList[i].data() == image) return i;
    }
}

messierimage* StelApp::addMessierObject(QString id, int iwidth, int iheight, QString btnName)
{
    StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    QString strFile = StelFileMgr::getInstallationDir()+"/catalogs/messiers/"+id;
    double aspectRatio =  (double)iwidth / (double) iheight;
    messierimage* i = new messierimage();
    i->id = id;
    i->filepath = strFile;
    i->ra = 60.0;
    i->dec = 90.0;
    i->size = 190;
    i->rotate = 0.0;
    i->p1 = 2.5;
    i->p2 = 14.0;
    i->width = iwidth;
    i->height = iheight;
    i->aspectratio = aspectRatio;
    i->animMode = 1;
    i->btnName = btnName;

    sPmgr->loadPresentImage(i->id, i->filepath,
                            i->ra,  i->dec, i->size,
                            i->rotate, i->p1, i->p2,
                            true, true, i->aspectratio);

    messierList.append(messierimagePtr(i));
    i->startTimer();

    if (getimageItemByIndex(messierList.count()-2) != NULL)
    {
        getimageItemByIndex(messierList.count()-2)->animMode = 2;
        getimageItemByIndex(messierList.count()-2)->dec = 20;
        getimageItemByIndex(messierList.count()-2)->size = 2500;
        getimageItemByIndex(messierList.count()-2)->startTimer();
        if (getimageItemByIndex(messierList.count()-3) != NULL)
        {
            getimageItemByIndex(messierList.count()-3)->animMode = 3;
            getimageItemByIndex(messierList.count()-3)->ra = 0;
            getimageItemByIndex(messierList.count()-3)->startTimer();
            if (getimageItemByIndex(messierList.count()-4) != NULL)
            {
                getimageItemByIndex(messierList.count()-4)->ra = -60;
                sPmgr->removeWithFade(getimageItemByIndex(messierList.count()-4)->id);
                return getimageItemByIndex(messierList.count()-4);
            }
        }
    }
    return NULL;

}

void StelApp::removeMessierObject(const QString &messier)
{
    StelPresentMgr* sPmgr = GETSTELMODULE(StelPresentMgr);
    sPmgr->removeWithFade(messier);
    messierimage* mitem = getimageItem(messier);
    messierList.remove(getimageIndex(mitem));
}
