#ifndef PREVIEWDIALOG_HPP
#define PREVIEWDIALOG_HPP

#include "StelDialog.hpp"
#include "StelMainGraphicsView.hpp"
#include "prevappgraphicwidget.hpp"
#include "PrevglWidget.hpp"
#include "StelAppGraphicsWidget.hpp"

#include <QGraphicsWidget>


class StelGui;
class Ui_PreviewDialog;

class PreviewDialog : public StelDialog
{
    Q_OBJECT
public:
    PreviewDialog(StelGui* agui);
    virtual~PreviewDialog();

    void languageChanged();
    //! Notify that the application style changed
    void styleChanged();

    Ui_PreviewDialog *ui;


private:
    //! Initialize the dialog widgets and connect the signals/slots
    virtual void createDialogContent();



    //StelQGLWidget*
    QGLWidget* glWidget;
    PrevAppGraphicWidget* prevSkyItem;
    QGraphicsWidget* backItem;

    PrevGLWidget* prevGLWidget;
    //---

    //class StelMainGraphicsView* prevGraphicsView;

};

#endif // PREVIEWDIALOG_HPP
