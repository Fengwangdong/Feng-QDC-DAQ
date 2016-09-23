#include "histogram.h"


histogram::histogram(QCustomPlot *qcp_, const QString &title_, const QString& xLabel_, const QString &yLabel_):
    qcp(qcp_)
{
    h = new QCPBars(qcp->xAxis, qcp->yAxis);
    qcp->addPlottable(h);


    init(title_, xLabel_, yLabel_);
}

void histogram::init(const QString &title_, const QString& xLabel_, const QString &yLabel_) {

    max = 1;
    xMin = 0;
    xMax = 100;
    nBins = 100;
    title = title_;
    xLabel = xLabel_;
    yLabel = yLabel_;

    initXY();
    h->setWidth((xMax - xMin)/x.size());
    h->setData(x, y);
    qcp->rescaleAxes();
    qcp->xAxis->setLabel(xLabel);
    qcp->yAxis->setLabel(yLabel);
    qcp->xAxis->setRange(xMin*0.99, xMax*1.01);
    qcp->yAxis->setRange(0, 10);
    qcp->axisRect()->setupFullAxesBox();

    // title
    qcp->plotLayout()->insertRow(0);
    qcp->plotLayout()->addElement(0, 0, new QCPPlotTitle(qcp, title));


    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(180, 180, 180));
    plotGradient.setColorAt(1, QColor(150, 150, 150));
    qcp->setBackground(plotGradient);
}

void histogram::initXY() {
    for (int i = 0; i < nBins; ++i)
    {
      x.push_back(xMin + i * xMax / nBins + 0.5 * (xMax - xMin) / nBins );
      y.push_back(0);
    }
}

void histogram::clearXY() {

    for (int i = 0; i < nBins; ++i) {
        x[i] = xMin + i * (xMax - xMin) / nBins + 0.5 * (xMax - xMin) / nBins;
        y[i] = 0;
    }

}

void histogram::resizeAndClearXY() {

    x.clear();
    y.clear();
    initXY();
}

void histogram::adjustPlot(int nBins_, double xMin_, double xMax_) {

    xMin = xMin_;
    xMax = xMax_;

    // if number of bins has changed, need to resize x and y vectors
    // and update nBins
    if (nBins_ != nBins) {
        nBins = nBins_;
        resizeAndClearXY();
    }
    // else just adjust the bin limits
    else {
        clearXY();
    }

    max = 1;
    h->clearData();
    h->setWidth((xMax - xMin)/x.size());
    h->setData(x, y);

    qcp->rescaleAxes();
    qcp->xAxis->setRange(xMin*0.99, xMax*1.01);
    qcp->yAxis->setRange(0, 10);
    qcp->replot();

}

void histogram::updatePlot(double value) {

    int index = (value - xMin) * nBins / (xMax - xMin);
    if (index >= nBins || index < 0) return;

    y[index]++;
    if (y[index] > max) max = y[index];
    h->clearData();
    h->setData(x, y);
    qcp->yAxis->setRange(0, 10 + 1.3*max);
    qcp->replot();
}

bool histogram::setLogY() {

    QCPAxis::ScaleType st = qcp->yAxis->scaleType();
    bool isLog = false;

    if (st == QCPAxis::stLogarithmic) {
        qcp->yAxis->setScaleType(QCPAxis::stLinear);
    }
    else if (st == QCPAxis::stLinear) {
        qcp->yAxis->setScaleType(QCPAxis::stLogarithmic);
        isLog = true;
    }

    qcp->replot();

    return isLog;
}

bool histogram::setLogX() {

    QCPAxis::ScaleType st = qcp->xAxis->scaleType();
    bool isLog = false;

    if (st == QCPAxis::stLogarithmic) {
        qcp->xAxis->setScaleType(QCPAxis::stLinear);
    }
    else if (st == QCPAxis::stLinear) {
        qcp->xAxis->setScaleType(QCPAxis::stLogarithmic);
        isLog = true;
    }

    qcp->replot();

    return isLog;
}
