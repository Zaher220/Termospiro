#ifndef ADCDATAREADER_H
#define ADCDATAREADER_H

#include <QObject>
#include "Rtusbapi.h"
#include "QDebug"
#include "tscurvebuffer.h"
#include "extremum.h"
#include <QElapsedTimer>
#include <QDateTime>
#include <QPair>

class ADCDataReader : public QObject
{
    Q_OBJECT
public:
    explicit ADCDataReader(QObject *parent = 0);
    ~ADCDataReader();
    bool isReady();
    TSUsbReadingType readingType() const;
    void setReadingType(const TSUsbReadingType &readingType);
signals:
    void sendNewData(QVector<double> volume, QVector<double> tempin, QVector<double> tempout);
public slots:
    void startADC();
    void stopADC();
private:
    void acquireData();
    bool initDevice();

    DWORD DllVersion;
    IRTUSB3000 *pModule = NULL; // указатель на интерфейс модуля
    const static WORD CHANNELS_QUANTITY = 0x4; // кол-во опрашиваемых каналов модуля
    SHORT AdcBuffer[CHANNELS_QUANTITY]; // буфер данных для кадра отсчётов
    TSUsbReadingType m_readingType;
    bool ReadingStarted = false;
    QVector< QPair< double,QString > > vol, tin, tout;
    QVector<double> m_vol, m_tin, m_tout;

    const QString dateformat="mm;ss;zzz";
};

#endif // ADCDATAREADER_H
