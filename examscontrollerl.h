#ifndef EXAMSMODEL_H
#define EXAMSMODEL_H

#include <QObject>
#include "models/TSExaminations.h"
#include "models/TSPatients.h"
#include "ui_tsview.h"
#include <QSqlRecord>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDir>
#include <QDate>
#include <QTime>
#include <QSqlDriver>

enum CurrentAction {NoAction,CreatePatientProfileAction,EditPatientProfileAction,GetingVolZero};

class ExamsController : public QObject
{
    Q_OBJECT
public:
    explicit ExamsController(QObject *parent = 0);

    void setm_ui(Ui::TSView *ui);
    void openPrivateDB(QSqlRecord record);
    void setAction(CurrentAction act);
    bool init();
    QSqlError getErrorPatientsConnection();
    QSqlError getErrorexaminationsConnection();
signals:

public slots:
    void writeExamToDB(QVector<int> volume, QVector<int> tempIn, QVector<int> tempOut, double volIn, double volOut);
    void CreatePatientProfile();
    void savePatientProfile();
    void setAction();
    void openPatientList();

    //void EditPatientProfileAction();
private:
    Ui::TSView *m_ui = nullptr;
    TSExaminations *examinationsModel = nullptr;
    TSPatients *patientsModel = nullptr;
    CurrentAction currentAction;

    QSqlDatabase patientsConnection;
    QSqlDatabase examinationsConnection;
};

#endif // EXAMSMODEL_H

