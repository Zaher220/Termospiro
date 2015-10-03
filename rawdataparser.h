#ifndef RAWDATAPARSER_H
#define RAWDATAPARSER_H

#include <QObject>
#include <QVector>
#include "src/datatypes.h"

class RawDataParser : public QObject
{
    Q_OBJECT
public:
    explicit RawDataParser(QObject *parent = 0);

signals:
    void changeProgress(int);
    void sendNewData(QVector<int>, QVector<int>, QVector<int>);
public slots:
    void  setACQData(AdcDataMatrix data);
};

#endif // RAWDATAPARSER_H
