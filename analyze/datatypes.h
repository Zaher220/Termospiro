#ifndef DATATYPES_H
#define DATATYPES_H
#include "QDebug"

struct FinderPrefs{
    int median_period = 15;//Период по которому будем считать медиану
    int zero_sigma = 15;//Отклонение от нулевого уровня объёма считающееся шумом
    int zero_level = 1540;//Нулевой уровень объёма
};

struct ing{
    int start_index = 0;
    int end_index = 0;
    double area = 0;
    void calcArea(QVector<int> * signal){
        ;
    }
};

struct parameters{
    double av_speed = 0;//Средняя скорость выдоха
    double max_speed = 0;//максимальная скорость выдоха
    double one_volume = 0;//объём выдоха
    double minute_volume = 0;//Минутный объём
    double all_volume = 0;//Суммарный объём
    double av_out_time = 0;//Среднее время выдоха
    double av_in_time = 0; //Среднее время вдоха
    double av_cycle_time = 0;//среднее время цикла
    double freq = 0;//Частота дыхания
    double av_temp_in = 0;//средняя температура вдоха
    double av_temp_out = 0;//Средняя температура выдоха
    void debug(){
        qDebug()<<"av_speed"<<av_speed;
        qDebug()<<"max_speed"<<max_speed;
        qDebug()<<"one_volume"<<one_volume;
        qDebug()<<"minute_volume"<<minute_volume;
        qDebug()<<"all_volume"<<all_volume;
        qDebug()<<"av_out_time"<<av_out_time;
        qDebug()<<"av_in_time"<<av_in_time;
        qDebug()<<"av_cycle_time"<<av_cycle_time;
        qDebug()<<"freq"<<freq;
        qDebug()<<"av_temp_in"<<av_temp_in;
        qDebug()<<"av_temp_out"<<av_temp_out;
    }
};


#endif // DATATYPES_H
