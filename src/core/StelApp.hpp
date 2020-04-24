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

#ifndef _STELAPP_HPP_
#define _STELAPP_HPP_

#include <QString>
#include <QVariant>
#include <QObject>
#include <QFile>

#include "socketutils/rsync.h"
#include "networkcommands.h"
#include "gui/splashscreen.h"

#include "catalogpages/messierimage.h"

//#include "recordutils/GLToMovie.h"

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

//! @class StelApp
//! Singleton main Stellarium application class.
//! This is the central class of Stellarium.  Only one singleton instance of
//! this class is created and can be accessed from anywhere else.  This class
//! is the access point to several "Manager" class which provide application-wide
//! services for managment of font, textures, localization, sky culture, and in
//! theory all other services used by the other part of the program.
//!
//! The StelApp class is also the one managing the StelModule in a generic manner
//! by calling their update, drawing and other methods when needed.
//! @author Fabien Chereau
class StelApp : public QObject
{
    Q_OBJECT

public:
    friend class StelAppGraphicsWidget;

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

    //! Create and initialize the main Stellarium application.
    //! @param argc The number of command line parameters
    //! @param argv an array of char* command line arguments
    //! @param parent the QObject parent
    //! The configFile will be search for in the search path by the StelFileMgr,
    //! it is therefor possible to specify either just a file name or path within the
    //! search path, or use a full path or even a relative path to an existing file
    StelApp(QObject* parent=NULL);

    //! Deinitialize and destroy the main Stellarium application.
    virtual ~StelApp();

    //! Initialize core and default modules.
    void init(QSettings* conf);

    //! Load and initialize external modules (plugins)
    void initPlugIns();

    //! Get the StelApp singleton instance.
    //! @return the StelApp singleton instance
    static StelApp& getInstance() {Q_ASSERT(singleton); return *singleton;}

    //! Get the module manager to use for accessing any module loaded in the application.
    //! @return the module manager.
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

    //! Get the audio manager
    StelAudioMgr* getStelAudioMgr() {return audioMgr;}

    //! Get the core of the program.
    //! It is the one which provide the projection, navigation and tone converter.
    //! @return the StelCore instance of the program
    StelCore* getCore() {return core;}

    //! Get the main loading bar used by modules for displaying loading informations.
    //! @return the main StelLoadingBar instance of the program.
    StelLoadingBar* getStelLoadingBar() {return loadingBar;}

    //! Get the font manager to use for loading fonts.
    //! @return the font manager to use for loading fonts.
    StelFontMgr& getFontManager() {return *fontManager;}

    //! Get the common instance of QNetworkAccessManager used in stellarium
    QNetworkAccessManager* getNetworkAccessManager() {return networkAccessManager;}

    //! Update translations, font for GUI and sky everywhere in the program.
    void updateI18n();

    //! Update and reload sky culture informations everywhere in the program.
    void updateSkyCulture();

    //! Return the main configuration options
    QSettings* getSettings() {return confSettings;}

    //! Return the currently used style
    const StelStyle* getCurrentStelStyle() {return currentStelStyle;}

    //! Update all object according to the deltaTime in seconds.
    void update(double deltaTime);

    //! Draw all registered StelModule in the order defined by the order lists.
    //! @return the max squared distance in pixels that any object has travelled since the last update.
    void draw();

    //! Iterate through the drawing sequence.
    //! This allow us to split the slow drawing operation into small parts,
    //! we can then decide to pause the painting for this frame and used the cached image instead.
    //! @return true if we should continue drawing (by calling the method again)
    bool drawPartial();

    //! Call this when the size of the GL window has changed.
    void glWindowHasBeenResized(float x, float y, float w, float h);

    //! Get the GUI instance implementing the abstract GUI interface.
    StelGuiBase* getGui() const {return stelGui;}
    //! Tell the StelApp instance which GUI si currently being used.
    //! The caller is responsible for destroying the GUI.
    void setGui(StelGuiBase* b) {stelGui=b;}

    //! Make sure that the GL context of the main window is current and valid.
    static void makeMainGLContextCurrent();

    static void initStatic();

    //! Get flag for using opengl shaders
    bool getUseGLShaders() const {return useGLShaders;}
    void setUseGLShaders(bool value) { useGLShaders = value;}

    //! ASAF client/server state
    rsync* getRsync(){return m_rsync;}

    NetworkCommands nc;
    bool isNetCom;
    bool isLiveMode;
    bool isTimeCommand;
    bool isScriptConsole;
    bool isAdvancedMode;

    void addNetworkCommand(QString cmd);

    //Tum cizimleri video modunda kontrol etmek icin
    bool isVideoMode;
    bool isVideoLandscape;

    DiscreteTimeStepsType m_discrete;
    bool isDiscreteTimeSteps;
    double m_timedirection;

