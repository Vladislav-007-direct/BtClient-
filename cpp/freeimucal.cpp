#include "freeimucal.h"
#include "ui_freeimu_cal.h"

FreeIMUCal::FreeIMUCal(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::FreeIMUCal) {
    // Set up the user interface from Designer.
    ui->setupUi(this);

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
}

void FreeIMUCal::set_status(QString status) {
    ui->statusbar->showMessage(status);
}
