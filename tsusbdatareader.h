#ifndef TSUSBDATAREADER_H
#define TSUSBDATAREADER_H

#include <QObject>
#include "Rtusbapi.h"
#include "QDebug"
#include "tscurvebuffer.h"
#include "extremum.h"
#include <QElapsedTimer>
#include "Rtusbapi.h"
#include <QDateTime>
#include <QPair>
class TSUsbDataReader : public QObject
{
    Q_OBJECT
public:
    explicit TSUsbDataReader(QObject *parent = 0);
    ~TSUsbDataReader();
    bool initDevice();
    void stopRead();
    void setReadingType(TSUsbReadingType type);
    void setBuffer(TSCurveBuffer *bffr);
    bool isReady();
signals:
    //void done(bool isCorrect);
    void done();
    void changeProgress(int val);
    void sendNewData(QVector<double> volume, QVector<double> tempin, QVector<double> tempout);
public slots:
    void doWork();
private:
    bool read();
    void releaseReader();
    TSCurveBuffer *buffer;
    SHORT* readData();
    DWORD DllVersion;
    IRTUSB3000 *pModule = NULL; // указатель на интерфейс модуля
    const static WORD CHANNELS_QUANTITY = 0x4; // кол-во опрашиваемых каналов модуля
    SHORT AdcBuffer[CHANNELS_QUANTITY]; // буфер данных для кадра отсчётов
    TSUsbReadingType readingType;
    bool ReadingStarted = false;
    QVector< QPair< double,QString > > vol, tin, tout;
    QVector<double> m_vol, m_tin, m_tout;
   // const QString dateformat="dd/MM/yyyy HH:mm:ss:zzz";
     const QString dateformat="mm;ss;zzz";
};

#endif // TSUSBDATAREADER_H
