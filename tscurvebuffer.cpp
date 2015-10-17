#include "tscurvebuffer.h"
#include <QDebug>
#include <tsanalitics.h>
#include <tstempanalitic.h>

TSCurveBuffer::TSCurveBuffer(QObject *parent) :QObject(parent){
    ts_volume.resize(18000);
    ts_tempIn.resize(18000);
    ts_tempOut.resize(18000);
    ts_integral.resize(18000);

    ts_end = -1;
    ts_volumeColibration = 0;
    ts_volumePosConvert=1;
    ts_volumeNegConvert=-1;
    max_v=-1100000;
    min_v=10000;
    moveToThread(&m_thread);
    //connect(this, SIGNAL(finished()), &m_thread, SLOT())
    m_thread.start();

}

TSCurveBuffer::~TSCurveBuffer(){
    m_thread.quit();
}

int TSCurveBuffer::end(){
    return ts_end;
}

void TSCurveBuffer::append(int v, int tI, int tO, bool realtime){
    if(ts_end > 18000){
        emit overflowed();
        return;
    }
    if(realtime){
        v*= VOLTAGE_RATE;
        tI*= VOLTAGE_RATE;
        tO*= VOLTAGE_RATE;
    }
    if(ts_end == 0){
        ts_minTempIn = tI;
        ts_maxTempIn = tI;
        ts_minTempOut = tO;
        ts_maxTempOut = tO;
    }else{
        if(tI<=ts_minTempIn)
            ts_minTempIn=tI;
        if(tI>ts_maxTempIn)
            ts_maxTempIn=tI;
        if(tO<=ts_minTempOut)
            ts_minTempOut=tO;
        if(tO>ts_maxTempOut)
            ts_maxTempOut=tO;
    }

    v -= ts_volumeColibration;

    CurvesSegnments segs;
    if(ts_end > 0){
        if (ts_integral[ts_end-1]>=max_v){
            max_v=ts_integral[ts_end-1];
            maxc_v=ts_end-1;
        }
        if (ts_integral[ts_end-1]<=min_v){
            min_v=ts_integral[ts_end-1];
            minc_v=ts_end-1;
        }
        segs.prevV = ts_volume[ts_end];
        segs.prevTin = ts_tempIn[ts_end];
        segs.prevTout = ts_tempOut[ts_end];
        if(abs(v)>=8){
            if(realtime)
                ts_integral[ts_end] = 0.1*v + ts_integral[ts_end-1];
            else
                ts_integral[ts_end] = v;
        }
        else
            ts_integral[ts_end]=0;
        if(ts_integral[ts_end]>ts_maxVolume)
            ts_maxVolume=ts_integral[ts_end];
        if(ts_integral[ts_end]<ts_minVolume)
            ts_minVolume=ts_integral[ts_end];
    }else{
        ts_integral[0] = 0;
        ts_minVolume = ts_integral[0];
        ts_minVolume = ts_integral[0];
    }
    if (ts_end > 0){
        ts_volume[ts_end] = segs.curV = v;
        ts_tempIn[ts_end] = segs.curTin = tI;
        ts_tempOut[ts_end] = segs.curTout = tO;
    }
    //emit changed(segs);
    /*if(realtime && ts_end > 0){
        paramcalc.addData(ts_tempIn[ts_end], ts_tempOut[ts_end], ts_integral[ts_end]);
        if ( ts_end%100 == 0 ){
            emit updateAverageData(paramcalc.getAvgTempIn(),paramcalc.getAvgTempOut(),paramcalc.getAvgInspirationVolume(),paramcalc.getInspirationFreqency());
            qDebug()<<"(paramcalc.getAvgTempIn()"<<paramcalc.getAvgTempIn()<<" paramcalc.getAvgTempOut()"<<paramcalc.getAvgTempOut()<<" paramcalc.getAvgInspirationVolume()"<<paramcalc.getAvgInspirationVolume();
        }

    }*/
    ts_end++;
}

void TSCurveBuffer::setVolumeColibration(int c, bool realtime = true){
    if(realtime)
        ts_volumeColibration = c * VOLTAGE_RATE;
    else
        ts_volumeColibration = c;
}

int TSCurveBuffer::startIndex(){
    return ts_startIndex;
}

void TSCurveBuffer::setStartIndex(int s){
    ts_startIndex = s;
}

void TSCurveBuffer::setEnd(int n){
    ts_end = n;
}

QPair<int, int> TSCurveBuffer::getTempInIntervalPair(){
    QPair<int, int> interval;
    if( abs(ts_maxTempIn-ts_minTempIn) < 500 ){
        interval.first = -5000;
        interval.second = 5000;
    }else{
        interval.first = ts_minTempIn-100;
        interval.second = ts_maxTempIn+100;
    }
    return interval;
}

