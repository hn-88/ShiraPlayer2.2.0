/*
 * ShiraPlayer(TM)
 * Copyright (C) 2009 Matthew Gates
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

#ifndef _STELMAINSCRIPTAPI_HPP_
#define _STELMAINSCRIPTAPI_HPP_

#include "ScriptSleeper.hpp"

#include <QObject>
#include <QVariant>
#include <QStringList>

//! Provide script API for Stellarium global functions.  Public slots in this class
//! may be used in Stellarium scripts, and are accessed as member function to the
//! "core" scripting object.  Module-specific functions, such as setting and clearing
//! of display flags (e.g. LandscapeMgr::setFlagAtmosphere) can be accessed directly
//! via the scripting object with the class name, e.g.  by using the scripting command:
//!  LandscapeMgr.setFlagAtmosphere(true);
class StelMainScriptAPI : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double JDay READ getJDay WRITE setJDay)
    Q_PROPERTY(double timeSpeed READ getTimeRate WRITE setTimeRate)

public:
    StelMainScriptAPI(QObject *parent = 0);
    ~StelMainScriptAPI();

    ScriptSleeper& getScriptSleeper(void);

    // These functions will be available in scripts
public slots:
    //! Set the current date in Julian Day
    //! @param JD the Julian Date
    void setJDay(double JD);
    //! Get the current date in Julian Day
    //! @return the Julian Date
    double getJDay(void) const;

    //! set the date in ISO format, e.g. "2008-03-24T13:21:01"
    //! @param dt the date string to use.  Formats:
    //! - ISO, e.g. "2008-03-24T13:21:01"
    //! - "now" (set sim time to real time)
    //! - relative, e.g. "+ 4 days", "-2 weeks".  can use these
    //!   units: seconds, minutes, hours, days, weeks, months, years.
    //!   You may also append " sidereal" to use sidereal days and so on.
    //!   You can also use "now" at the start.  For example:
    //!   "now + 3 hours sidereal"
    //!   Note: you must use the plural all the time, even when the number
    //!   of the unit is 1.  i.e. use "+ 1 days" not "+1 day".
    //! Note: when sidereal time is used, the length of time for
    //! each unit is dependent on the current planet.  By contrast
    //! when sidereal timeis not specified (i.e. solar time is used)
    //! the value is conventional - i.e. 1 day means 1 Earth Solar day.
    //! @param spec "local" or "utc" - only has an effect when
    //! the ISO date type is used.
    void setDate(const QString& dt, const QString& spec="utc");

    //! get the simulation date and time as a string in ISO format,
    //! e.g. "2008-03-24T13:21:01"
    //! @param spec if "utc", the returned string's timezone is UTC,
    //! else it is local time.
    //! @return the current simulation time.
    QString getDate(const QString& spec="utc");

    //! Set time speed in JDay/sec
    //! @param ts the new rate of passage of time as a multiple of real time.
    //! For example if ts is 1, time will pass at the normal rate.  If ts == 10
    //! then simulation time will pass at 10 times the normal rate.
    //! If ts is negative, simulation time will go backwards.
    void setTimeRate(double ts);
    //! Get simulation time rate.
    //! @return time speed as a multiple of real time.
    double getTimeRate(void) const;

    //! Get the simulation time and rate state - is it "real time"
    //! @return true if the time rate is normal, and the simulation time
    //! is real time, else return false
    bool isRealTime();

    //! Set the simulation time to the current system time, and the time rate to 1
    void setRealTime();

    //! Pauses the script for t seconds
    //! @param t the number of seconds to wait
    void wait(double t);

    //! Waits until a specified simulation date/time.  This function
    //! will take into account the rate (and direction) in which simulation
    //! time is passing. e.g. if a future date is specified and the
    //! time is moving backwards, the function will return immediately.
    //! If the time rate is 0, the function will not wait.  This is to
    //! prevent infinite wait time.
    //! @param dt the date string to use
    //! @param spec "local" or "utc"
    void waitFor(const QString& dt, const QString& spec="utc");

    //! Select an object by name
    //! @param name the name of the object to select (english)
    //! If the name is "", any currently selected objects will be
    //! de-selected.
    //! @param pointer whether or not to have the selection pointer enabled
    void selectObjectByName(const QString& name, bool pointer=false);

    //! Fetch a map with data about an object's position, magnitude and so on
    //! @param name is the English name of the object for which data will be
    //! returned.
    //! @return a map of object data.  Keys:
    //! - altitude : altitude angle in decimal degrees
    //! - azimuth : azimuth angle in decimal degrees
    //! - ra : right ascension angle (current date frame) in decimal degrees
    //! - dec : declenation angle in (current date frame) decimal degrees
    //! - raJ2000 : right ascension angle (J2000 frame) in decimal degrees
    //! - decJ2000 : declenation angle in (J2000 frame) decimal degrees
    QVariantMap getObjectPosition(const QString& name);

    //! Clear the display options, setting a "standard" view.
    //! Preset states:
    //! - natural : azimuthal mount, atmosphere, landscape,
    //!   no lines, labels or markers
    //! - starchart : equatorial mount, constellation lines,
    //!   no landscape, atmoshere etc.  labels & markers on.
    //! @param state the name of a preset state.
    void clear(const QString& state="natural");

    //! Get the current viewing direction altitude angle at center of view.
    //! @return the altitude angle in decimal degrees.
    //! 0 is horizon, zenith is 180, nadir = -180.
    double getViewAltitudeAngle();

    //! Get the current viewing direction azimuth angle at center of view.
    //! @return the azimuth angle in decimal degrees as a compass bearing
    //! i.e.  0 is North, 90 is East, 180 is South, 270 is West.
    double getViewAzimuthAngle();

    //! Get the current viewing direction Right Ascension at center of view.
    //! @return the Right Ascension angle in decimal degrees.
    //! The value returned falls in the range 0 .. 360
    double getViewRaAngle();

    //! Get the current viewing direction Declination angle at center of view.
    //! @return the Declination angle in decimal degrees.
    //! The value returned falls in the range -180 .. 180
    double getViewDecAngle();

    //! Get the current viewing direction Right Ascension in J2000 frame at center of view.
    //! @return the Right Ascension angle in J2000 frame in decimal degrees.
    double getViewRaJ2000Angle();

    //! Get the current viewing direction Declination angle in J2000 frame at center of view.
    //! @return the Declination angle in J2000 frame in decimal degrees.
    double getViewDecJ2000Angle();

    //! move the current viewing direction to some specified altitude and azimuth
    //! angles may be specified in a format recognised by StelUtils::getDecAngle()
    //! @param alt the altitude angle
    //! @param azi the azimuth angle
    //! @param duration the duration of the movement in seconds
    void moveToAltAzi(const QString& alt, const QString& azi, float duration=1.);

    //! move the current viewing direction to some specified right ascension and declination
    //! angles may be specified in a format recognised by StelUtils::getDecAngle()
    //! @param ra the right ascension angle
    //! @param dec the declination angle
    //! @param duration the duration of the movement in seconds
    void moveToRaDec(const QString& ra, const QString& dec, float duration=1.);

    //! move the current viewing direction to some specified right ascension and declination in the J2000 frame
    //! angles may be specified in a format recognised by StelUtils::getDecAngle()
    //! @param ra the right ascension angle
    //! @param dec the declination angle
    //! @param duration the duration of the movement in seconds
    void moveToRaDecJ2000(const QString& ra, const QString& dec, float duration=1.);

    //! Set the observer location
    //! @param longitude the longitude in degrees. E is +ve.
    //!        values out of the range -180 .. 180 mean that
    //!        the longitude will not be set
    //! @param latitude the longitude in degrees. N is +ve.
    //!        values out of the range -180 .. 180 mean that
    //!        the latitude will not be set
    //! @param altitude the new altitude in meters.
    //!        values less than -1000 mean the altitude will not
    //!        be set.
    //! @param duration the time for the transition from the
    //!        old to the new location.
    //! @param name A name for the location (which will appear
    //!        in the status bar.
    //! @param planet the English name of the new planet.
    //!        If the planet name is not known (e.g. ""), the
    //!        planet will not be set.
    void setObserverLocation(double longitude, double latitude, double altitude, double duration=1., const QString& name="", const QString& planet="");

    //! Set the location by the name of the location.
    //! @param id the location ID as it would be found in the database
    //! of locations - do a search in the Location window to see what
    //! where is.  e.g. "York, UnitedKingdom".
    //! @param duration the number of seconds to take to move location.
    void setObserverLocation(const QString id, double duration=1.);

    //! Get the ID of the current observer location.
    //! @param duration the number of seconds to take to move location.
    QString getObserverLocation();

    //! Save a screenshot.
    //! @param prefix the prefix for the file name to use
    //! @param dir the path of the directory to save the screenshot in.  If
    //! none is specified, the default screenshot directory will be used.
    //! @param invert whether colors have to be inverted in the output image
    void screenshot(const QString& prefix, bool invert=false, const QString& dir="");

    //! Show or hide the GUI (toolbars).  Note this only applies to GUI plugins which
    //! provide the public slot "setGuiVisible(bool)".
    //! @param b if true, show the GUI, if false, hide the GUI.
    void setGuiVisible(bool b);

    //! Set the minimum frames per second.  Usually this minimum will
    //! be switched to after there are no user events for some seconds
    //! to save power.  However, if can be useful to set this to a high
    //! value to improve playing smoothness in scripts.
    //! @param m the new minimum fps setting.
    void setMinFps(float m);

    //! Get the current minimum frames per second.
    //! @return The current minimum frames per secon setting.
    float getMinFps();

    //! Set the maximum frames per second.
    //! @param m the new maximum fps setting.
    void setMaxFps(float m);

    //! Get the current maximum frames per second.
    //! @return The current maximum frames per secon setting.
    float getMaxFps();

    //! Get the mount mode as a string
    //! @return "equatorial" or "azimuthal"
    QString getMountMode();

    //! Set the mount mode
    //! @param mode should be "equatorial" or "azimuthal"
    void setMountMode(const QString& mode);

    //! Get the current status of Night Mode
    //! @return true if night mode is currently set
    bool getNightMode();

    //! Set the status of Night Mode
    //! @param b if true, set Night Mode, else set Normal Mode
    void setNightMode(bool b);

    //! Get the current projection mode ID string
    //! @return the string which identifies the current projection mode.
    //! For a list of possibl results, see setProjectionMode();
    QString getProjectionMode();

    //! Set the current projection mode
    //! @param id the name of the projection mode to use, e.g. "Perspective" and so on.
    //! valid values of id are:
    //! - ProjectionPerspective
    //! - ProjectionEqualArea
    //! - ProjectionStereographic
    //! - ProjectionFisheye
    //! - ProjectionHammer
    //! - ProjectionCylinder
    //! - ProjectionMercator
    //! - ProjectionOrthographic
    void setProjectionMode(const QString& id);

    //! Get the status of the disk viewport
    //! @return true if the disk view port is currently enabled
    bool getDiskViewport();

    //! Set the disk viewport
    //! @param b if true, sets the disk viewport on, else sets it off
    void setDiskViewport(bool b);

    //! Get a list of Sky Culture IDs
    //! @return a list of valid sky culture IDs
    QStringList getAllSkyCultureIDs(void);

    //! Find out the current sky culture
    //! @return the ID of the current sky culture (i.e. the name of the directory in
    //! which the curret sky cultures files are found, e.g. "western")
    QString getSkyCulture();

    //! Set the current sky culture
    //! @param id the ID of the sky culture to set, e.g. western or inuit etc.
    void setSkyCulture(const QString& id);

    //! Get the current status of the gravity labels option
    //! @return true if gravity labels are enabled, else false
    bool getFlagGravityLabels();

    //! Turn on/off gravity labels
    //! @param b if true, turn on gravity labels, else turn them off
    void setFlagGravityLabels(bool b);

    //! Load an image which will have sky coordinates.
    //! @param id a string ID to be used when referring to this
    //! image (e.g. when changing the displayed status or deleting
    //! it.
    //! @param filename the file name of the image.  If a relative
    //! path is specified, "scripts/" will be prefixed before the
    //! image is searched for using StelFileMgr.
    //! @param ra0 The right ascension of the first corner of the image in degrees
    //! @param dec0 The declenation of the first corner of the image in degrees
    //! @param ra1 The right ascension of the second corner of the image in degrees
    //! @param dec1 The declenation of the second corner of the image in degrees
    //! @param ra2 The right ascension of the third corner of the image in degrees
    //! @param dec2 The declenation of the third corner of the image in degrees
    //! @param ra3 The right ascension of the fourth corner of the image in degrees
    //! @param dec3 The declenation of the fourth corner of the image in degrees
    //! @param minRes The minimum resolution setting for the image
    //! @param maxBright The maximum brightness setting for the image
    //! @param visible The initial visibility of the image
    void loadSkyImage(const QString& id, const QString& filename,
                      double ra0, double dec0,
                      double ra1, double dec1,
                      double ra2, double dec2,
                      double ra3, double dec3,
                      double minRes=2.5, double maxBright=14, bool visible=true);

    //! Convenience function which allows the user to provide RA and DEC angles
    //! as strings (e.g. "12d 14m 8s" or "5h 26m 8s" - formats accepted by
    //! StelUtils::getDecAngle()).
    void loadSkyImage(const QString& id, const QString& filename,
                      const QString& ra0, const QString& dec0,
                      const QString& ra1, const QString& dec1,
                      const QString& ra2, const QString& dec2,
                      const QString& ra3, const QString& dec3,
                      double minRes=2.5, double maxBright=14, bool visible=true);

    //! Convenience function which allows loading of a sky image based on a
    //! central coordinate, angular size and rotation.
    //! @param id a string ID to be used when referring to this
    //! image (e.g. when changing the displayed status or deleting it.
    //! @param filename the file name of the image.  If a relative
    //! path is specified, "scripts/" will be prefixed before the
    //! image is searched for using StelFileMgr.
    //! @param ra The right ascension of the center of the image in J2000 frame degrees
    //! @param dec The declenation of the center of the image in J2000 frame degrees
    //! @param angSize The angular size of the image in arc minutes
    //! @param rotation The clockwise rotation angle of the image in degrees
    //! @param minRes The minimum resolution setting for the image
    //! @param maxBright The maximum brightness setting for the image
    //! @param visible The initial visibility of the image
    void loadSkyImage(const QString& id, const QString& filename,
                      double ra, double dec, double angSize, double rotation,
                      double minRes=2.5, double maxBright=14, bool visible=true);

    //! Convenience function which allows loading of a sky image based on a
    //! central coordinate, angular size and rotation.  Parameters are the same
    //! as the version of this function which takes double values for the
    //! ra and dec, except here text expressions of angles may be used.
    void loadSkyImage(const QString& id, const QString& filename,
                      const QString& ra, const QString& dec, double angSize, double rotation,
                      double minRes=2.5, double maxBright=14, bool visible=true);

    //! Remove a SkyImage.
    //! @param id the ID of the image to remove.
    void removeSkyImage(const QString& id);

    //! Load a sound from a file.
    //! @param filename the name of the file to load.
    //! @param id the identifier which will be used to refer to the sound
    //! when calling playSound, pauseSound, stopSound and dropSound.
    void loadSound(const QString& filename, const QString& id);

    //! Play a sound which has previously been loaded with loadSound
    //! @param id the identifier used when loadSound was called
    void playSound(const QString& id);

    //! Pause a sound which is playing.  Subsequent playSound calls will
    //! resume playing from the position in the file when it was paused.
    //! @param id the identifier used when loadSound was called
    void pauseSound(const QString& id);

    //! Stop a sound from playing.  This resets the position in the
    //! sound to the start so that subsequent playSound calls will
    //! start from the beginning.
    //! @param id the identifier used when loadSound was called
    void stopSound(const QString& id);

    //! Drop a sound from memory.  You should do this before the end
    //! of your script.
    //! @param id the identifier used when loadSound was called
    void dropSound(const QString& id);

    //! Get the screen width in pixels.
    //! @return The screen width in pixels
    int getScreenWidth(void);
    //! Get the screen height in pixels.
    //! @return The screen height in pixels
    int getScreenHeight(void);

    //! Get the script execution rate as a multiple of normal execution speed
    //! @return the current script execution rate.
    double getScriptRate(void);
    //! Set the script execution rate as a multiple of normal execution speed
    //! @param r the multiple of the normal script execution speed, i.e.
    //! if 5 is passed the script will execute 5 times faster than it would
    //! if the script rate was 1.
    void setScriptRate(double r);

    //! Set the amount of selected object information to display
    //! @param level, can be "AllInfo", "ShortInfo", "None"
    void setSelectedObjectInfo(const QString& level);

    //! stop the script
    void exit(void);

    //! Close Stellarium
    void quitStellarium(void);

    //! print a debugging message to the console
    //! @param s the message to be displayed on the console.
    void debug(const QString& s);

    //ASAF
    void setDiscreteTime(const QString& type);
    void setTimeNow();

    void SetFade(bool fader);
    void showDateTime(bool state);
    void showLocation(bool state);
    void showProperties(bool state);
    void showFPS(bool value);
    void selectObjectByEqPos(const QString& ra, const QString& dec, bool pointer);
    void selectObjectByI18Name(const QString& name, bool pointer);

    void loadVideo(const QString& filename);
    void playVideo(bool is_Audio);
    void pauseVideo(bool pause);
    void stopVideo();
    int  getTotalTimeVideo();
    void playAudio(const QString& filename);
    void stopAudio();
    int  getTotalTimeAudio();

    //! Convenience function which allows loading of a sky image based on a
    //! central coordinate, angular size and rotation.
    //! @param id a string ID to be used when referring to this
    //! image (e.g. when changing the displayed status or deleting it.
    //! @param filename the file name of the image.  If a relative
    //! path is specified, "scripts/" will be prefixed before the
    //! image is searched for using StelFileMgr.
    //! @param ra The right ascension of the center of the image in J2000 frame degrees
    //! @param dec The declenation of the center of the image in J2000 frame degrees
    //! @param angSize The angular size of the image in arc minutes
    //! @param rotation The clockwise rotation angle of the image in degrees
    //! @param minRes The minimum resolution setting for the image
    //! @param maxBright The maximum brightness setting for the image
    //! @param visible The initial visibility of the image
    //! @param screenORdome screenORdome
    //! @param aspectratio AspectRatio
    void loadNewPresentImage(const QString& id, const QString& filename,
                             double ra, double dec, double angSize, double rotation,
                             double minRes=2.5, double maxBright=14, bool visible=true,
                             bool screenORdome=false,double aspectratio=1.0);

    void loadNewPresentVideo(const QString& id, const QString& filename,
                             double ra, double dec, double angSize, double rotation,
                             double minRes=2.5, double maxBright=14, bool visible=true,
                             bool screenORdome=false,double aspectratio=1.0);

    void playPresentVideo(const QString& id, bool status);
    void pauseTogglePresentVideo(const QString& id);
    void removePresentVideo(const QString& id);

    void setPresentProperties(const QString& id, const QString& command);
    void setLayerdomeORsky(const QString& id,bool bscreen);

    void showPresentImage(const QString& id,bool visible);
    void mixPresentImage(const QString& id,bool value);

    //! Load an image which will have sky coordinates.
    //! @param id a string ID to be used when referring to this
    //! image (e.g. when changing the displayed status or deleting
    //! it.
    //! @param filename the file name of the image.  If a relative
    //! path is specified, "scripts/" will be prefixed before the
    //! image is searched for using StelFileMgr.
    //! @param ra0 The right ascension of the first corner of the image in degrees
    //! @param dec0 The declenation of the first corner of the image in degrees
    //! @param ra1 The right ascension of the second corner of the image in degrees
    //! @param dec1 The declenation of the second corner of the image in degrees
    //! @param ra2 The right ascension of the third corner of the image in degrees
    //! @param dec2 The declenation of the third corner of the image in degrees
    //! @param ra3 The right ascension of the fourth corner of the image in degrees
    //! @param dec3 The declenation of the fourth corner of the image in degrees
    //! @param minRes The minimum resolution setting for the image
    //! @param maxBright The maximum brightness setting for the image
    //! @param visible The initial visibility of the image
    void loadPresentImage(const QString& id, const QString& filename,
                          double ra0, double dec0,
                          double ra1, double dec1,
                          double ra2, double dec2,
                          double ra3, double dec3,
                          double minRes, double maxBright,
                          bool   visible, bool screenORdome);


    //! Remove a SkyImage.
    //! @param id the ID of the image to remove.
    void removePresentImage(const QString& id);

    void setPVideoContrast(int presentID, double value);
    void setPVideoBrightness(int presentID,double value);
    void setPVideoSaturation(int presentID,double value);

    void setFlipHorz(bool b);
    void setFlipVert(bool b);
    void decreaseTimeSpeed();
    void increaseTimeSpeed();
    void decreaseTimeSpeedLess();
    void increaseTimeSpeedLess();

    //About FlyBy
    void prepareFlyBy();
    void setFlyBy(const QString &planetFlyByname, bool isInner);
    void goHome();

signals:
    void requestLoadSkyImage(const QString& id, const QString& filename,
                             double c1, double c2,
                             double c3, double c4,
                             double c5, double c6,
                             double c7, double c8,
                             double minRes, double maxBright,
                             bool visible);

    void requestRemoveSkyImage(const QString& id);

    void requestLoadSound(const QString& filename, const QString& id);
    void requestPlaySound(const QString& id);
    void requestPauseSound(const QString& id);
    void requestStopSound(const QString& id);
    void requestDropSound(const QString& id);
    void requestSetNightMode(bool b);
    void requestSetProjectionMode(QString id);
    void requestSetSkyCulture(QString id);
    void requestSetDiskViewport(bool b);
    void requestExit();

    //About media player
    void requestLoadVideo(const QString& filename);
    void requestPlayVideo(bool isAudio);
    void requestPauseVideo(bool pause);
    void requestStopVideo();
    void requestPlayAudio(const QString& filename);
    void requestStopAudio();
    //About FlyBy
    void requestPrepareFlyBy();
    void requestSetFlyBy(const QString &planetFlyByname, bool isInner);
    void requestGoHome();

    void requestLoadPresentImage(const QString& id, const QString& filename,
                                 double c1, double c2,
                                 double c3, double c4,
                                 double c5, double c6,
                                 double c7, double c8,
                                 double minRes, double maxBright,
                                 bool visible,bool screenORdome);


    void requestLoadNewPresentImage(const QString& id,const QString& filename,
                                    double ra, double dec,
                                    double angSize, double rotation,
                                    double minRes, double maxBright,
                                    bool visible, bool screenORdome,
                                    double aspectratio);

    void requestLoadNewPresentVideo(const QString& id,const QString& filename,
                                    double ra, double dec,
                                    double angSize, double rotation,
                                    double minRes, double maxBright,
                                    bool visible, bool screenORdome,
                                    double aspectratio);

    void requestShowPresentImage(const QString& id,bool visible);

    void requestMixPresentImage(const QString& id,bool value);

    void requestSetPresentProperties(const QString& id, const QString& command);

    void requestSetLayerdomeORsky(const QString& id,bool bscreen);

    void requestRemovePresentImage(const QString& id);

    void requestPVideoContrast(int presentID,double value);
    void requestPVideoBrightness(int presentID,double value);
    void requestPVideoSaturation(int presentID,double value);

    void requestPlayPresentVideo(const QString& id,bool status);
    void requestPauseTogglePresentVideo(const QString& id);
    void requestRemovePresentVideo(const QString& id);
private:
    //! For use in setDate and waitFor
    //! For parameter descriptions see setDate().
    //! @returns Julian day.
    double jdFromDateString(const QString& dt, const QString& spec);
    ScriptSleeper scriptSleeper;
    QStringList recordData;

};

#endif // _STELMAINSCRIPTAPI_HPP_

