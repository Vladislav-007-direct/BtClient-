#ifndef FREEIMUCAL_H
#define FREEIMUCAL_H

#include <QMainWindow>

namespace Ui {
class FreeIMUCal;
}

class FreeIMUCal : public QMainWindow {
    Q_OBJECT

public:
    FreeIMUCal(QWidget* parent = nullptr);
    ~FreeIMUCal();

private:
    Ui::FreeIMUCal* ui;
};
#endif // FREEIMUCAL_H
