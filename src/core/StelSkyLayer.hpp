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

#ifndef STELSKYLAYER_HPP
#define STELSKYLAYER_HPP

#include <QObject>
#include <QString>
#include <QSharedPointer>

#include <QTimeLine>

class StelCore;
class StelPainter;

//! Abstract class defining the API to implement for all sky layer.
//! A sky layer is a graphical layer containing image or polygons displayed in the sky.
//! The StelSkyImageMgr allows to set the display order for layers, as well as opacity.
class StelSkyLayer : public QObject
{
    Q_OBJECT
public:
    StelSkyLayer(QObject* parent=NULL) : QObject(parent) { texFader = new QTimeLine();
                                                           willchangeProject = false;
                                           //texFader->setCurveShape(QTimeLine::LinearCurve);
                                                         }

    //! Draws the content of the layer.
    virtual void draw(StelCore* core, StelPainter& sPainter, float opacity=1.)=0;

    //! Return the short name to display in the loading bar.
    virtual QString getShortName() const =0;

    //! Return the short server name to display in the loading bar.
    virtual QString getShortServerCredits() const {return QString();}

    //! Return a hint on which key to use for referencing this layer.
    //! Note that the key effectively used may be different.
    virtual QString getKeyHint() const {return getShortName();}

    //! Return a human readable description of the layer with e.g.
    //! links and copyrights.
    virtual QString getLayerDescriptionHtml() const {return "No description.";}

    // Used for smooth fade in
    QTimeLine* texFader;

    void setFlagShow(bool b);
    bool willchangeProject;

private slots:
    void textFaderFinish();

signals:
    //! Emitted when loading of data started or stopped.
    //! @param b true if data loading started, false if finished.
    void loadingStateChanged(bool b);

    //! Emitted when the percentage of loading tiles/tiles to be displayed changed.
    //! @param percentage the percentage of loaded data.
    void percentLoadedChanged(int percentage);

    void texFaderFinished();
};

//! @file StelSkyLayerMgr.hpp
//! Define the classes needed for managing layers of sky elements display.

//! @typedef StelSkyLayerP
//! Shared pointer on a StelSkyLayer instance (implement reference counting)
typedef QSharedPointer<StelSkyLayer> StelSkyLayerP;

#endif // STELSKYLAYER_HPP
