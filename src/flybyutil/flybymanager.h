#ifndef FLYBYMANAGER_H
#define FLYBYMANAGER_H

#include <QWidget>
#include "StelLocation.hpp"
#include "SolarSystem.hpp"

namespace Ui {
class FlybyManager;
}

class FlybyManager : public QWidget
{
    Q_OBJECT

public:
    explicit FlybyManager(QWidget *parent = 0);
    ~FlybyManager();

    void languageChanged();

private slots:
    void on_altitudeSlider_sliderReleased();

    void on_btnStart_clicked();

    void on_btnEarth_clicked();

    void on_velocitySlider_sliderReleased();

    void on_comboBox_currentIndexChanged(int index);

    void on_btnOrbit_toggled(bool checked);

    void on_chFullLight_toggled(bool checked);

   // void on_btnAxis_toggled(bool checked);

   // void on_btnEquline_toggled(bool checked);

protected slots:
    void on_flyby_tmeout();
private:
    Ui::FlybyManager *ui;
    int realtrackValue ;
    QTimer* timerFlyby;
    PlanetP selectedPlanet;

    void calcVelocity();
    void setButtons(QToolButton *sender);
};

#endif // FLYBYMANAGER_H
