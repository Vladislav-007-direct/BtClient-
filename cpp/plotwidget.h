#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QChartView>
#include <QObject>

class PlotWidget : public QChartView {
public:
    PlotWidget(QWidget* parent);
};

#endif // PLOTWIDGET_H