QPair<int, int> TSCurveBuffer::getTempOutIntervalPair(){
    QPair<int, int> interval;
    if( abs(ts_maxTempOut-ts_minTempOut) < 500 ){
        interval.first = -5000;
        interval.second = 5000;
    }else{
        interval.first = ts_minTempOut - 100;
        interval.second = ts_minTempOut + 100;
    }
    return interval;
}

QPair<int, int> TSCurveBuffer::getVolumeIntervalPair(){
    QPair<int, int> interval;
    if( abs(ts_maxVolume-ts_minVolume) < 250){
        interval.first = -5000;
        interval.second = 5000;
    }else{
        if( abs(ts_minVolume) > ts_maxVolume){
            interval.first = ts_minVolume - 120;
            interval.second = (-ts_minVolume) + 100;
        }else{
            interval.first = -ts_maxVolume - 120;
            interval.second = ts_maxVolume+100;
        }
    }
    return interval;
}

int* TSCurveBuffer::getTempInInterval(){
    int *interval = new int[2];
    if(abs(ts_maxTempIn-ts_minTempIn)<500){
        interval[0]=-5000;
        interval[1]=5000;
    }else{
        interval[0]=ts_minTempIn-100;
        interval[1]=ts_maxTempIn+100;
    }
    return interval;
}

int* TSCurveBuffer::getTempOutInterval(){
    int *interval = new int[2];
    if(abs(ts_maxTempOut-ts_minTempOut)<500){
        interval[0]=-5000;
        interval[1]=5000;
    }else{
        interval[0]=ts_minTempOut-100;
        interval[1]=ts_maxTempOut+100;
    }
    return interval;
}

int* TSCurveBuffer::getVolumeInterval(){
    int *interval = new int[2];
    if(abs(ts_maxVolume-ts_minVolume)<250){
        interval[0]=-5000;
        interval[1]=5000;
    }else{
        if(abs(ts_minVolume)>ts_maxVolume){
            interval[0]=ts_minVolume-120;
            interval[1]=(-ts_minVolume)+100;
        }else{
            interval[0]=-ts_maxVolume-120;
            interval[1]=ts_maxVolume+100;
        }
    }
    return interval;
}

int TSCurveBuffer::volumeColibration(){
    return ts_volumeColibration;
}

void TSCurveBuffer::setVolumeConverts(int pos, int neg){
    ts_volumePosConvert=pos;
    ts_volumeNegConvert=neg;
}

QVector<int> TSCurveBuffer::volumeConverts(){
    QVector<int> v;
    v.push_back(ts_volumeNegConvert);
    v.push_back(ts_volumePosConvert);
    return v;
}

float TSCurveBuffer::volToLtr(int vol){
    if (vol==0)
        return 0;
    if(vol<0)
        return (float)vol/ts_volumeNegConvert;
    else
        return (float)vol/ts_volumePosConvert;
}

float TSCurveBuffer::tempInToDeg(int temp){
    return TAN_1*(REF_VOLTAGE_1-temp);//+(float)REF_TEMP;
}

float TSCurveBuffer::tempOutToDeg(int temp){
    return TAN_2*(REF_VOLTAGE_2-temp);//+(float)REF_TEMP;
}

int TSCurveBuffer::getLenght(){
    return ts_integral.length();
}

void TSCurveBuffer::clean(){
    setEnd(0);
    ts_end=0;
    ts_volume.clear();
    ts_tempIn.clear();
    ts_tempOut.clear();
    ts_integral.clear();
    ts_volume.resize(18000);
    ts_tempIn.resize(18000);
    ts_tempOut.resize(18000);
    ts_integral.resize(18000);
    paramcalc.reset();
}

int TSCurveBuffer::setReference(QSettings *set){
    if(!set->contains("RefTemp")||!set->contains("tempInVolt")||
            !set->contains("tanTempIn")||!set->contains("tempOutVolt")||
            !set->contains("tanTempOut")){
        qDebug()<<"settings.ini error";
        return 1;
    }else{
        ts_refTemp = set->value("RefTemp").toInt();
        ts_tempInVolt = set->value("tempInVolt").toInt();
        ts_tempOutVolt = set->value("tempOutVolt").toInt();
        ts_tanTempIn = set->value("tanTempIn").toDouble();
        ts_tanTempOut = set->value("tanTempOut").toDouble();
    }
    return 0;
}

QVector<int> TSCurveBuffer::tempInVector()
{
    return ts_tempIn;
}

QVector<int> TSCurveBuffer::tempOutVector()
{
    return ts_tempOut;
}

QVector<int> TSCurveBuffer::volumeVector()
{
    return ts_integral;
}

void TSCurveBuffer::updateAvData(int avgTempIn, int avgTempOut, int avgDo, int ChD){
    emit updateAverageData(avgTempIn,avgTempOut,avgDo,ChD);
}

void TSCurveBuffer::appendData(IntegerVector volume, IntegerVector tempin, IntegerVector tempout)
{
    for(int i=0; i < volume.size(); i++){
        this->append(volume[i], tempin[i], tempout[i]);
    }
}
