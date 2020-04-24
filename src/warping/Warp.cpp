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

#include <math.h>
#include "Warp.h"
#include<GL/glu.h>

Warp::Warp()
{
    m_bEnabled = true;
    m_pCtrlPoints = NULL;
    setNumCtrlPoints(8);
    m_gridSize = 24;
    m_zoom = 1.0f;
    m_selectedCtrlPoint = -1;
    m_bShowCtrlPoints = true;
}

Warp::~Warp()
{
}

void Warp::draw(bool bForExport, bool bForBlend)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glGetIntegerv(GL_VIEWPORT, m_viewport);

    glOrtho(-0.5f, 0.5f, -0.5f, 0.5f, -1.0f, 1.0f);
    glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    if (!bForExport)
        glScalef(m_zoom, m_zoom, 1.0f);
    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelMatrix);

    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MAP2_VERTEX_3);
    glEnable(GL_MAP2_TEXTURE_COORD_2);
    glMapGrid2f(m_gridSize, 0.0f, 1.0f, m_gridSize, 0.0f, 1.0f);
    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, m_numCtrlPoints, 0, 1, m_numCtrlPoints * 3, m_numCtrlPoints, m_pCtrlPoints);
    GLfloat pTexCoords[2][2][2] = {
        {{0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 1.0f}, {1.0f, 1.0f}}};
    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0.0f, 1.0f, 2, 2, 0.0f, 1.0f, 4, 2, &pTexCoords[0][0][0]);
    glEvalMesh2(GL_FILL, 0, m_gridSize, 0, m_gridSize);

    if (m_bShowCtrlPoints && !bForExport)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        glLineWidth(1.0f);
        for (int y=0; y<m_numCtrlPoints; ++y)
        {
            for (int x=0; x<m_numCtrlPoints; ++x)
            {
                float dx = 1.0f / (m_numCtrlPoints-1) * (float)x - 0.5f;
                float dy = 1.0f / (m_numCtrlPoints-1) * (float)y - 0.5f;
                if (m_selectedCtrlPoint == y*m_numCtrlPoints+x)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                {
                    if (bForBlend)
                        glColor3f(0.0f, 0.0f, 1.0f);
                    else
                        glColor3f(1.0f, 1.0f, 1.0f);
                }
                glBegin(GL_LINES);
                glVertex3f(dx, dy, 0.0f);
                glVertex3fv(&m_pCtrlPoints[(y*m_numCtrlPoints+x)*3]);
                glEnd();
            }
        }

        glPointSize(12.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_POINTS);
        for (int i=0; i<m_numCtrlPoints*m_numCtrlPoints; ++i) {
            if (i != m_selectedCtrlPoint)
                glVertex3fv(&m_pCtrlPoints[i*3]);
        }
        glEnd();
        if (m_selectedCtrlPoint >= 0)
        {
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_POINTS);
            glVertex3fv(&m_pCtrlPoints[m_selectedCtrlPoint*3]);
            glEnd();
        }
    }

    glPopMatrix();

    glPopAttrib();
}

bool Warp::pick(int x, int y)
{
    if (!m_bEnabled)
        return 0;

    m_lastMouseX = x;
    m_lastMouseY = y;

    int newCtrlPoint = pickCtrlPoint(x, y);
    if (newCtrlPoint != m_selectedCtrlPoint && newCtrlPoint >= 0)
    {
        m_selectedCtrlPoint = newCtrlPoint;
        return true;
    }
    return false;
}

bool Warp::drag(int x, int y)
{
    if (!m_bEnabled)
        return 0;

    if (m_selectedCtrlPoint < 0)
        return false;

    GLdouble objx, objy, objz;
    GLdouble objx2, objy2, objz2;

    gluUnProject(x - m_lastMouseX, -(y - m_lastMouseY), 0.95,
                 m_modelMatrix, m_projMatrix, m_viewport,
                 &objx, &objy, &objz);
    gluUnProject(0, 0, 0.95,
                 m_modelMatrix, m_projMatrix, m_viewport,
                 &objx2, &objy2, &objz2);
    m_pCtrlPoints[m_selectedCtrlPoint * 3 + 0] += objx-objx2;
    m_pCtrlPoints[m_selectedCtrlPoint * 3 + 1] += objy-objy2;

    m_lastMouseX = x;
    m_lastMouseY = y;

    return true;
}

void Warp::setEnabled(bool bEnabled)
{
    m_bEnabled = bEnabled;
}

