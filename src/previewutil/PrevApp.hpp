#ifndef PREVAPP_HPP
#define PREVAPP_HPP

#include <QString>
#include <QVariant>
#include <QObject>
#include <QFile>

#include "prevappgraphicwidget.hpp"

// Predeclaration of some classes
class StelCore;
class SkyLocalizer;
class StelTextureMgr;
class StelObjectMgr;
class StelLocaleMgr;
class StelModuleMgr;
class StelSkyCultureMgr;
class QStringList;
class StelLoadingBar;
class QSettings;
class QNetworkAccessManager;
class StelStyle;
class QTime;
class StelLocationMgr;
class StelSkyLayerMgr;
class StelAudioMgr;
class QNetworkReply;
class StelGuiBase;
class StelGui;
//ASAF
class StelFontMgr;
//ASAF

class PrevApp : public QObject
{
    Q_OBJECT
public:
    //ASAF
    enum DiscreteTimeStepsType
    {
            DiscreteTimeStepsSeconds,
            DiscreteTimeStepsMinutes,
            DiscreteTimeStepsHours,
            DiscreteTimeStepsDays,
            DiscreteTimeStepsYears
    };
    ///

    friend class PrevAppGraphicsWidget;
    PrevApp(QObject *parent = NULL);
    virtual ~PrevApp();

    //! Initialize core and default modules.
    void init(QSettings* conf);

    static PrevApp& getInstance() {Q_ASSERT(singleton); return *singleton;}

    void updateI18n();

    StelModuleMgr& getModuleMgr() {return *moduleMgr;}

    //! Get the locale manager to use for i18n & date/time localization.
    //! @return the font manager to use for loading fonts.
    StelLocaleMgr& getLocaleMgr() {return *localeMgr;}

    //! Get the sky cultures manager.
    //! @return the sky cultures manager
    StelSkyCultureMgr& getSkyCultureMgr() {return *skyCultureMgr;}

    //! Get the texture manager to use for loading textures.
    //! @return the texture manager to use for loading textures.
    StelTextureMgr& getTextureManager() {return *textureMgr;}

    //! Get the Location manager to use for managing stored locations
    //! @return the Location manager to use for managing stored locations
    StelLocationMgr& getLocationMgr() {return *planetLocationMgr;}

    //! Get the StelObject manager to use for querying from all stellarium objects.
    //! @return the StelObject manager to use for querying from all stellarium objects 	.
    StelObjectMgr& getStelObjectMgr() {return *stelObjectMgr;}

    //! Get the StelObject manager to use for querying from all stellarium objects.
    //! @return the StelObject manager to use for querying from all stellarium objects 	.
    StelSkyLayerMgr& getSkyImageMgr() {return *skyImageMgr;}

    //! Get the core of the program.
    //! It is the one which provide the projection, navigation and tone converter.
    //! @return the StelCore instance of the program
    StelCore* getCore() {return core;}

    static void initStatic();

    void update(double deltaTime);

    void draw();
    bool drawPartial();

    DiscreteTimeStepsType m_discrete;
    bool isDiscreteTimeSteps;
    double m_timedirection;

    //Presentation default deðiþkenleri
    float presentRA,presentDec,presentRotate,presentSize;
    bool presentdomeORsky, presentVisible;
    int presentID;

    //! Handle mouse clics.
    void handleClick(class QMouseEvent* event);
    //! Handle mouse wheel.
    void handleWheel(class QWheelEvent* event);
    //! Handle mouse move.
    void handleMove(int x, int y, Qt::MouseButtons b);
    //! Handle key press and release.
    void handleKeys(class QKeyEvent* event);

    //! Set the colorscheme for all the modules
    void setColorScheme(const QString& section);


private:

    // The StelApp singleton
    static PrevApp* singleton;

    // The associated StelCore instance
    StelCore* core;

    //ASAF
    // Font manager for the application
    StelFontMgr* fontManager;
    //ASAF

    // Module manager for the application
    StelModuleMgr* moduleMgr;

    // Locale manager for the application
    StelLocaleMgr* localeMgr;

    // Sky cultures manager for the application
    StelSkyCultureMgr* skyCultureMgr;

    // Textures manager for the application
    StelTextureMgr* textureMgr;

    // Manager for all the StelObjects of the program
    StelObjectMgr* stelObjectMgr;

    // Manager for the list of observer locations on planets
    StelLocationMgr* planetLocationMgr;

    StelSkyLayerMgr* skyImageMgr;

    // Currently used StelStyle
    StelStyle* currentStelStyle;

    //! Define whether we use opengl shaders
    bool useGLShaders;

    QSettings* confSettings;

    float fps;
    int frame;
    double timefr, timeBase;		// Used for fps counter

public:
    bool initialized;
private:
    static QTime* qtime;

    // Temporary variables used to store the last gl window resize
    // if the core was not yet initialized
    int saveProjW;
    int saveProjH;

public:
    //! The state of the drawing sequence
    int drawState;

};

#endif // PREVAPP_HPP
