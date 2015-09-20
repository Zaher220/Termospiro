#ifndef PATIENTPROFILECONTROL_H
#define PATIENTPROFILECONTROL_H

#include <QObject>
#include "./models/TSPatients.h"
#include "ui_tsview.h"
#include <QSql>
#include <QSqlRecord>
#include "./models/TSExaminations.h"

class PatientProfileControl : public QObject
{
    Q_OBJECT
public:
    explicit PatientProfileControl(QObject *parent = 0);

signals:

public slots:
    void openPatientProfile(QModelIndex ind);
    void editPatientProfile();
    void savePatientProfile();
    void rejectPatientProfile();
private:
    TSPatients *patientsModel;
    Ui::TSView *ui;
};

#endif // PATIENTPROFILECONTROL_H
