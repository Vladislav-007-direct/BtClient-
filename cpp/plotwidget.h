#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QChartView>
#include <QLineSeries>
#include <QObject>
#include <QValueAxis>

class PlotWidget : public QChartView {
public:
    PlotWidget(QWidget* parent);
    ~PlotWidget();
    void setXRange(double min, double max);
    void setYRange(double min, double max);
    //    void setAspectLocked();

private:
    QChart* chart;
    QLineSeries* series;
};

#endif // PLOTWIDGET_H
