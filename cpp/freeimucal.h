#ifndef FREEIMUCAL_H
#define FREEIMUCAL_H

#include <QMainWindow>
#include <QSettings>
#include <QThread>
#include <QVector2D>

#define acc_file_name "acc.txt"
#define magn_file_name "magn.txt"
#define calibration_h_file_name "calibration.h"
#define word 2
#define acc_range 25000
#define magn_range 1500

namespace Ui {
class FreeIMUCal;
}

class FreeIMUCal : public QMainWindow {
    Q_OBJECT

public:
    FreeIMUCal(QWidget* parent = nullptr);
    ~FreeIMUCal();

    void set_status(QString);
    void serial_connect();
    void serial_disconnect();
    void sampling_start();
    void sampling_end();
    void calibrate();
    void save_calibration_eeprom();
    void clear_calibration_eeprom();
    void newData(QVector<int>);

private:
    Ui::FreeIMUCal* ui;
    QSettings* settings;
    QVector<QVector<long>> acc_data;
    QVector<QVector<long>> magn_data;
};

class SerialWorker : public QThread {
public:
    SerialWorker();
};

#endif // FREEIMUCAL_H
