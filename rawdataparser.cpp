#include "rawdataparser.h"

RawDataParser::RawDataParser(QObject *parent) : QObject(parent)
{

}

void RawDataParser::setACQData(AdcDataMatrix data)
{
    if ( data.size() > 3 ){
        QVector<int> vol, tempin, tempout;
        for(int i = 0; i < data[0].size(); i++){
            vol.push_back(data[0][i]);
            tempin.push_back(data[1][i]);
            tempout.push_back(data[1][i]);
        }
        emit sendNewData(vol, tempin, tempout);
    }
}

