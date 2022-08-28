#ifndef CALLIB_H
#define CALLIB_H

#include <QVector>

class CalLib {
public:
    CalLib();
    static QPair<QVector<long>, QVector<double>>&& calibrate(double x, double y,
                                                             double z);
    static QPair<QVector<long>, QVector<double>>&&
    calibrate_from_file(QString file_name);

    static QVector<QVector<double>>
    compute_calibrate_data(QVector<QVector<double>>& data,
                           QVector<long>& offsets, QVector<double>& scale);
};

#endif // CALLIB_H
