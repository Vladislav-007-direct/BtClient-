#include "plotwidget.h"

PlotWidget::PlotWidget(QWidget* parent)
    : QChartView(parent) {

    setRenderHint(QPainter::Antialiasing);

    chart = new QtCharts::QChart();
    series = new QtCharts::QLineSeries();

    chart->setAxisX(new QtCharts::QValueAxis(), series);
    chart->setAxisY(new QtCharts::QValueAxis(), series);

    chart->addSeries(series);
    setChart(chart);
}

PlotWidget::~PlotWidget() {
    delete series;
    delete chart;
}

void PlotWidget::setXRange(double min, double max) {
    chart->axisX()->setRange(min, max);
}

void PlotWidget::setYRange(double min, double max) {
    chart->axisY()->setRange(min, max);
}
