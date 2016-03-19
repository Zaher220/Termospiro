#include "signalanalyzer.h"

SignalAnalyzer::SignalAnalyzer(QObject *parent) : QObject(parent)
{

}

QVector<double> SignalAnalyzer::getMedian()
{
    return m_median_signal;
}

QVector<double> SignalAnalyzer::getClear()
{
    return m_clean_signal;
}

QVector<double> SignalAnalyzer::getIntegrated()
{
    return m_int_signal;
}

QVector<double> SignalAnalyzer::getAreas()
{
    QVector<double> res;
    for(auto & i: m_ings){
        res.push_back(i.area);
    }
    return res;
}

void SignalAnalyzer::setFinderPrefs(FinderPrefs prefs)
{
    period = prefs.median_period;
    zero_level = prefs.zero_level;
    zero_sigma = prefs.zero_sigma;
}

void SignalAnalyzer::addRawData(QVector<int> *signal)
{
    int start = 0;
    if ( m_raw_signal.size() != 0 )
        start = m_raw_signal.size() - 1;

    m_raw_signal.append(*signal);

    int end = 0;
    end = m_raw_signal.size() - 1;

    QVector<double> data;
    data = median(&m_raw_signal, start, end, period);
    m_median_signal.erase(m_median_signal.begin() + start, m_median_signal.end());//Удалим те данные которые будут пересчитаны
    m_median_signal.append(data);

    clearSignal(start);
    findIng(start);

    emit Inhalations(m_ings);


//    QTime tm;
//    tm.start();

//    m_vol_calc.setIngs(m_ings);
//    auto ps = m_vol_calc.getParams();
//      qDebug()<<"tm.elapsed() msecs"<<tm.elapsed();
//    ps.debug();
    //    qDebug()<<"Stop";
}

void SignalAnalyzer::setFullPatientData(VTT_Data data)
{
    m_raw_signal.clear();
    m_median_signal.clear();
    m_clean_signal.clear();
    m_int_signal.clear();
    m_ings.clear();
    QVector<int> vec;
    for(int i=0;i<data.volume.size();i++)
        vec.push_back(data.volume.at(i)+zero_level);
    this->addRawData(&vec);
}

QVector<double> SignalAnalyzer::median(QVector<int> *signal, int start, int end, int period)
{

    if( signal->size() < start || signal->size() < end ){//начало или конец выходят за размер данных
        return QVector<double>();
    }
    QVector<double> result(end-start);

    QVector<double> temp(period);//Временный массив для сортировки данных при расчете медианы
    double med = 0;//медиана
    for(int i = start; i < end - period; i++){
        for(int k = 0; k < period; k++ ){
            temp[k] = signal->at(i+k);
        }
        //memcpy(temp.data(), signal->data(), period*sizeof(int));
        qSort(temp);
        if( temp.size()%2 == 0 ){
            med = ((double)temp.at(period/2) + (double)temp.at(period/2+1)) / 2;
        }else{
            med = temp.at(period/2+1);
        }
        result[i] = med;
    }
    return result;

}

void SignalAnalyzer::clearSignal(int start)/*Удалим нули т.е. промежутки когда человек не выдыхает*/
{
    m_clean_signal.erase(m_clean_signal.begin() + start, m_clean_signal.end());

    for(int i = start; i < m_median_signal.size(); i++){
        if( zero_level - m_median_signal.at(i)  < 0 || abs(zero_level - m_median_signal.at(i)) < zero_sigma ){//всё что в окрестности нулевого уровня или выше прировняем к 0 уровню
            m_clean_signal.push_back( zero_level );
        }else{
            m_clean_signal.push_back( m_median_signal.at(i) );
        }
        m_clean_signal[i] -= zero_level;
    }
}

void SignalAnalyzer::integrateSignal(int start)
{
    m_int_signal.erase(m_int_signal.begin() + start, m_int_signal.end());
    for(int i = start; i < m_clean_signal.size(); i++){
        int a = m_clean_signal[i];
        int b = 0;
        /*if ( m_int_signal.size() == 1 )
            m_int_signal[0];*/
        if ( m_int_signal.size() > 0 ){
            b = m_clean_signal[i-1];
            m_int_signal.push_back(area(a,b));
        }else{
            m_int_signal.push_back(0);
        }
    }

}

void SignalAnalyzer::findIng(int start)
{
    auto findStartIndex = [](QVector<double> * data, int start_pos, int zero_count ){
        for(int i = start_pos; i < data->size() - zero_count; i++){
            bool is_zero_seq = true;
            for(int k = i; k < i + zero_count; k++){
                if( data->at(k) != 0 )
                    is_zero_seq = false;
            }
            if( is_zero_seq )
                return i;
        }
        return -1;
    };

    auto findZeroSeqEnd = [](QVector<double> * data, int start_pos){
        for(int i = start_pos; i < data->size(); i++){
            if( data->at(i) != 0 )
                return i;
        }
        return -1;
    };

    auto countSumm = [](QVector<double> * data, int start_pos, int end_pos){
        double summ = 0;
        for(int i = start_pos; i < end_pos; i++){
            summ += data->at(i);
        }
        return summ;
    };


    int start_pos = start;
    int zero_seq_leh = 5;

    int zeros_start = 0;
    int zeros_end = 0;
    int ind_end = 0;

    double summ = 0;
    ing ingal;
    while(true){
        zeros_start = findStartIndex(&m_clean_signal, start_pos, zero_seq_leh); //найдем последовательность нулей
        if( zeros_start == -1)
            return;
        zeros_end = findZeroSeqEnd(&m_clean_signal, zeros_start);//найдём окончание последовательности нулей это и будет начало выдоха
        if( zeros_end ==-1 )
            return;

        ind_end = findStartIndex(&m_clean_signal, zeros_end, zero_seq_leh);// найдём окончание выдоха
        if( ind_end == -1 )
            return;

        summ = countSumm(&m_clean_signal, zeros_end, ind_end);

        ingal.start_index = zeros_end;
        ingal.end_index = ind_end;
        ingal.area = summ;
        m_ings.push_back(ingal);

        start_pos = ind_end;
    }
}

double SignalAnalyzer::area(double a, double b)
{
    return (a+b)/2;
}
