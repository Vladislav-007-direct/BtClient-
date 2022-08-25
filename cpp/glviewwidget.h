#ifndef GLVIEWWIDGET_H
#define GLVIEWWIDGET_H

#include <Q3DScatter>
#include <QHBoxLayout>
#include <QObject>
#include <QWidget>

class GLViewWidget : public QWidget {
public:
    GLViewWidget(QWidget* parent);
    ~GLViewWidget();

    void setSize(long, long, long);

private:
    QtDataVisualization::Q3DScatter* graph;
    QWidget* container;
    QHBoxLayout* layout;
};

#endif // GLVIEWWIDGET_H
