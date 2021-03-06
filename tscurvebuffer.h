#ifndef TSCURVEBUFFER_H
#define TSCURVEBUFFER_H

#include <QObject>
#include <QColor>
#include <QSettings>
#include <tsanalitics.h>
#include <QVector>
#include <tsanalitics.h>
#include <tstempanalitic.h>
#include <QThread>
#include "tsrealcalcwrapper.h"
using namespace std;

#define VOLTAGE_RATE    0.61
#define REF_VOLTAGE_1   ts_tempInVolt
#define REF_VOLTAGE_2   ts_tempOutVolt
#define TAN_1           ts_tanTempIn
#define TAN_2           ts_tanTempOut
#define REF_TEMP        ts_refTemp

struct CurvesSegnments
{
    int curV,prevV;
    int curTin,prevTin;
    int curTout,prevTout;
};
class TSCurveBuffer : public QObject{
    Q_OBJECT
public:
    explicit TSCurveBuffer(QObject *parent = 0);
    ~TSCurveBuffer();
    int end();
    CurvesSegnments lastSegments();
    int* tempIn();
    int* tempOut();
    int* volume();
    void setVolumeColibration(int c,bool realtime);
    void setValues(int* volume,int* tin,int* tout, int n);
    void append(int v, int tI, int tO, bool realtime = true);
    int startIndex();
    void setStartIndex(int s);
    void setEnd(int n);
    int* getTempInInterval();
    int* getTempOutInterval();
    int* getVolumeInterval();
    int volumeColibration();
    void setVolumeConverts(int pos,int neg);
    QVector<int> volumeConverts();
    float volToLtr(int vol);
    int setReference(QSettings *set);
    float tempInToDeg(int temp);
    float tempOutToDeg(int temp);
    int getLenght();
    void clean();
    void setLenght(int l);
    QColor volColor;
    QColor tinColor;
    QColor toutColor;
    int lenght;
signals:
    void changed(CurvesSegnments s);
    void overflowed();
    void updateAverageData(int avgTempIn, int avgTempOut, int avgDo, int ChD);
public slots:
    void updateAvData(int avgTempIn, int avgTempOut, int avgDo, int ChD);
private:
    int ts_volume[18000];
    int ts_tempIn[18000];
    int ts_tempOut[18000];
    int ts_integral[18000];
    int ts_end;
    int ts_volumeColibration;
    int ts_screenLimit;
    int ts_startIndex;
    int max_v,maxc_v,min_v,minc_v;
    int ts_minTempIn;
    int ts_maxTempIn;
    int ts_minTempOut;
    int ts_maxTempOut;
    int ts_minVolume;
    int ts_maxVolume;
    int ts_volumePosConvert;
    int ts_volumeNegConvert;
    float ts_tanTempIn;
    float ts_tanTempOut;
    int ts_tempInVolt;
    int ts_tempOutVolt;
    int ts_refTemp;
    QVector<int> BreathVolumes;
    tsRealCalcWrapper paramcalc;
};

#endif // TSCURVEBUFFER_H
