#ifndef VOLUMEVALUESCALC_H
#define VOLUMEVALUESCALC_H

#include <QObject>
#include <QVector>
#include "datatypes.h"
//#include "signalanalyzer.h"


class VolumeValuesCalc : public QObject
{
    Q_OBJECT
public:
    explicit VolumeValuesCalc(QObject *parent = 0);
    void setIngs(QVector<ing> ings);
    //void setRawData();
    parameters getParams();
    void setFrequancy(int Frequancy);

signals:
    void signalParameters(parameters);
public slots:
private:
    double av_speed(QVector<ing> & ings);
    double max_speed(QVector<ing> & ings);
    double one_volume(QVector<ing> & ings);
    double minute_volume(QVector<ing> & ings);
    double all_volume(QVector<ing> & ings);
    double av_out_time(QVector<ing> & ings);
    double av_in_time(QVector<ing> & ings);
    double av_cycle_time(QVector<ing> & ings);
    double freq(QVector<ing> & ings);
    double av_temp_in(QVector<ing> & ings);
    double av_temp_out(QVector<ing> & ings);
    QVector<ing> m_ings;
    int m_Frequancy = 100;
};

#endif // VOLUMEVALUESCALC_H
