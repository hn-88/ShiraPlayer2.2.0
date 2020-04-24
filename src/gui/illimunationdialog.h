#ifndef ILLIMUNATIONDIALOG_H
#define ILLIMUNATIONDIALOG_H

#include <QPushButton>
#include "StelDialog.hpp"


class Ui_illimunationDialog;

class illimunationDialog : public StelDialog
{
    Q_OBJECT

public:
    illimunationDialog();
    ~illimunationDialog();

    void languageChanged();

private:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();
    virtual void prepareAsChild();
    Ui_illimunationDialog *ui;
public slots:
    void retranslate();
private slots:
    void on_btnColors_click();
    void setButtonChecks(QPushButton* sender);

};

#endif // ILLIMUNATIONDIALOG_H
