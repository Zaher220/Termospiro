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
#include "examscontrollerl.h"
#include "reportprinter.h"
#include "plotterwidjet.h"

namespace Ui {
class TSView;
class TSProgressDialog;
class MainWindow;
class TSVolSignalWidget;
class Form;
}



class TSController : public QMainWindow
{
    Q_OBJECT
public:
    explicit TSController(QWidget *parent = 0);
    ~TSController();
public slots:

    void selectedExamination(VTT_Data data);

    void savePatientProfile();

    void completePatientId();

    void calibrateVolume();
    void calibrationFinished();
    void rejectColibration();

    void startExam();
    void stopExam();
    void breakExam();

    void createNewExam();

    void showAverageData(int avgTempIn, int avgTempOut,int avgDO, int avgCHD);
    void scrollGraphics(int value);
    void changeScrollBarAfterScaling(int before,int after);

    void processDataParams();
    void printReport();
    void stopCalibration();
protected:
    void resizeEvent(QResizeEvent *evt);
    void closeEvent(QCloseEvent *e);
    void openPrivateDB(QSqlRecord record);
private slots:

    void on_backCallibrateButton_clicked();
    void getZeroSognalLevels();

private:
    ADCDataReader *m_adc_reader;
    RawDataParser m_raw_data_parser;
    TSCurveBuffer curveBuffer;
    Plotter m_plotter;


    QWidget wpf;
    QDialog *mvlDialog;
    QTableWidgetItem* getQTableWidgetItem(QVariant text);
    Ui::TSView ui;

    Ui::TSVolSignalWidget *volWidget;

    //Все для рисования


    bool recordingStarted;

    bool isInitialized;
    QTime myTimer;
    ExamsController m_exam_cntrl;
    ReportPrinter m_reports;
    PlotterWidjet * m_plotter_widjet = nullptr;
};

#endif // TSVIEW_H
