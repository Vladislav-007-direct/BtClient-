#include "freeimucal.h"
#include "ui_freeimu_cal.h"

FreeIMUCal::FreeIMUCal(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::FreeIMUCal) {
    // Set up the user interface from Designer.
    ui->setupUi(this);

    ser = std::make_shared<QSerialPort>();

    // load user settings
    settings =
        new QSettings("FreeIMU Calibration Application", "Fabio Varesano");
    // restore previous serial port used
    ui->serialPortEdit->setText(
        settings->value("calgui/serialPortEdit", "COM1").toString());

    // when user hits enter, we generate the clicked signal to the button so
    // that connections
    connect(ui->serialPortEdit, &QLineEdit::returnPressed, ui->connectButton,
            &QPushButton::click);

    // Connect up the buttons to their functions
    connect(ui->connectButton, &QPushButton::clicked, this,
            &FreeIMUCal::serial_connect);
    connect(ui->samplingToggleButton, &QPushButton::clicked, this,
            &FreeIMUCal::sampling_start);
    set_status("Disconnected");

    // data storages
    acc_data.resize(3);
    magn_data.resize(3);

    // setup graphs
    ui->accXY->setXRange(-acc_range, acc_range);
    ui->accXY->setYRange(-acc_range, acc_range);

    ui->accYZ->setXRange(-acc_range, acc_range);
    ui->accYZ->setYRange(-acc_range, acc_range);
    ui->accZX->setXRange(-acc_range, acc_range);
    ui->accZX->setYRange(-acc_range, acc_range);

    //    ui->accXY->setAspectLocked();
    //    ui->accYZ->setAspectLocked();
    //    ui->accZX->setAspectLocked();

    ui->magnXY->setXRange(-magn_range, magn_range);
    ui->magnXY->setYRange(-magn_range, magn_range);
    ui->magnYZ->setXRange(-magn_range, magn_range);
    ui->magnYZ->setYRange(-magn_range, magn_range);
    ui->magnZX->setXRange(-magn_range, magn_range);
    ui->magnZX->setYRange(-magn_range, magn_range);

    //    ui->magnXY->setAspectLocked();
    //    ui->magnYZ->setAspectLocked();
    //    ui->magnZX->setAspectLocked();

    ui->accXY_cal->setXRange(-1.5, 1.5);
    ui->accXY_cal->setYRange(-1.5, 1.5);
    ui->accYZ_cal->setXRange(-1.5, 1.5);
    ui->accYZ_cal->setYRange(-1.5, 1.5);
    ui->accZX_cal->setXRange(-1.5, 1.5);
    ui->accZX_cal->setYRange(-1.5, 1.5);

    //    ui->accXY_cal->setAspectLocked();
    //    ui->accYZ_cal->setAspectLocked();
    //    ui->accZX_cal->setAspectLocked();

    ui->magnXY_cal->setXRange(-1.5, 1.5);
    ui->magnXY_cal->setYRange(-1.5, 1.5);
    ui->magnYZ_cal->setXRange(-1.5, 1.5);
    ui->magnYZ_cal->setYRange(-1.5, 1.5);
    ui->magnZX_cal->setXRange(-1.5, 1.5);
    ui->magnZX_cal->setYRange(-1.5, 1.5);

    //    ui->magnXY_cal->setAspectLocked();
    //    ui->magnYZ_cal->setAspectLocked();
    //    ui->magnZX_cal->setAspectLocked();

    //    ui->acc3D->opts['distance'] = 30000;
    // self.acc3D.show();

    //    ui->magn3D->opts['distance'] = 2000;
    // self.magn3D.show();

    ui->acc3D->setSize(20000, 20000, 20000);
    ui->magn3D->setSize(1000, 1000, 1000);

    // axis for the cal 3D graph
    ui->acc3D_cal->setSize(10000, 10000, 10000);
    ui->magn3D_cal->setSize(1000, 1000, 1000);
}

FreeIMUCal::~FreeIMUCal() {
    delete ui;
    delete settings;
    ser->close();
    delete serWorker;
}

void FreeIMUCal::set_status(QString status) {
    ui->statusbar->showMessage(status);
}