    //ASAF
    //!Projektor de Width ve Height FOV ayarlamak i�in (Sadece Steregraphic projekt�rde kullan�lacak)
    float fov_height_rate;
    float fov_width_rate;
    void set_fwr(float fwr)
    {
        if (fwr>1)
            fwr = 1.0;
        if (fwr<0.1)
            fwr = 0.1;
        fov_width_rate = fwr;
    }
    void set_fhr(float fhr)
    {
        if (fhr>1)
            fhr = 1.0;
        if (fhr<0.1)
            fhr = 0.1;
        fov_height_rate = fhr;
    }

    //Information Yazilarini yazdirmak icin
    bool showPropGui;
    bool show_only_this_propgui;

    //Presentation default degiskenleri
    float presentRA,presentDec,presentRotate,presentSize;
    int presentCon,presentBri,presentSat;
    bool presentdomeORsky, presentVisible;
    int presentID;

    //Spherical mirror spec
    void setspUseCustomdata(bool val){ spheric_usecustomdata= val;}
    bool getspUseCustomData(){ return spheric_usecustomdata ;}

    void setcustomDistortDataFile(QString val) { customDistortData = val;}
    QString getcustomDistortionFile(){ return customDistortData;}

    void setDistortHorzMirror(bool val){ disthorzmirror = val;}
    bool getDistortHorzMirror(){ return disthorzmirror;}

    void setDistortVertMirror(bool val){ distvertmirror = val;}
    bool getDistortVertMirror(){ return distvertmirror;}

    //
    void setAllowFreeHand(bool val) { allowFreeHand = val; }
    bool getAllowFreeHand() { return allowFreeHand;}

    void setAllowFreeHandDel(bool val) { allowFreeHandDel = val; }
    bool getAllowFreeHandDel() { return allowFreeHandDel;}


    //FLYBY
    void setFlyBy(const QString& planetFlyByname, double fark, bool isInner);
    void goHome();
    void prepareFlyBy();
    bool isSettingFlyBy = false;
    bool isFlybySS = false;
    bool startedFlyby = false;
    bool getstartedFlyby() { return startedFlyby; }
    void setAltitudebyConsole(QString pname,double slidervalue,double slidermaxvalue);
    //Multi projector
    double viewportRes;
    bool framelessProjWindow;

    //Messier Catalog
    messierimage* addMessierObject(QString id, int iwidth, int iheight, QString btnName);
    void removeMessierObject(const QString& messier);
    QVector<messierimagePtr> messierList;
    messierimage *getimageItem(QString id);
    messierimage *getimageItemByIndex(int index);
    int getimageIndex(messierimage* image);

    //Constellation Window
    bool usingConstModule = false;

    ///////////////////////////////////////////////////////////////////////////
    // Scriptable methods
public slots:

    //! Set flag for activating night vision mode.
    void setVisionModeNight(bool);
    //! Get flag for activating night vision mode.
    bool getVisionModeNight() const {return flagNightVision;}

    //! Get the current number of frame per second.
    //! @return the FPS averaged on the last second
    float getFps() const {return fps;}

    //! Return the time since when stellarium is running in second.
    static double getTotalRunTime();

    //! Report that a download occured. This is used for statistics purposes.
    //! Connect this slot to QNetworkAccessManager::finished() slot to obtain statistics at the end of the program.
    void reportFileDownloadFinished(QNetworkReply* reply);

private:

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


    // The StelApp singleton
    static StelApp* singleton;

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

    // Main network manager used for the program
    QNetworkAccessManager* networkAccessManager;

    // The audio manager.  Must execute in the main thread.
    StelAudioMgr* audioMgr;

    // The main loading bar
    StelLoadingBar* loadingBar;

    // Currently used StelStyle
    StelStyle* currentStelStyle;

    StelSkyLayerMgr* skyImageMgr;

    StelGuiBase* stelGui;

    float fps;
    int frame;
    double timefr, timeBase;		// Used for fps counter

    //! Define whether we are in night vision mode
    bool flagNightVision;

    //! Define whether we use opengl shaders
    bool useGLShaders;

    QSettings* confSettings;

    bool allowFreeHand;
    bool allowFreeHandDel;

    // Define whether the StelApp instance has completed initialization
public:
    bool initialized;
private:
    static QTime* qtime;

    // Temporary variables used to store the last gl window resize
    // if the core was not yet initialized
    int saveProjW;
    int saveProjH;

    //! Store the number of downloaded files for statistics.
    int nbDownloadedFiles;
    //! Store the the summed size of all downloaded files in bytes.
    qint64 totalDownloadedSize;

    //! Store the number of downloaded files read from the cache for statistics.
    int nbUsedCache;
    //! Store the the summed size of all downloaded files read from the cache in bytes.
    qint64 totalUsedCacheSize;
public:
    //! The state of the drawing sequence
    int drawState;
private:
    //ASAF
    rsync* m_rsync;
    bool spheric_usecustomdata;
    QString customDistortData;
    bool disthorzmirror;
    bool distvertmirror;
};

#endif // _STELAPP_HPP_
