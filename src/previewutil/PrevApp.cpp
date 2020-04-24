#include "PrevApp.hpp"

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
#include "PrevPainter.hpp"
//ASAF
#include "socketutils/mysocket.h"
#include "socketutils/rsync.h"
#include "StelFontMgr.hpp"
#include "StelNavigator.hpp"
#include "shiraplayerform.hpp"
#include "StelMainWindow.hpp"
#include "StelPresentMgr.hpp"
//ASAF
#include <iostream>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMouseEvent>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QSysInfo>
#include <QNetworkProxy>
#include <QMessageBox>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QtPlugin>

// Initialize static variables
PrevApp* PrevApp::singleton = NULL;
QTime* PrevApp::qtime = NULL;

void PrevApp::initStatic()
{
    PrevApp::qtime = new QTime();
    PrevApp::qtime->start();
}

PrevApp::PrevApp(QObject *parent) :
        QObject(parent), core(NULL),
        confSettings(NULL), initialized(false), saveProjW(-1), saveProjH(-1), drawState(0)
{
    setObjectName("PrevApp");

    skyCultureMgr=NULL;
    localeMgr=NULL;
    stelObjectMgr=NULL;
    textureMgr=NULL;
    moduleMgr=NULL;

    Q_ASSERT(!singleton);
    singleton = this;

    moduleMgr = new StelModuleMgr();

    // Init a default StelStyle, before loading modules, it will be overrided
    currentStelStyle = NULL;
    setColorScheme("color");

    //    //ASAF
    //    isLiveMode = false;
    //    isNetCom = false;
    //    isScriptConsole = false;
    //    isVideoMode = false;
    //    isVideoLandscape = false;
    //    isDiscreteTimeSteps = false;
    //    m_timedirection = 1.0;


}

PrevApp::~PrevApp()
{
    stelObjectMgr->unSelect();
    moduleMgr->unloadModule("StelSkyLayerMgr", false);  // We need to delete it afterward
    moduleMgr->unloadModule("StelObjectMgr", false);// We need to delete it afterward
    StelModuleMgr* tmp = moduleMgr;
    moduleMgr = new StelModuleMgr(); // Create a secondary instance to avoid crashes at other deinit
    delete tmp; tmp=NULL;
    delete core; core=NULL;
    delete skyCultureMgr; skyCultureMgr=NULL;
    delete localeMgr; localeMgr=NULL;
    delete stelObjectMgr; stelObjectMgr=NULL; // Delete the module by hand afterward
    delete textureMgr; textureMgr=NULL;
    delete planetLocationMgr; planetLocationMgr=NULL;
    delete moduleMgr; moduleMgr=NULL; // Delete the secondary instance
    delete currentStelStyle;

    Q_ASSERT(singleton);
    singleton = NULL;
}
// Handle mouse clics
void PrevApp::handleClick(QMouseEvent* event)
{
    event->setAccepted(false);
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleMouseClicks))
    {
        i->handleMouseClicks(event);
        if (event->isAccepted())
            return;
    }
}

// Handle mouse wheel.
void PrevApp::handleWheel(QWheelEvent* event)
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
void PrevApp::handleMove(int x, int y, Qt::MouseButtons b)
{
    // Send the event to every StelModule
    foreach (StelModule* i, moduleMgr->getCallOrders(StelModule::ActionHandleMouseMoves))
    {
        if (i->handleMouseMoves(x, y, b))
            return;
    }
}

// Handle key press and release
void PrevApp::handleKeys(QKeyEvent* event)
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
void PrevApp::setColorScheme(const QString& section)
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

}

void PrevApp::init(QSettings* conf)
{
    confSettings = conf;

    core = new StelCore();
    if (saveProjW!=-1 && saveProjH!=-1)
        core->windowHasBeenResized(0, 0, saveProjW, saveProjH);
    //ASAF
    fontManager = new StelFontMgr();
    //ASAF

#ifndef USE_OPENGL_ES2
    // Avoid using GL Shaders by default since it causes so many problems with broken drivers.
    useGLShaders = confSettings->value("main/use_glshaders", false).toBool();
    useGLShaders = useGLShaders && QGLShaderProgram::hasOpenGLShaderPrograms();

    // We use OpenGL 2.1 features in our shaders
    useGLShaders = useGLShaders && (QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_Version_2_1) || QGLFormat::openGLVersionFlags().testFlag(QGLFormat::OpenGL_ES_Version_2_0));
#else
    useGLShaders = true;
#endif

    //PrevPainter::initSystemGLInfo();

    // Initialize AFTER creation of openGL context
    textureMgr = new StelTextureMgr();
    textureMgr->init();

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

    //Presenting image and videos - ASAF
    StelPresentMgr* skyPresents= new StelPresentMgr();
    skyPresents->init();
    getModuleMgr().registerModule(skyPresents);

    skyCultureMgr->init();

    updateI18n();

    initialized = true;

    // StelMainGraphicsView::getInstance().getOpenGLWin()->updateGL();// BURASI YAZILMALI


    //    StelMainWindow::getInstance().setGeometry(0,
    //                0,
    //                conf->value("video/screen_l", 800).toInt()+conf->value("video/screen_w", 800).toInt(),
    //                conf->value("video/screen_t", 0).toInt()+conf->value("video/screen_h", 600).toInt());
    //
    //    //Shira Player Tek projektör için ayarlar
    //    StelMainGraphicsView::getInstance().setGeometry(confSettings->value("video/screen_l", 800).toInt(),  // Projeksiyon Ekraný Left
    //                                  confSettings->value("video/screen_t", 0).toInt(),    // Projeksiyon ekraný Top
    //                                  confSettings->value("video/screen_w", 800).toInt(),  // Projeksiyon ekraný Width
    //                                  confSettings->value("video/screen_h", 600).toInt()); // Projeksiyon ekraný Height
    //
    //    ShiraPlayerForm::getInstance().setGeometry(0,0,confSettings->value("video/screen_l", 800).toInt(),// Kontrol Ekraný Width
    //                                                   confSettings->value("video/screen_h", 600).toInt()); // Kontrol Ekraný Height


    //Present iþlemleri
    presentID = 0;
}

void PrevApp::updateI18n()
{
    // Send the event to every StelModule
    foreach (StelModule* iter, moduleMgr->getAllModules())
    {
        iter->updateI18n();
    }
}
void PrevApp::update(double deltaTime)
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
        i->update(deltaTime);
    }
    stelObjectMgr->update(deltaTime);
}

//! Iterate through the drawing sequence.
bool PrevApp::drawPartial()
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
        if (modules[index]->drawPartial(core))
            return true;

        drawState++;
        return true;
    }

    core->postDraw();
    drawState = 0;
    return false;
}

//! Main drawing function called at each frame
void PrevApp::draw()
{
    Q_ASSERT(drawState == 0);
    while (drawPartial()) {}
    Q_ASSERT(drawState == 0);
}
