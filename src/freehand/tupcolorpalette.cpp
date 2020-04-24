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
#include "tupcolorpalette.h"
#include "tupcolorwidget.h"
#include "tupslider.h"

#include <cmath>
#include <QTabWidget>
#include <QFrame>
#include <QBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QDialogButtonBox>
#include <QMouseEvent>
#include <QDebug>

struct TupColorPalette::Private
{
    QBoxLayout *paletteGlobalLayout;
    QGridLayout *colorMatrixLayout;
    QGridLayout *centralLayout;

    QList<TupColorWidget *> colors;
    QList<TupColorWidget *> baseColors;

    int currentColorIndex;
    int currentBaseColor;
    int currentLeadColor;

    QBrush brush;
    QColor color;
    int rows;
    int columns;
    QWidget *sliderWidget;
    TupSlider *slider;
};

TupColorPalette::TupColorPalette(const QBrush brush, const QSize size, QWidget *parent) : QWidget(parent), k(new Private)
{
    k->brush = brush;
    k->currentLeadColor = 255;
    k->currentColorIndex = -1;
    int w = size.width();
    int h = size.height();

    k->columns = w/150;
    k->rows = h/90;

    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    k->colorMatrixLayout = new QGridLayout;

    QBoxLayout *box = new QVBoxLayout;
    box->addLayout(k->colorMatrixLayout);

    k->paletteGlobalLayout = new QVBoxLayout;
    k->centralLayout = new QGridLayout;
    k->centralLayout->setHorizontalSpacing(0);

    initColorsArray();
    setSliderPanel();

    k->centralLayout->addLayout(box, 0, 2);
    k->paletteGlobalLayout->addLayout(k->centralLayout);

    setBaseColorsPanel();
    layout->addLayout(k->paletteGlobalLayout);
}

TupColorPalette::~TupColorPalette()
{
}

void TupColorPalette::setSliderPanel()
{
    k->sliderWidget = new QWidget();    
    //k->sliderWidget->setStyleSheet("* { background: rgb(106, 107, 110)}");
    k->sliderWidget->setStyleSheet("* { background:rgba(149, 150, 152, 20%)}");
    //k->sliderWidget->setMaximumWidth(53);
    //k->sliderWidget->setFixedHeight(220);

    QBoxLayout *sliderLayout = new QVBoxLayout(k->sliderWidget);
    sliderLayout->setAlignment(Qt::AlignVCenter);
    //sliderLayout->setContentsMargins(0, 0, 0, 0);
    //sliderLayout->setSpacing(0);

    k->slider = new TupSlider(Qt::Vertical, TupSlider::Color, QColor(255, 0, 0), QColor(0, 0, 0));
    k->slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //k->slider->setFixedWidth(100);

    k->slider->setRange(0, 255);

    sliderLayout->addWidget(k->slider);
    connect(k->slider, SIGNAL(valueChanged(int)), this, SLOT(updateMatrixFromSlider(int)));

    k->centralLayout->addWidget(k->sliderWidget);
}

void TupColorPalette::setBaseColorsPanel()
{
    QSize cellSize(50, 30);

    k->currentBaseColor = 0;
    QColor redColor(255, 0, 0);
    QBrush redBrush(redColor, k->brush.style());
    TupColorWidget *red = new TupColorWidget(0, redBrush, cellSize, false);
    red->setState(true);
    connect(red, SIGNAL(clicked(int)), this, SLOT(updateMatrix(int)));
    k->baseColors << red;

    QColor greenColor(0, 255, 0);
    QBrush greenBrush(greenColor, k->brush.style());
    TupColorWidget *green = new TupColorWidget(1, greenBrush, cellSize, false);
    connect(green, SIGNAL(clicked(int)), this, SLOT(updateMatrix(int)));
    k->baseColors << green;

    QColor blueColor(0, 0, 255);
    QBrush blueBrush(blueColor, k->brush.style());
    TupColorWidget *blue = new TupColorWidget(2, blueBrush, cellSize, false);
    connect(blue, SIGNAL(clicked(int)), this, SLOT(updateMatrix(int)));
    k->baseColors << blue;

    QColor whiteColor(255, 255, 255);
    QBrush whiteBrush(whiteColor, k->brush.style());
    TupColorWidget *white = new TupColorWidget(3, whiteBrush, cellSize, false);
    connect(white, SIGNAL(clicked(int)), this, SLOT(updateMatrix(int)));
    k->baseColors << white;

    QBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->setAlignment(Qt::AlignHCenter);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    bottomLayout->addWidget(red);
    bottomLayout->addWidget(green);
    bottomLayout->addWidget(blue);
    bottomLayout->addWidget(white);

    k->paletteGlobalLayout->addLayout(bottomLayout);
}

