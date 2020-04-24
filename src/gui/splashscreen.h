#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QDateTime>
#include "StelFader.hpp"

namespace Ui {
class splashScreen;
}

class splashScreen : public QWidget
{
    Q_OBJECT

public:
    explicit splashScreen(QWidget *parent = 0);
    ~splashScreen();
private:
    Ui::splashScreen *ui;
    LinearFader fader;
    int timerId;

protected:
    void paintEvent(QPaintEvent *event);
    void timerEvent(QTimerEvent *event);

};

#endif // SPLASHSCREEN_H
