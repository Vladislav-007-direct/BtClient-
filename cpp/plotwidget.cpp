#include "plotwidget.h"

PlotWidget::PlotWidget(QWidget* parent)
    : QChartView(parent) {

    setRenderHint(QPainter::Antialiasing);

    chart = new QChart();
    series = new QLineSeries();

    chart->setAxisX(new QValueAxis(), series);
    chart->setAxisY(new QValueAxis(), series);

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
