#include "freeimucal.h"
#include "ui_freeimu_cal.h"

FreeIMUCal::FreeIMUCal(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::FreeIMUCal) {
    ui->setupUi(this);
}

FreeIMUCal::~FreeIMUCal() {
}
