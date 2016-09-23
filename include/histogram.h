#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QString>
#include "qcustomplot.h"

class histogram
{
public:
    histogram(QCustomPlot* qcp_, const QString &title_ = "", const QString& xLabel_ = "", const QString &yLabel_ = "");

private:
    QCustomPlot* qcp;
    QCPBars* h;

    QVector<double> x, y;
    int nBins;
    double xMin, xMax, yMin, yMax, max;
    bool xLog, yLog;
    QString xLabel, yLabel, title;

    void init(const QString &title_, const QString& xLabel_, const QString &yLabel_);
    void initXY();
    void clearXY();
    void resizeAndClearXY();


public:
    void updatePlot(double value);
    void adjustPlot(int nBins_, double xMin_, double xMax_);
    double getXMin();
    double getXMax();
    bool setLogX();
    bool setLogY();


};


inline double histogram::getXMin() {
    return xMin;
}

inline double histogram::getXMax() {
    return xMax;
}
#endif // HISTOGRAM_H
