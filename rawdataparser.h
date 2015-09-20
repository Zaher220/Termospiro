#ifndef RAWDATAPARSER_H
#define RAWDATAPARSER_H

#include <QObject>

class RawDataParser : public QObject
{
    Q_OBJECT
public:
    explicit RawDataParser(QObject *parent = 0);

signals:
    void changeProgress(int);
    void sendNewData(QVector<double>,QVector<double>,QVector<double>);
public slots:
    void  setACQData(std::vector<std::vector<short>>);
};

#endif // RAWDATAPARSER_H
