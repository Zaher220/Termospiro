#include "patientresearchslist.h"
#include "ui_patientresearchslist.h"

PatientResearchsList::PatientResearchsList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientResearchsList)
{
    ui->setupUi(this);
}

PatientResearchsList::~PatientResearchsList()
{
    delete ui;
}
