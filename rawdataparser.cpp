#include "rawdataparser.h"

RawDataParser::RawDataParser(QObject *parent) : QObject(parent)
{
    out = fopen("mydata.csv", "a");
}

RawDataParser::~RawDataParser()
{
    fclose(out);
}

void RawDataParser::setACQData(ADCData data)
{
    if ( data.size() > 3 ){
        QVector<int> vol, tempin, tempout;
        for(int i = 0; i < data[0].size(); i++){
            vol.push_back(data[0][i]);
            tempin.push_back(data[1][i]);
            tempout.push_back(data[2][i]);
            QDateTime dt;

            fprintf(out,"%s;%d;%d;%d\n", dt.currentDateTime().toString("hh:mm:ss.zzz").toStdString().c_str(),
                    data[0][i], data[1][i], data[2][i]);
        }
        emit sendNewData(vol, tempin, tempout);
    }
}

