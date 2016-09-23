#include "qdc.h"
#include "ui_qdc.h"
#include <chrono>

QDC::QDC(QWidget *parent, int32_t handleChef_) :
    QWidget(parent),
    ui(new Ui::QDC)
{
    ui->setupUi(this);

    module = new QDCModule(handleChef_);
    ui->spinBox_qdc_channelN->setMaximum(module->getNChannels()-1);

    hQDC = new histogram(ui->qdc_profile, "Integral charge profile", "charge (pC)", "nEvents");
    hQDC->adjustPlot(ui->spinBox_qdc_nbins->value(), ui->doubleSpinBox_qdc_vmin->value(), ui->doubleSpinBox_qdc_vmax->value());

    nExp = ui->spinBox_qdc_nevents->value();

    ui->pushButton_stopprofile->setEnabled(false);
    for (int i = 0; i < module->getNChannels(); i++) module->ConfigureChannel(i, false, 0);

}

QDC::~QDC()
{
    delete ui;
}

void QDC::on_pushButton_clicked()
{
    int channelN = ui->spinBox_qdc_channelN->value();
    bool enable = ui->radioButton_qdc_enable->isChecked();
    int threshold = ui->spinBox_qdc_threshold->value();
    qDebug() << threshold;

    if(threshold < 0){
        QMessageBox::information(this,
                                 "Error!",
                                 "Invalid threshold!",
                                 QMessageBox::Ok);
        return;
    }

    else if(threshold < 16 ){
        QMessageBox::information(this,
                                 "Warning!",
                                 "The threshold value below 16 is treated equivalently as 0!",
                                 QMessageBox::Ok);
    }

    else if((threshold % 16) != 0){
        QMessageBox::information(this,
                                 "Warning!",
                                 "The threshold value is not the multiple of 16, and it is treated equivalently as the adjacent smaller value that is the multiple of 16!",
                                 QMessageBox::Ok);
    }

    if (channelN == -1) {
        for (int i = 0; i < ui->spinBox_qdc_channelN->maximum()+1; i++)
            module->ConfigureChannel(i, enable, threshold);
    }
    else {
        module->ConfigureChannel(channelN, enable, threshold);
    }

}

void QDC::on_spinBox_qdc_channelN_valueChanged(int channelN)
{
    if (channelN == -1) return;
    bool enable = module->ReadChannelStatus(channelN);
    int threshold = module->ReadChannelThreshold(channelN);
    qDebug() << enable;
    ui->radioButton_qdc_enable->setChecked(enable);
    ui->spinBox_qdc_threshold->setValue(threshold);
}


void QDC::on_pushButton_clearChannel_clicked()
{
    module->ClearChannels();
}

void QDC::on_spinBox_qdc_nbins_valueChanged(int nBins)
{
    hQDC->adjustPlot(nBins, ui->doubleSpinBox_qdc_vmin->value(), ui->doubleSpinBox_qdc_vmax->value());
}

void QDC::on_doubleSpinBox_qdc_vmin_valueChanged(double vMin)
{
    double vMax = ui->doubleSpinBox_qdc_vmax->value();
    if(vMax <= vMin) return;
    hQDC->adjustPlot(ui->spinBox_qdc_nbins->value(), vMin, vMax);
}

void QDC::on_doubleSpinBox_qdc_vmax_valueChanged(double vMax)
{
    double vMin = ui->doubleSpinBox_qdc_vmin->value();
    if(vMax <= vMin) return;
    hQDC->adjustPlot(ui->spinBox_qdc_nbins->value(), vMin, vMax);
}

void QDC::on_pushButton_qdc_startprofile_clicked()
{
    nExp = ui->spinBox_qdc_nevents->value();
    ui->spinBox_qdc_nevents->setEnabled(false);
    ui->pushButton_clearChannel->setEnabled(false);
    ui->pushButton_clearProfile->setEnabled(false);
    ui->pushButton_qdc_startprofile->setEnabled(false);
    ui->pushButton_stopprofile->setEnabled(true);

    ui->spinBox_qdc_nbins->setEnabled(false);
    ui->doubleSpinBox_qdc_vmin->setEnabled(false);
    ui->doubleSpinBox_qdc_vmax->setEnabled(false);

    std::string textFileName = "IntegralChargeProfile_" + std::to_string(nExp) + "_events.txt";
    text = new std::ofstream(textFileName.c_str(), std::ios_base::out);

    for(int i=0; i<nExp; i++){ // the stop will be implemented in the next function
        bool validData = false;
        uint32_t value32 = 0;

        do {
            module->ReadChannelData(0, &value32, &validData);
            QCoreApplication::processEvents();
        } while(!validData);

        hQDC->updatePlot(value32/10.0);
        (*text) << (value32/10.0) << std::endl;
    }

    text->close();
    ui->spinBox_qdc_nevents->setEnabled(true);
    ui->pushButton_clearChannel->setEnabled(true);
    ui->pushButton_clearProfile->setEnabled(true);
    ui->pushButton_qdc_startprofile->setEnabled(true);
    ui->pushButton_stopprofile->setEnabled(false);

    ui->spinBox_qdc_nbins->setEnabled(true);
    ui->doubleSpinBox_qdc_vmin->setEnabled(true);
    ui->doubleSpinBox_qdc_vmax->setEnabled(true);

}

void QDC::on_pushButton_stopprofile_clicked()
{
    nExp = 0;
    text->close();
    ui->spinBox_qdc_nevents->setEnabled(true);
    ui->pushButton_clearChannel->setEnabled(true);
    ui->pushButton_clearProfile->setEnabled(true);
    ui->pushButton_qdc_startprofile->setEnabled(true);
    ui->pushButton_stopprofile->setEnabled(false);

    ui->spinBox_qdc_nbins->setEnabled(true);
    ui->doubleSpinBox_qdc_vmin->setEnabled(true);
    ui->doubleSpinBox_qdc_vmax->setEnabled(true);
}

void QDC::on_pushButton_clearProfile_clicked()
{
    QMessageBox::StandardButton clrbtn = QMessageBox::question(
                this,
                "Clear Graph",
                tr("Warning: Are you sure that you would like to clear the graph? \n Otherwise you will lose the recorded profile!"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
    );

    if(clrbtn == QMessageBox::Yes){
        hQDC->adjustPlot(ui->spinBox_qdc_nbins->value(), ui->doubleSpinBox_qdc_vmin->value(), ui->doubleSpinBox_qdc_vmax->value());
    }
}
