#include "volumevaluescalc.h"

VolumeValuesCalc::VolumeValuesCalc(QObject *parent) : QObject(parent)
{

}

void VolumeValuesCalc::setIngs(QVector<ing> ings)
{
    m_ings = ings;
    emit signalParameters(this->getParams());
}

parameters VolumeValuesCalc::getParams()
{
    parameters ps;
    if(m_ings.size()==0)
        return ps;
    ps.all_volume = all_volume(m_ings);
    ps.av_speed = av_speed(m_ings);
    ps.max_speed = this->max_speed(m_ings);
    ps.one_volume = this->one_volume(m_ings);
    ps.minute_volume = this->minute_volume(m_ings);
    ps.freq = this->freq(m_ings);
    ps.av_out_time = this->av_out_time(m_ings);
    return ps;
}

double VolumeValuesCalc::av_speed(QVector<ing> &ings)
{
    long double average_speed = 0;
    long double speed = 0;
    for(int i = 0; i < ings.size(); i++){
        speed = ings[i].area / (ings.at(i).end_index - ings.at(i).start_index);
        average_speed = ((average_speed * i) + speed) / (i + 1);
    }
    average_speed = average_speed * m_Frequancy;//Преобразуем в единицы объёма в секунду

    return average_speed;
}

double VolumeValuesCalc::max_speed(QVector<ing> &ings)
{
    long double maximum_speed = 0;
    long double speed = 0;
    for(int i = 0; i < ings.size(); i++){
        speed = ings[i].area / (ings.at(i).end_index - ings.at(i).start_index);
        if( fabs(speed) > fabs(maximum_speed) )
            maximum_speed = speed;
    }
    maximum_speed = maximum_speed * m_Frequancy;//Преобразуем в единицы объёма в секунду

    return maximum_speed;
}

double VolumeValuesCalc::one_volume(QVector<ing> &ings)//Средний дыхательный объём
{
    long double average_volume = 0;
    long double volume = 0;
    for(int i = 0; i < ings.size(); i++){
        volume = ings[i].area;
        average_volume = ((average_volume * i) + volume) / (i + 1);
    }
    return average_volume;
}

double VolumeValuesCalc::minute_volume(QVector<ing> &ings)
{
    long double summ_volume = this->all_volume(ings);
    long double full_time =  ings.last().end_index - ings.first().start_index;
    long double minute_vol = summ_volume / ( full_time / (m_Frequancy * 60) );
    return minute_vol;
}

double VolumeValuesCalc::all_volume(QVector<ing> &ings)
{
    double summ = 0;
    foreach (ing i, ings) {
        summ += i.area;
    }
    return summ;
}

double VolumeValuesCalc::av_out_time(QVector<ing> &ings)
{
    long double average_time = 0;
    long double one_time = 0;
    for(int i = 0; i < ings.size(); i++){
        one_time = ings[i].end_index - ings[i].start_index;
        average_time = ((average_time * i) + one_time) / (i + 1);
    }
    average_time = average_time / m_Frequancy;//Преобразуем такты ацп в секунды
    return average_time;
}

double VolumeValuesCalc::av_in_time(QVector<ing> &ings)
{
    return -1;
}

double VolumeValuesCalc::av_cycle_time(QVector<ing> &ings)
{
    return -1;
}

double VolumeValuesCalc::freq(QVector<ing> &ings)//Частота дыхания единицы в минуту
{
    long double inh_number =  ings.size();
    long double full_time =  ings.last().end_index - ings.first().start_index;
    long double inh_freq = inh_number / ( full_time / (m_Frequancy * 60) );
    return inh_freq;
}

double VolumeValuesCalc::av_temp_in(QVector<ing> &ings)
{
    return -1;
}

double VolumeValuesCalc::av_temp_out(QVector<ing> &ings)
{
    return -1;
}

void VolumeValuesCalc::setFrequancy(int Frequancy)
{
    m_Frequancy = Frequancy;
}
