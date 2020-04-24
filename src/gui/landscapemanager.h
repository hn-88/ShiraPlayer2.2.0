#ifndef LANDSCAPEMANAGER_H
#define LANDSCAPEMANAGER_H

#include "StelDialog.hpp"
#include <QListWidgetItem>

class Ui_landscapeManager;

class landscapeManager : public StelDialog
{
    Q_OBJECT

public:
    landscapeManager();
    ~landscapeManager();

    void languageChanged();
private:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();

    Ui_landscapeManager *ui;
private slots:
    void landscapeChanged(QListWidgetItem* item);

public slots:
    void retranslate();

};

#endif // LANDSCAPEMANAGER_H
