/*
 * ShiraPlayer(TM)
 * Copyright (C) 2012 Asaf Yurdakul
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

#ifndef TUPCOLORPALETTE_H
#define TUPCOLORPALETTE_H

#include <QWidget>

class TupColorPalette : public QWidget
{
    Q_OBJECT

    public:
        enum Color { Red = 0, Green, Blue, White };

        TupColorPalette(const QBrush brush, const QSize size, QWidget *parent);
        ~TupColorPalette();

    signals:
        void updateColor(const QColor color);
        void selectionDone();

    private slots:
        void updateSelection(int index);
        void pickSelection(int index);
        void updateMatrixFromSlider(int value);
        void updateMatrix(int newColor, bool fromSlider = false);

    private:
        void setSliderPanel();
        void initColorsArray();
        void setBaseColorsPanel();

        struct Private;
        Private *const k;
};

#endif
