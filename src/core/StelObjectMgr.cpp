/*
 * ShiraPlayer(TM)
 * Copyright (C) 2007 Fabien Chereau
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
#include "StelObjectMgr.hpp"
#include "StelObjectModule.hpp"
#include "StelModuleMgr.hpp"
#include "StelCore.hpp"
#include "StelUtils.hpp"
#include "StelProjector.hpp"
#include "StelMovementMgr.hpp"
#include <QMouseEvent>
#include <QString>
#include <QDebug>
#include <QStringList>

StelObjectMgr::StelObjectMgr() : searchRadiusPixel(30.f)
{
    setObjectName("StelObjectMgr");
    objectPointerVisibility = true;
}

StelObjectMgr::~StelObjectMgr()
{
}

/*************************************************************************
 Handle mouse click events.
*************************************************************************/
void StelObjectMgr::handleMouseClicks(QMouseEvent* event)
{
    // Deselect the selected object
    if (event->button()==Qt::RightButton && event->type()==QEvent::MouseButtonRelease)
    {
        unSelect();
        StelApp::getInstance().addNetworkCommand("core.selectObjectByName('',true);");
        event->accept();
        return;
    }
    StelMovementMgr* mvmgr = GETSTELMODULE(StelMovementMgr);
    if (event->button()==Qt::LeftButton && event->type()==QEvent::MouseButtonRelease &&
            !mvmgr->getHasDragged() && (!StelApp::getInstance().getAllowFreeHand()))
    {
#ifdef Q_OS_MAC
        // CTRL + left clic = right clic for 1 button mouse
        if (event->modifiers().testFlag(Qt::ControlModifier))
        {
            unSelect();
            event->accept();
            return;
        }

        // Try to select object at that position
        findAndSelect(StelApp::getInstance().getCore(), event->x(), event->y(),
                      event->modifiers().testFlag(Qt::MetaModifier) ? StelModule::AddToSelection : StelModule::ReplaceSelection);
#else
        findAndSelect(StelApp::getInstance().getCore(),
                      event->x(),
                      event->y(),
                      event->modifiers().testFlag(Qt::ControlModifier) ? StelModule::AddToSelection : StelModule::ReplaceSelection);
#endif
        // If an object was selected update informations
        if (getWasSelected())
        {
            mvmgr->setFlagTracking(false);
        }

        if (getSelectedObject().count()>0)
        {

            QString s = getSelectedObject()[0]->getEnglishName();
            static const StelObject::InfoStringGroup infoTextFilters= (StelObject::InfoStringGroup)(StelObject::CatalogNumber|StelObject::PlainText);
            if (s=="")
            {
                s = getSelectedObject()[0]->getInfoString(StelApp::getInstance().getCore(),infoTextFilters);
                s = s.replace("\n","");
            }

            StelApp::getInstance().addNetworkCommand("core.selectObjectByName('"+s+"',true);");
        }
    }
    return;
}

/*************************************************************************
 Add a new StelObject manager into the list of supported modules.
*************************************************************************/
void StelObjectMgr::registerStelObjectMgr(StelObjectModule* mgr)
{
    objectsModule.push_back(mgr);
}


StelObjectP StelObjectMgr::searchByNameI18n(const QString &name) const
{
    StelObjectP rval;
    QList<StelObjectModule*>::const_iterator iter;
    for (iter=objectsModule.begin();iter!=objectsModule.end();++iter)
    {
        rval = (*iter)->searchByNameI18n(name);
        if (rval)
            return rval;
    }
    return rval;
}

//! Find any kind of object by its standard program name
StelObjectP StelObjectMgr::searchByName(const QString &name) const
{
    StelObjectP rval;
    QList<StelObjectModule*>::const_iterator iter;
    for (iter=objectsModule.begin();iter!=objectsModule.end();++iter)
    {
        rval = (*iter)->searchByName(name);
        if (rval)
            return rval;
    }
    return rval;
}


//! Find and select an object from its translated name
//! @param nameI18n the case sensitive object translated name
//! @return true if an object was found with the passed name
bool StelObjectMgr::findAndSelectI18n(const QString &nameI18n, StelModule::StelModuleSelectAction action)
{
    // Then look for another object
    StelObjectP obj = searchByNameI18n(nameI18n);
    if (!obj)
        return false;
    else
        return setSelectedObject(obj, action);
}

//! Find and select an object from its standard program name
bool StelObjectMgr::findAndSelect(const QString &name, StelModule::StelModuleSelectAction action)
{
    // Then look for another object
    StelObjectP obj = searchByName(name);
    if (!obj)
        return false;
    else
        return setSelectedObject(obj, action);
}


//! Find and select an object near given equatorial position
bool StelObjectMgr::findAndSelect(const StelCore* core, const Vec3d& pos, StelModule::StelModuleSelectAction action)
{
    StelObjectP tempselect = cleverFind(core, pos);
    return setSelectedObject(tempselect, action);
}

