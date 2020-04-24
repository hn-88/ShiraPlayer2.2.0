#ifndef MESSIERIMAGE_H
#define MESSIERIMAGE_H

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QTimer>

class messierimage : public QObject
{
    Q_OBJECT
private:
    QTimer* timAnimate;
public:
    messierimage();

    QString id;
    QString filepath;
    double ra;
    double dec;
    double size;
    double rotate;
    double p1;
    double p2;
    double width;
    double height;
    double aspectratio;

    int animMode ;
    QString btnName;
    void startTimer();
private slots:
    void on_animate_timeout();

};
typedef QSharedPointer<messierimage> messierimagePtr;

#endif // MESSIERIMAGE_H