void TupColorPalette::initColorsArray()
{
    int deltaX = 255/k->rows;
    int deltaY = 255/k->columns;
    int r = 255;
    int g = 0;
    int b = 0;
    int index = 0;

    for (int i=0; i < k->rows; i++) {
         for (int j=0; j < k->columns; j++) {
              g = (i*deltaY); 
              b = (j*deltaX);

              if (g > 255)
                  g = 255;
              if (b > 255)
                  b = 255;

              QColor cellColor(r, g, b);
              QBrush brush(cellColor, k->brush.style());
              QSize cellSize(50, 30);
              TupColorWidget *button = new TupColorWidget(index, brush, cellSize, true);
              connect(button, SIGNAL(clicked(int)), this, SLOT(updateSelection(int)));
              connect(button, SIGNAL(doubledClicked(int)), SLOT(pickSelection(int)));
              index++;
              k->colors << button;
              k->colorMatrixLayout->addWidget(button, i, j);
         }
    }
}

void TupColorPalette::updateSelection(int index)
{
    if (index != k->currentColorIndex) {
        if (k->currentColorIndex >= 0) {
            TupColorWidget *button = (TupColorWidget *) k->colors.at(k->currentColorIndex);
            button->setState(false);
        }
        TupColorWidget *selection = (TupColorWidget *) k->colors.at(index);
        k->color = selection->color();
        k->currentColorIndex = index;
        emit updateColor(k->color);
    }
}

void TupColorPalette::pickSelection(int index)
{
    if (index != k->currentColorIndex) {
        TupColorWidget *selection = (TupColorWidget *) k->colors.at(index);
        k->color = selection->color();
    }

    emit updateColor(k->color);
    emit selectionDone();
}

void TupColorPalette::updateMatrixFromSlider(int value)
{
    k->currentLeadColor = value;
    updateMatrix(k->currentBaseColor, true);
}

void TupColorPalette::updateMatrix(int newColor, bool fromSlider)
{
    if (!fromSlider) {
        if (k->currentColorIndex >= 0) {
            TupColorWidget *button = (TupColorWidget *) k->colors.at(k->currentColorIndex);
            button->setState(false);
        }

        TupColorWidget *current = (TupColorWidget *) k->baseColors.at(newColor);
        if ((newColor != k->currentBaseColor)) {
             k->slider->setColors(current->color(), Qt::black);
             k->baseColors.at(k->currentBaseColor)->setState(false);
        }

        k->color = current->color();
        if (k->color == Qt::white) {
            k->slider->hide();
            k->slider->setEnabled(false);
        } else {
           k->slider->show();
           if (!k->slider->isEnabled())
                k->slider->setEnabled(true);
        }
    }

    k->currentBaseColor = newColor;
    int deltaX = 255 / k->rows;
    int deltaY = 255 / k->columns;
    int delta = ceil(255.0 / (k->rows * k->columns));

    int r = 0;
    int g = 0;
    int b = 0;
    int index = 0;
    TupColorPalette::Color color = TupColorPalette::Color(newColor);

    for (int i=0; i < k->rows; i++) {
         for (int j=0; j < k->columns; j++) {
              switch (color) {
                      case TupColorPalette::Red :
                           r = k->currentLeadColor;
                           if (k->rows > k->columns) {
                               g = (i*deltaY);
                               b = (j*deltaX);
                           } else {
                               g = (j*deltaY);
                               b = (i*deltaX);
                           }
                      break;
                      case TupColorPalette::Green :
                           r = (i*deltaX);
                           g = k->currentLeadColor;
                           b = (j*deltaX);
                      break;
                      case TupColorPalette::Blue :
                           r = (i*deltaX);
                           g = (j*deltaY);
                           b = k->currentLeadColor;
                      break;
                      case TupColorPalette::White :
                           if (j == k->columns - 1 && i == k->rows - 1) {
                               r = 255;
                               g = 255;
                               b = 255;
                           } else {
                               r += delta;
                               g += delta;
                               b += delta;
                           }
                      break;
              }


              if (r > 255)
                  r = 255;
              if (g > 255)
                  g = 255;
              if (b > 255)
                  b = 255;

              QColor cellColor(r, g, b);
              QBrush brush(cellColor, k->brush.style());
              TupColorWidget *cell = k->colors.at(index);
              cell->setBrush(brush);
              index++;
        } 

        if (k->currentColorIndex >= 0 && fromSlider) {
            TupColorWidget *last = (TupColorWidget *) k->colors.at(k->currentColorIndex);
            k->color = last->color();
        }

        emit updateColor(k->color);
    }

   this->update();
   //k->sliderWidget->update();
}
