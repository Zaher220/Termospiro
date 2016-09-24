#include "patientslistwidget.h"
#include "ui_patientslistwidget.h"

PatientsListWidget::PatientsListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientsListWidget)
{
    ui->setupUi(this);
}

PatientsListWidget::~PatientsListWidget()
{
    delete ui;
}
