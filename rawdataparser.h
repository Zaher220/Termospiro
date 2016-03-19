#ifndef RAWDATAPARSER_H
#define RAWDATAPARSER_H

#include <QObject>
#include <QVector>
#include <QDateTime>
#include "src/datatypes.h"

class RawDataParser : public QObject
{
    Q_OBJECT
public:
    explicit RawDataParser(QObject *parent = 0);
    ~RawDataParser();
signals:
    void changeProgress(int);
    void sendNewData(QVector<int>, QVector<int>, QVector<int>);
public slots:
    void  setACQData(ADCData data);
private:
    FILE* out;
};

#endif // RAWDATAPARSER_H
