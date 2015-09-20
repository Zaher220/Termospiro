#ifndef TSVIEW_H
#define TSVIEW_H

#include <QMainWindow>
#include <QMessageBox>
#include <tscurvebuffer.h>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QRegExp>
#include "models/TSExaminations.h"
#include "models/TSPatients.h"
#include <QTableWidget>
#include "ui_tsprintview.h"
#include "volumesolver.h"
#include <QTime>
#include "ADCDataReader.h"
#include "rawdataparser.h"
#include "plotter.h"
//#include "patientprofilecontrol.h"

namespace Ui {
class TSView;
class TSProgressDialog;
class MainWindow;
class TSVolSignalWidget;
class Form;
}

enum CurrentAction {NoAction,CreatePatientProfileAction,EditPatientProfileAction,GetingVolZero};

class TSController : public QMainWindow
{
    Q_OBJECT
public:
    explicit TSController(QWidget *parent = 0);
    ~TSController();
public slots:
    void openPatientProfile(QModelIndex ind);
    void editPatientProfile();
    void savePatientProfile();
    void rejectPatientProfile();
    void deletePatient(int index);
    void completePatientName(QString string);
    void completePatientId();

    void calibrateVolume();
    void rejectColibration();

    void startExam();
    void stopExam();
    void breakExam();

    void openPatientList();

    void openExam(QModelIndex ind);
    void createNewExam();
    void deleteExam(int index);

    void showAverageData(int avgTempIn, int avgTempOut,int avgDO, int avgCHD);
    void scrollGraphics(int value);
    void changeScrollBarAfterScaling(int before,int after);

    void processDataParams();
    void printReport();
protected:
    void resizeEvent(QResizeEvent *evt);
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *e);
    void openPrivateDB(QSqlRecord record);
private slots:
    void on_examsTableView_doubleClicked(const QModelIndex &index);
private:
    ADCDataReader m_adc_reader;
    RawDataParser m_raw_data_parser;
    TSCurveBuffer curveBuffer;
    Plotter m_plotter;
    // PatientProfileControl m_profile_control;

    QWidget wpf;
    QDialog *mvlDialog;
    QTableWidgetItem* getQTableWidgetItem(QVariant text);
    Ui::TSView *ui;
    Ui::MainWindow *w;
    Ui::TSVolSignalWidget *volWidget;
    CurrentAction currentAction;

    bool openUser;
    //Все для рисования


    bool recordingStarted;
    //модели
    TSPatients *patientsModel;
    TSExaminations *examinationsModel;
    QSqlDatabase patientsConnection;
    QSqlDatabase examinationsConnection;
    bool isInitialized;
    QTime myTimer;

};

#endif // TSVIEW_H
