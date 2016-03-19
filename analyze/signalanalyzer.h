#ifndef SIGNALANALYZER_H
#define SIGNALANALYZER_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <QTime>
#include "datatypes.h"
#include "volumevaluescalc.h"
#include "examscontrollerl.h"

class SignalAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit SignalAnalyzer(QObject *parent = 0);
    QVector<double> getMedian();
    QVector<double> getClear();
    QVector<double> getIntegrated();
    QVector<double> getAreas();
    void setFinderPrefs(FinderPrefs prefs);    
signals:
    void Inhalations(QVector<ing>);
public slots:
    void addRawData(QVector<int> * signal);
    void setFullPatientData(VTT_Data data);
private:
    QVector<double> median(QVector<int> *signal, int start, int end, int period);
    void clearSignal(int start);
    void integrateSignal(int start);
    void findIng(int start);
    double area(double a, double b);
    QVector<int> m_raw_signal;//чистый сигнал
    QVector<double> m_median_signal;//сигнал пропущенный через медианный фильтр
    QVector<double> m_clean_signal;//сигнал после фильтра и с занулёнными промежетьками когда человек вдыхает
    QVector<double> m_int_signal;
    QVector<ing> m_ings;
    int period = 15;
    int zero_level = 1540;
    int zero_sigma = 15;
    //VolumeValuesCalc m_vol_calc;

};

#endif // SIGNALANALYZER_H
