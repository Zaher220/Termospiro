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
#include <QMessageBox>

enum CurrentAction {NoAction,CreatePatientProfileAction,EditPatientProfileAction,GetingVolZero};
struct VTT_Data{
    QVector<int> volume,tempin, tempout;
    int volZero;
    int volOut;
    int volIn;
};

class ExamsController : public QObject
{
    Q_OBJECT
public:
    explicit ExamsController(QObject *parent = 0);

    void setm_ui(Ui::TSView *ui);

    void setAction(CurrentAction act);
    bool init();
    QSqlError getErrorPatientsConnection();
    QSqlError getErrorexaminationsConnection();
    double getMVL();
    QString getFName();
    QString getSName();
signals:
    void selectedExam(VTT_Data data);
    void reset();
public slots:
    void on_backExamButton_clicked();
    void writeExamToDB(QVector<int> volume, QVector<int> tempIn, QVector<int> tempOut, double volIn, double volOut);
    void CreatePatientProfile();
    void savePatientProfile();
    //void setAction();
    void openPatientList();
    void completePatientName(QString string);

    void openPatientProfile(QModelIndex ind);
    void deleteExam(int index);
    void deletePatient(int index);
    void openPrivateDB(QSqlRecord record);
    void openExam(QModelIndex ind);//!!!!!!
    void EditPatientProfile();
    void rejectPatientProfile();
    void on_backPatientProfileButton_clicked(bool);
    void on_backPatientListButton_clicked(bool);
    void on_openButton_clicked(bool);
private:
    Ui::TSView *m_ui = nullptr;
    TSExaminations *examinationsModel = nullptr;
    TSPatients *patientsModel = nullptr;
    CurrentAction currentAction ;

    bool openUser = false;
    QSqlDatabase patientsConnection;
    QSqlDatabase examinationsConnection;
};

#endif // EXAMSMODEL_H

