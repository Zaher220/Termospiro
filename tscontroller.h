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
    void editPatientProfile();
    void savePatientProfile();
    void deletePatient(int index);
    void rejectPatientProfile();
    void completePatientName(QString string);
    void completePatientId();

    void calibrateVolume();
    void rejectColibration();
    void plotCalibration();

    void startExam();
    void stopExam();
    void breakExam();
    void plotNow();

    void openPatientList();
    void openPatientProfile(QModelIndex ind);

    void openExam(QModelIndex ind);
    void createNewExam();
    void deleteExam(int index);

    void showAverageData(int avgTempIn, int avgTempOut,int avgDO, int avgCHD);
    void scrollGraphics(int value);
    void scaleTempIn(int value);
    void scaleTempOut(int value);
    void scaleVolume(int value);
    void scaleForHorizontal(int value);
    void changeScrollBarAfterScaling(int before,int after);
    void changeTempInScrollValue(int value);

    void processDataParams();
    void printReport();
protected:
    void initPaintDevices();
    void resizeEvent(QResizeEvent *evt);
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *e);
    void openPrivateDB(QSqlRecord record);    
private slots:
    void on_examsTableView_doubleClicked(const QModelIndex &index);

private:
    ADCDataReader m_adc_reader;
    RawDataParser m_raw_data_parser;

    QWidget wpf;
    QDialog *mvlDialog;
    QTableWidgetItem* getQTableWidgetItem(QVariant text);
    Ui::TSView *ui;
    Ui::MainWindow *w;
    Ui::TSVolSignalWidget *volWidget;
    CurrentAction currentAction;

    bool openUser;
    //Все для рисования
    TSCurveBuffer* curveBuffer;
    QTimer plotingTimer;
    QTimer cPlotingTimer;
    QPixmap bcVolume;
    QPixmap bVolume;
    QPixmap bTempIn;
    QPixmap bTempOut;
    QPainter pcVolume;
    QPainter pVolume;
    QPainter pTempIn;
    QPainter pTempOut;
    int* volume;
    int* tempIn;
    int* tempOut;
    int screenLimit;
    int startIndex;
    int W;
    int H;
    int cH;
    int cW;
    bool recordingStarted;
    double volumeScaleRate;
    double tempInScaleRate;
    double tempOutScaleRate;
    double horizontalStep;
    int tempInZerPos;
    int tempOutZerPos;
    int scaleScroll[5];
    int* tempInInterval;
    double tempInAdaptive;
    int* tempOutInterval;
    double tempOutAdaptive;
    double volumeAdaptive;
    int maxcVol;

    //модели
    TSPatients *patientsModel;
    TSExaminations *examinationsModel;
    QSqlDatabase patientsConnection;
    QSqlDatabase examinationsConnection;
    bool isInitialized;
    QTime myTimer;

};

#endif // TSVIEW_H
