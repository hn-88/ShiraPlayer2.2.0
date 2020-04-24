/*
 * ShiraPlayer(TM)
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

#ifndef _WARP_H_
#define _WARP_H_

#include <QtOpenGL/QGLWidget>
#include <QtXml/QtXml>


class Warp : public QObject
{
public:

    Warp();

    virtual ~Warp();

    void draw(bool bForExport, bool bForBlend);
    void resize(int width, int height);

    bool pick(int x, int y);
    bool drag(int x, int y);

    void setEnabled(bool bEnabled);
    bool getEnabled() const;

    void setNumCtrlPoints(int num);
    int  getNumCtrlPoints() const;
    bool isShowWarp();

    void setZoom(float zoom);
    float getZoom() const;

    void setGridSize(int gridSize);
    int  getGridSize() const;

    void setShowCtrlPoints(bool bShow);
    bool getShowCtrlPoints() const;

    void reset();

    /**
     * Restore the warp data from XML data.
     *
     * @param element Parent XML element of the warp data.
     */
    void initFromDOMElement(const QDomElement& element);

    /**
     * Store the current warp data as XML data.
     *
     * @param name XML node name of the data.
     * @param doc XML document to store the data.
     * @return Current warp data as XML data.
     */
    QDomElement domElement(const QString& name, QDomDocument& doc) const;

private:

    int  pickCtrlPoint(float x, float y);

private:

    bool m_bEnabled;

    int m_numCtrlPoints;
    float* m_pCtrlPoints;
    int m_gridSize;
    bool m_bShowCtrlPoints;
    int m_selectedCtrlPoint;

    GLint m_viewport[4];
    GLdouble m_modelMatrix[16];
    GLdouble m_projMatrix[16];
    float m_zoom;
    int m_lastMouseX;
    int m_lastMouseY;

};

#endif // _WARP_H_