void FreeIMUCal::serial_connect() {
    serial_port = ui->serialPortEdit->text();
    // save serial value to user settings
    settings->setValue("calgui/serialPortEdit", serial_port);

    ui->connectButton->setEnabled(false);
    // waiting mouse cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    set_status("Connecting to " + serial_port + " ...");

    // TODO : serial port field input validation !

    try {
        ser->setPort(QSerialPortInfo(serial_port));
        ser->setBaudRate(115200);
        ser->setParity(QSerialPort::Parity::NoParity);
        ser->setStopBits(QSerialPort::StopBits::OneStop);
        ser->setDataBits(QSerialPort::DataBits::Data8);

        if (ser->isOpen()) {

            qDebug() << "Arduino serial port opened correctly";
            set_status("Connection Successfull. Waiting for Arduino reset...");
            // wait for arduino reset on serial open
            QThread::sleep(3);
            // clear serial interface buffers
            ser->clear();
            QThread::msleep(100);
            ser->write("v", 1); // ask version

            set_status("Connected to: " + QString(ser->readLine(100000)));

            // TODO hangs if a wrong serial protocol has been loaded.To be
            // fixed.

            ui->connectButton->setText("Disconnect");
            connect(ui->connectButton, &QPushButton::clicked, this,
                    &FreeIMUCal::serial_disconnect);

            ui->serialPortEdit->setEnabled(false);
            ui->serialProtocol->setEnabled(false);

            ui->samplingToggleButton->setEnabled(true);

            ui->clearCalibrationEEPROMButton->setEnabled(true);
            connect(ui->clearCalibrationEEPROMButton, &QPushButton::clicked,
                    this, &FreeIMUCal::clear_calibration_eeprom);
        }
    } catch (...) {
        ui->connectButton->setEnabled(true);
        set_status("Impossible to connect: error id" +
                   QString::number(ser->error()));
    }

    // restore mouse cursor
    QApplication::restoreOverrideCursor();
    ui->connectButton->setEnabled(true);
}

void FreeIMUCal::serial_disconnect() {
    qDebug() << "Disconnecting from " + serial_port;
    ser->close();
    set_status("Disconnected");
    ui->serialPortEdit->setEnabled(true);
    ui->serialProtocol->setEnabled(true);

    ui->connectButton->setText("Connect");
    disconnect(ui->connectButton, &QPushButton::clicked, this,
               &FreeIMUCal::serial_disconnect);
    connect(ui->connectButton, &QPushButton::clicked, this,
            &FreeIMUCal::serial_connect);

    ui->samplingToggleButton->setEnabled(false);

    ui->clearCalibrationEEPROMButton->setEnabled(false);
    disconnect(ui->clearCalibrationEEPROMButton, &QPushButton::clicked, this,
               &FreeIMUCal::clear_calibration_eeprom);
}

void FreeIMUCal::sampling_start() {
    serWorker = new SerialWorker(ser);
    connect(serWorker, &SerialWorker::new_data_signal, this,
            &FreeIMUCal::newData);

    serWorker->start();
    qDebug() << "Starting SerialWorker";
    ui->samplingToggleButton->setText("Stop Sampling");
    disconnect(ui->samplingToggleButton, &QPushButton::clicked, this,
               &FreeIMUCal::sampling_start);
    connect(ui->samplingToggleButton, &QPushButton::clicked, this,
            &FreeIMUCal::sampling_end);
}

void FreeIMUCal::sampling_end()
{

}

void FreeIMUCal::calibrate()
{

}

void FreeIMUCal::save_calibration_eeprom()
{

}

void FreeIMUCal::clear_calibration_eeprom()
{

}

void FreeIMUCal::newData(QVector<short>)
{

}

SerialWorker::SerialWorker(std::shared_ptr<QSerialPort> ser, QObject* parent)
    : QThread(parent),
      ser(ser),
      exiting(false) {
}

SerialWorker::~SerialWorker() {
    exiting = true;
    wait();
    qDebug() << "SerialWorker exits..";
}

void SerialWorker::run() {
    qDebug() << "sampling start..";
    acc_file.setFileName(acc_file_name);
    acc_file.open(QFile::WriteOnly);
    magn_file.setFileName(magn_file_name);
    magn_file.open(QFile::WriteOnly);
    int count = 100;
    int in_values = 9;
    QVector<int16_t> reading(in_values, 0);
    // read data for calibration
    while (!exiting) {
        // determine word size
        ser->write("b");
        ser->write(QString(QChar(count)).toUtf8());
        for (int j = 0; j < count; ++j) {
            for (int i = 0; i < in_values; ++i) {
                if (word == 4) {
                    QDataStream stream(ser->read(4));
                    stream >> reading[i];
                }
                if (word == 2) {
                    QDataStream stream(ser->read(2));
                    stream >> reading[i];
                }
            }
            ser->read(2);
            if (reading[8] == 0) {
                reading[6] = 1;
                reading[7] = 1;
                reading[8] = 1;
            }
            // prepare readings to store on file
            QString acc_readings_line =
                QString("%d %d %d\r\n").arg(reading[0], reading[1], reading[2]);
            acc_file.write(acc_readings_line.toUtf8());
            QString magn_readings_line =
                QString("%d %d %d\r\n").arg(reading[6], reading[7], reading[8]);
            magn_file.write(magn_readings_line.toUtf8());
        }

        // every count times we pass some data to the GUI
        emit new_data_signal(reading);
        qDebug() << ".";
    }

    // closing acc and magn files
    acc_file.close();
    magn_file.close();
    return;
}
