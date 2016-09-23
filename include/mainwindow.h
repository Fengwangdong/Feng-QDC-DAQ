#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>

// e.g.: #include "hv.h"
#include "qdc.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closing();
    void closeEvent (QCloseEvent *event);
    void connectToVMECrate();


private slots:

private:
    Ui::MainWindow *ui;

    // handle to VME Crate
    int32_t handleChef;

    // tab widgets
    // e.g.: hv* hvWidget;
    QDC* qdcWidget;
};

#endif // MAINWINDOW_H