//! Find and select an object near given screen position
bool StelObjectMgr::findAndSelect(const StelCore* core, int x, int y, StelModule::StelModuleSelectAction action)
{
    StelObjectP tempselect = cleverFind(core, x, y);
    return setSelectedObject(tempselect, action);
}

// Find an object in a "clever" way, v in J2000 frame
StelObjectP StelObjectMgr::cleverFind(const StelCore* core, const Vec3d& v) const
{
    StelObjectP sobj;
    QList<StelObjectP> candidates;

    const StelProjectorP prj = core->getProjection(StelCore::FrameJ2000);

    // Field of view for a searchRadiusPixel pixel diameter circle on screen
    float fov_around = core->getMovementMgr()->getCurrentFov()/qMin(prj->getViewportWidth(), prj->getViewportHeight()) * searchRadiusPixel;

    // Collect the objects inside the range
    for (QList<StelObjectModule*>::const_iterator iteromgr=objectsModule.begin();iteromgr!=objectsModule.end();++iteromgr)
    {
        candidates += (*iteromgr)->searchAround(v, fov_around, core);
    }

    // Now select the object minimizing the function y = distance(in pixel) + magnitude
    Vec3d winpos;
    prj->project(v, winpos);
    float xpos = winpos[0];
    float ypos = winpos[1];

    float best_object_value;
    best_object_value = 100000.f;
    QList<StelObjectP>::iterator iter = candidates.begin();
    while (iter != candidates.end())
    {
        prj->project((*iter)->getJ2000EquatorialPos(core->getNavigator()), winpos);
        float distance = sqrt((xpos-winpos[0])*(xpos-winpos[0]) + (ypos-winpos[1])*(ypos-winpos[1]));
        float priority =  (*iter)->getSelectPriority(core->getNavigator());
        // qDebug() << (*iter).getShortInfoString(core->getNavigator()) << ": " << priority << " " << distance;
        if (distance + priority < best_object_value)
        {
            best_object_value = distance + priority;
            sobj = *iter;
        }
        ++iter;
    }

    return sobj;
}

/*************************************************************************
 Find in a "clever" way an object from its equatorial position
*************************************************************************/
StelObjectP StelObjectMgr::cleverFind(const StelCore* core, int x, int y) const
{
    Vec3d v;
    core->getProjection(StelCore::FrameJ2000)->unProject(x,y,v);
    return cleverFind(core, v);
}

/*************************************************************************
 Notify that we want to unselect any object
*************************************************************************/
void StelObjectMgr::unSelect(void)
{
    lastSelectedObjects.clear();

    // Send the event to every StelModule
    foreach (StelModule* iter, StelApp::getInstance().getModuleMgr().getAllModules())
    {
        iter->selectedObjectChangeCallBack(StelModule::RemoveFromSelection);
    }
}

/*************************************************************************
 Notify that we want to select the given object
*************************************************************************/
bool StelObjectMgr::setSelectedObject(const StelObjectP obj, StelModule::StelModuleSelectAction action)
{
    if (!obj)
    {
        unSelect();
        return false;
    }

    // An object has been found
    QList<StelObjectP> objs;
    objs.push_back(obj);
    return setSelectedObject(objs, action);
}

/*************************************************************************
 Notify that we want to select the given objects
*************************************************************************/
bool StelObjectMgr::setSelectedObject(const QList<StelObjectP>& objs, StelModule::StelModuleSelectAction action)
{
    lastSelectedObjects=objs;

    // Send the event to every StelModule
    foreach (StelModule* iter, StelApp::getInstance().getModuleMgr().getAllModules())
    {
        iter->selectedObjectChangeCallBack(action);
    }
    return true;
}

/*************************************************************************
 Return the list objects of type "withType" which was recently selected by
  the user
*************************************************************************/
QList<StelObjectP> StelObjectMgr::getSelectedObject(const QString& type)
{
    QList<StelObjectP> result;
    for (QList<StelObjectP>::iterator iter=lastSelectedObjects.begin();iter!=lastSelectedObjects.end();++iter)
    {
        if ((*iter)->getType()==type)
            result.push_back(*iter);
    }
    return result;
}


/*************************************************************************
 Find and return the list of at most maxNbItem objects auto-completing
 passed object I18 name
*************************************************************************/
QStringList StelObjectMgr::listMatchingObjectsI18n(const QString& objPrefix, unsigned int maxNbItem) const
{
    QStringList result;

    // For all StelObjectmodules..
    QList<StelObjectModule*>::const_iterator iteromgr;
    for (iteromgr=objectsModule.begin();iteromgr!=objectsModule.end();++iteromgr)
    {
        // Get matching object for this module
        QStringList matchingObj = (*iteromgr)->listMatchingObjectsI18n(objPrefix, maxNbItem);
        result += matchingObj;
        maxNbItem-=matchingObj.size();
    }

    result.sort();
    return result;
}

