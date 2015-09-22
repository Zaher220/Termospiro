#ifndef PLOTTER_H
#define PLOTTER_H

#include <QObject>
#include <QTimer>
#include <QPainter>
#include "ui_tsview.h"
#include "tscurvebuffer.h"
#include "ui_tsprogressdialog.h"
#include <QDialog>

class Plotter : public QObject
{
    Q_OBJECT
public:
    explicit Plotter(QObject *parent = 0);

    void resetPlotting();

    Ui::TSView *getUi() const;
    void setUi(Ui::TSView *value);
    void initPaintDevices();
    TSCurveBuffer *getCurveBuffer() const;
    void setCurveBuffer(TSCurveBuffer *value);
    void scrollGraphics(int value);

    int getMaxcVol() const;
    void setMaxcVol(int value);
    void startCPlottingTimer(int m_volume);



    int getW() const;
    void setW(int value);

    double getHorizontalStep() const;
    void setHorizontalStep(double value);

    int getCH() const;
    void setCH(int value);

    int getCW() const;
    void setCW(int value);

    double getVolumeScaleRate() const;
    void setVolumeScaleRate(double value);

    double getTempInScaleRate() const;
    void setTempInScaleRate(double value);

    double getTempOutScaleRate() const;
    void setTempOutScaleRate(double value);

    int *getTempInInterval() const;
    void setTempInInterval(int *value);

    int *getTempOutInterval() const;
    void setTempOutInterval(int *value);

    double getTempInAdaptive() const;
    void setTempInAdaptive(double value);

    double getTempOutAdaptive() const;
    void setTempOutAdaptive(double value);

    QWidget *getParentWindow() const;
    void setParentWindow(QWidget *value);

    bool getRecordingStarted() const;
    void setRecordingStarted(bool value);

signals:
    void stopACQU();
public slots:
    void plotNow();
    void plotCalibration();
    void startPlottingTimer(int m_volume);
    void stopPlottingTimer();
    void scaleTempIn(int value);
    void scaleTempOut(int value);
    void scaleVolume(int value);
    void scaleForHorizontal(int value);
    void changeTempInScrollValue(int value);
private slots:


    void on_horizontalScrollBar_sliderPressed();

private:
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
    int* m_volume;
    int* m_tempIn;
    int* m_tempOut;
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
    int *tempInInterval;
    double tempInAdaptive;
    int *tempOutInterval;
    double tempOutAdaptive;
    double volumeAdaptive;
    int maxcVol;
    Ui::TSView *ui;
    TSCurveBuffer *curveBuffer;
    QWidget *parentWindow;
};

#endif // PLOTTER_H