bool Warp::getEnabled() const
{
    return m_bEnabled;
}

void Warp::setNumCtrlPoints(int num)
{
    if (num == 0)
        return;

    if (num > 8)
        num = 8;
    if (num <2)
        num = 2;

    if (m_pCtrlPoints)
        delete [] m_pCtrlPoints;
    m_numCtrlPoints = num;
    m_pCtrlPoints = new float[num*num*3];
    reset();
}

int Warp::getNumCtrlPoints() const
{
    return m_numCtrlPoints;
}

void Warp::setZoom(float zoom)
{
    m_zoom = zoom;
}

float Warp::getZoom() const
{
    return m_zoom;
}

void Warp::setGridSize(int gridSize)
{
    m_gridSize = gridSize;
}

int Warp::getGridSize() const
{
    return m_gridSize;
}

void Warp::setShowCtrlPoints(bool bShow)
{
    m_bShowCtrlPoints = bShow;
}

bool Warp::getShowCtrlPoints() const
{
    return m_bShowCtrlPoints;
}

void Warp::reset()
{
    for (int y=0; y<m_numCtrlPoints; ++y) {
        for (int x=0; x<m_numCtrlPoints; ++x) {
            m_pCtrlPoints[(y*m_numCtrlPoints+x)*3+0] = 1.0f / (m_numCtrlPoints-1) * (float)x - 0.5f;
            m_pCtrlPoints[(y*m_numCtrlPoints+x)*3+1] = 1.0f / (m_numCtrlPoints-1) * (float)y - 0.5f;
            m_pCtrlPoints[(y*m_numCtrlPoints+x)*3+2] = 0.0f;
        }
    }
}

int Warp::pickCtrlPoint(float x, float y)
{
    int hits;

    GLuint selectBuffer[64];
    glSelectBuffer(sizeof(selectBuffer), selectBuffer);

    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(~0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPickMatrix(x, m_viewport[3] - y, 8.0, 8.0, m_viewport);
    glMultMatrixd(m_projMatrix);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMultMatrixd(m_modelMatrix);
    for (int i=0; i<m_numCtrlPoints*m_numCtrlPoints; ++i) {
        glLoadName(i);
        glBegin(GL_POINTS);
        glVertex3fv(&m_pCtrlPoints[i*3]);
        glEnd();
    }
    glPopMatrix();
    hits = glRenderMode(GL_RENDER);

    if (hits)
        return selectBuffer[3];
    return ~0;
}

/**
 * Restore the channel data from XML data.
 *
 * @param element Parent XML element of the warp data.
 */
void Warp::initFromDOMElement(const QDomElement& element)
{
    if (!element.isNull())
    {
		m_bEnabled = (element.attribute("enabled")=="true");
		int numCtrlPoints = (int)sqrtf(element.attribute("numCtrlPoints").toInt());
		if (numCtrlPoints != m_numCtrlPoints)
			setNumCtrlPoints(numCtrlPoints);

		int count = 0;
		QDomElement ctrlPointElem = element.firstChildElement("ctrlPoint");
		while (!ctrlPointElem.isNull()) {
			if (count >= m_numCtrlPoints * m_numCtrlPoints)
				break;
			m_pCtrlPoints[count*3+0] = ctrlPointElem.attribute("x").toDouble();
			m_pCtrlPoints[count*3+1] = ctrlPointElem.attribute("y").toDouble();
			m_pCtrlPoints[count*3+2] = 0.0f;
			ctrlPointElem = ctrlPointElem.nextSiblingElement("ctrlPoint");
			count++;
		}
		
    }
}

/**
 * Store the current warp data as XML data.
 *
 * @param name XML node name of the data.
 * @param doc XML document to store the data.
 * @return Current warp data as XML data.
 */
QDomElement Warp::domElement(const QString& name, QDomDocument& doc) const
{
    QDomElement de = doc.createElement(name);
    de.setAttribute("enabled", m_bEnabled?"true":"false");
    de.setAttribute("numCtrlPoints", m_numCtrlPoints*m_numCtrlPoints);
    for (int i=0; i<m_numCtrlPoints*m_numCtrlPoints; ++i) {
        QDomElement ctrlPointElem = doc.createElement("ctrlPoint");
        ctrlPointElem.setAttribute("x", m_pCtrlPoints[i*3+0]);
        ctrlPointElem.setAttribute("y", m_pCtrlPoints[i*3+1]);
        de.appendChild(ctrlPointElem);
    }

    return de;
}



