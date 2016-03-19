#ifndef TSVIEW_H
#define TSVIEW_H

#include <QMainWindow>
#include <tscurvebuffer.h>
#include <QPixmap>
#include <QPainter>
#include "models/TSExaminations.h"
#include "models/TSPatients.h"
#include <QTableWidget>
#include "ui_tsprintview.h"
#include "volumesolver.h"
#include "ADCDataReader.h"
#include "rawdataparser.h"
#include "examscontrollerl.h"
#include "reportprinter.h"
#include "plotterwidjet.h"
#include "calibrationpkotterwidjet.h"

#include "analyze/signalanalyzer.h"
#include "analyze/volumevaluescalc.h"

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

    void processDataParams();
    void printReport();
    void stopCalibration();

    void setSignalParameters(parameters params);
protected:
    void closeEvent(QCloseEvent *e);
    void openPrivateDB(QSqlRecord record);
private slots:
    void on_backCallibrateButton_clicked();
    void getZeroSognalLevels();
private:
    ADCDataReader *m_adc_reader;
    RawDataParser m_raw_data_parser;
    TSCurveBuffer curveBuffer;

    QWidget wpf;
    QDialog *mvlDialog;
    QTableWidgetItem* getQTableWidgetItem(QVariant text);
    Ui::TSView ui;

    Ui::TSVolSignalWidget *volWidget;

    bool recordingStarted;

    ExamsController m_exam_cntrl;
    ReportPrinter m_reports;
    PlotterWidjet * m_plotter_widjet = nullptr;
    CalibrationPkotterWidjet * m_calib_plotter = nullptr;
    VolumeValuesCalc * m_volumes_calc = nullptr;
    SignalAnalyzer * m_signal_analyzer = nullptr;
};

#endif // TSVIEW_H
