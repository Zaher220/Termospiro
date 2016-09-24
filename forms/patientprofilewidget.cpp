#include "patientprofilewidget.h"
#include "ui_patientprofilewidget.h"

PatientProfileWidget::PatientProfileWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientProfileWidget)
{
    ui->setupUi(this);
}

PatientProfileWidget::~PatientProfileWidget()
{
    delete ui;
}
