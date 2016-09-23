#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <CAENVMElib.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // configure global view
    this->setWindowTitle("Base VME DAQ");
    this->setCentralWidget(ui->tabWidget);
    this->setFixedSize(this->size());


    connectToVMECrate();


	 // build Widgets
    // e.g.: hvWidget = new hv(this, handleChef);
    qdcWidget = new QDC(this, handleChef);

    // add the tabs for the program
    // e.g.: ui->tabWidget->addTab(hvWidget, "HV");
    ui->tabWidget->addTab(qdcWidget, "QDC");

    // set the starting tab index
    // e.g.: ui->tabWidget->setCurrentIndex(0); // HV
    ui->tabWidget->setCurrentIndex(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closing()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question(
        this,
        this->windowTitle(),
        tr("Are you sure you want to exit ?\n"),
        QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
        QMessageBox::Yes
    );

    if (resBtn == QMessageBox::Yes) {
        event->accept();
        closing();
    }
    else {
        event->ignore();
    }
}

void MainWindow::connectToVMECrate()
{
    while (CAENVME_Init(cvV1718, 0, 0, &handleChef) != cvSuccess) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(
            this,
            "Error",
            tr("Impossible to connect to VME crate!\nPlease, check that the usb cable is connected. If it is connected, check if it is seen in /etc/usb/\nTry again ?"),
            QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
            QMessageBox::Yes
        );

        if (resBtn != QMessageBox::Yes) {
            throw std::runtime_error("Failed to connect to VME crate");
        }
    }
}
