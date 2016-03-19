//#pragma once
#ifndef ADCDATAREADER_H
#define ADCDATAREADER_H
#include <QObject>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <vector>
#include "Rtusbapi.h"
#include "src/datatypes.h"
#include <QDebug>
#include <QThread>


class ADCDataReader: public QObject
{
    Q_OBJECT
public:
    explicit ADCDataReader(QObject *parent = 0);
    ~ADCDataReader();
    AdcDataMatrix getACQData();
    int getSamples_number() const;
    void setSamples_number(int samples_number);
    bool isReady();
    QVector<int> getSamplesSinc(int channel, int samplesNumber);
public slots:
    void startADC(int samples_number);
    void stopADC();
    void processADC();
signals:
    void finished();


    void sendACQData(AdcDataMatrix);
    void done();
    void changeProgress(int);
private:
    bool initADC();
    void ShowThreadErrorMessage(void);
    void TerminateApplication(QString ErrorString, bool TerminationFlag = false);
    bool WaitingForRequestCompleted(OVERLAPPED *ReadOv);
    bool is_acq_started = false;

    // идентификатор потока ввода
    HANDLE 	hReadThread = 0;
    DWORD 	ReadTid;

    // текущая версия библиотеки Rtusbapi.dll
    DWORD DllVersion;
    // указатель на интерфейс модуля
    IRTUSB3000 *pModule = NULL;
    // хэндл модуля
    HANDLE ModuleHandle;
    // название модуля
    char ModuleName[10];
    // скорость работы шины USB
    BYTE UsbSpeed;
    // серийный номер модуля
    char ModuleSerialNumber[9];
    // версия драйвера AVR
    char AvrVersion[5];
    // структура, содержащая информацию о версии драйвера DSP
    RTUSB3000::DSP_INFO di;
    // структура информации в ППЗУ модуля
    RTUSB3000::FLASH fi;
    // структура параметров работы АЦП
    RTUSB3000::INPUT_PARS ip;

    // максимально возможное кол-во опрашиваемых виртуальных слотов
    //const WORD MaxVirtualSoltsQuantity = 127;
    const WORD MaxVirtualSoltsQuantity = 4;
    // частота  ввода данных
    //const double ReadRate = 0.6;/*150Hz per channel*/
    //const double ReadRate = 2.048;
    const double ReadRate = 0.512;
    //max возможное кол-во передаваемых отсчетов (кратное 32) для ф. ReadData и WriteData()
    //DWORD DataStep = 1024 * 1024;
    //DWORD DataStep = 8192;
    DWORD DataStep = 512;
    //Число каналов
    const DWORD ChannaleQuantity = 0x4;
    // столько блоков по DataStep отсчётов нужно собрать в файл
    const WORD NBlockRead = 2;
    // указатель на буфер для вводимых данных
    SHORT	ReadBuffer[1024];
    SHORT	*ReadBuffer1, *ReadBuffer2;

    // номер ошибки при выполнении потока сбора данных
    WORD ThreadErrorNumber;
    // флажок завершения потоков ввода данных
    bool IsThreadComplete = false;

    char mod_name[8];


    HANDLE hMutex ;
    int m_samples_number = 1800000;
    int m_samples_count = 0;
    AdcDataMatrix data = AdcDataMatrix(MaxVirtualSoltsQuantity);
    QThread *m_thread = nullptr;
    char ss1[8] ;
};
#endif
