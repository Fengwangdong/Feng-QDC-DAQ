#ifndef QDC_H
#define QDC_H

#include <QWidget>
#include "qdcmodule.h"
#include "qdebug.h"
#include "histogram.h"
#include <fstream>
#include <string>

namespace Ui {
class QDC;
}

class QDC : public QWidget
{
    Q_OBJECT

public:
    explicit QDC(QWidget *parent = 0, int32_t handleChef_ = 0);
    ~QDC();

private slots:
    void on_pushButton_clicked();

    void on_spinBox_qdc_channelN_valueChanged(int channelN);

    void on_pushButton_clearChannel_clicked();

    void on_spinBox_qdc_nbins_valueChanged(int nBins);

    void on_doubleSpinBox_qdc_vmin_valueChanged(double vMin);

    void on_doubleSpinBox_qdc_vmax_valueChanged(double vMax);

    void on_pushButton_qdc_startprofile_clicked();

    void on_pushButton_stopprofile_clicked();

    void on_pushButton_clearProfile_clicked();

private:
    Ui::QDC *ui;
    QDCModule *module;
    histogram *hQDC;
    int nExp;
    std::ofstream *text;

};

#endif // QDC_H
