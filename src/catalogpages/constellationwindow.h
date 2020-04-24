#ifndef CONSTELLATIONWINDOW_H
#define CONSTELLATIONWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QSettings>

#include "presenter/flowlayout.h"
#include "VecMath.hpp"

namespace Ui {
class ConstellationWindow;
}

class ConstellationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ConstellationWindow(QWidget *parent = 0);
    ~ConstellationWindow();
    void languageChanged();
private:
    Ui::ConstellationWindow *ui;
    FlowLayout *flowLayoutLeft;
    FlowLayout *flowLayoutMiddle;
    FlowLayout *flowLayoutRight;
    QStringList leftList;
    QStringList middleList;
    QStringList rightList;
    QSpinBox* spW;
    QDoubleSpinBox* spArt;
    //QComboBox *cbLineColor;
    QPushButton *btnColor;
    QPushButton *getButtonFromName(const QString& name);
    QSettings* conf;

    void uncheckSingleConsts();    
    void uncheckMultiConsts(QPushButton *btn);
    bool IsExistMultiCheck();
    QString colToConf(Vec3f c);
    void setBtnBackround(QPushButton* btn, QColor col);

private slots:
    void clickButton(bool val);
    void spWvalueChanged(int val);
    void btnColorClicked();
protected:
    void LoadButtonObjects();
    void LoadConstLists();

};

#endif // CONSTELLATIONWINDOW_H
