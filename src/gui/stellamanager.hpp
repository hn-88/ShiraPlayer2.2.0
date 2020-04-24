#ifndef STELLAMANAGER_HPP
#define STELLAMANAGER_HPP


#include "StelModule.hpp"
#include "StelObject.hpp"
#include "LocationDialog.hpp"
#include "HelpDialog.hpp"
#include "DateTimeDialog.hpp"
#include "SearchDialog.hpp"
#include "StelGuiBase.hpp"
#include "recordmanager.hpp"
#include "planetszoomdialog.h"
#include "landscapemanager.h"
#include "illimunationdialog.h"
#include "StarMgr.hpp"

#include <QWidget>
#include <QPushButton>
class Ui_StellaManagerForm;

class stellamanager : public QWidget
{
Q_OBJECT
public:
    stellamanager(QWidget *parent = 0);
    ~stellamanager();
    Ui_StellaManagerForm* ui;

    LocationDialog locationDialog;
    HelpDialog helpDialog;
    DateTimeDialog dateTimeDialog;
    SearchDialog searchDialog;
    recordmanager recordmanagerDialog;
    PlanetsZoomDialog planetszoomdialog;
    landscapeManager landscapemanager;
    illimunationDialog illimunationdialog;
    void languageChanged();
protected:
    void showEvent(QShowEvent * event);

private:
    QTimer* loadButtonStatusTimer;
    double trackTime;
    void setButtonChecks(QPushButton* sender);
signals:
private slots:


    void on_btnDateTime_clicked();
    void on_btnLocation_clicked();
    void on_btnRecord_clicked();
    void on_btnHelp_clicked();
    void on_btnSearch_clicked();
    void on_btnPlanetZoom_clicked();
    void on_btnLandscape_clicked();
    void on_btnIlluminate_clicked();
    void on_btnSatllites_toggled(bool);
    void on_btnGoto_toggled(bool);
    void on_btnNightView_toggled(bool checked);    
    void on_btnStarTrails_toggled(bool checked);
    void on_horizontalSlider_moved(int value);
    void on_btnStarMag_toggled(bool checked);
    void on_btnNorth_clicked();
    void on_btnSouth_clicked();
    void on_btnWest_clicked();
    void on_btnEast_clicked();

public slots:
    void retranslate();
    void loadButtonStatus();

};

#endif // STELLAMANAGER_HPP
