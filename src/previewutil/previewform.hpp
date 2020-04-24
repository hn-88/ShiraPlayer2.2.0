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

#ifndef PREVIEWFORM_HPP
#define PREVIEWFORM_HPP

#include <QWidget>
#include <QEvent>
#include <QPushButton>

class Ui_PreviewForm;

class PreviewForm : public QWidget {
    Q_OBJECT
public:
    PreviewForm(QWidget *parent = 0);
    ~PreviewForm();
    //Ui::PreviewForm *ui;
    Ui_PreviewForm* ui;
    void languageChanged();
    void setInfoText(QString text);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void setInformationBoxHeight(int value);

    QTimer* stellaHideTimer;
    QTimer* resetAllTimer;

protected:
    void changeEvent(QEvent *e);
    void focusInEvent(QFocusEvent *);
    void showEvent ( QShowEvent * event );

private:
    void setButtonChecks(QPushButton* sender);
    QPoint mpos;
private slots:
    void on_btnQuit_clicked();
    void on_btnFadeEffect_toggled(bool toggled);
    void on_btnNightView_clicked(bool checked);
    void on_btnRealTime_clicked(bool checked);
    void on_btnApply_clicked();
    void on_btnLock_clicked(bool checked); 
    void on_btnStellaHide_toggled(bool toggled);
    void on_btnResetAll_clicked();

    void on_simpleMovies_clicked();
    void on_simpleExit_clicked();
    void on_simpleStellarium_clicked();
    void on_simpleFisheyeImage_clicked();
    void on_simpleFlatMedia_clicked();
    void on_simpleScripts_clicked();
    void on_simpleFreeHand_clicked();
    void on_simpleFlyby_clicked();
    void on_simpleMessier_clicked();
    void on_simpleConst_clicked();
    void on_simpleAudio_clicked();

    void on_btnInform_clicked();
    void on_btnControls_clicked();

    void on_widgetInform_visibilityChanged(bool visible);
    void on_widgetControls_visibilityChanged(bool visible);

    void on_stellaHideTimer();
    void resetAllProc();

    void on_testButton_clicked();
};

#endif // PREVIEWFORM_HPP
