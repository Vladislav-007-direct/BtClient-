#include "glviewwidget.h"

GLViewWidget::GLViewWidget(QWidget* parent)
    : QWidget(parent) {
    graph = new Q3DScatter();
    container = QWidget::createWindowContainer(graph);
    layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(container);
    this->setLayout(layout);
}

GLViewWidget::~GLViewWidget() {
    delete graph;
    delete container;
    delete layout;
}