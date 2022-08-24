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

private:
    Q3DScatter* graph;
    QWidget* container;
    QHBoxLayout* layout;
};

#endif // GLVIEWWIDGET_H
