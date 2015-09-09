#include "adcdatareader.h"

ADCDataReader::ADCDataReader(QObject *parent) : QObject(parent)
{

}

ADCDataReader::~ADCDataReader()
{

}

bool ADCDataReader::isReady()
{
    return false;
}

bool ADCDataReader::initDevice()
{
    try{
        //buffer=_bf;
        char ModuleName[10]; // название модуля
        BYTE UsbSpeed; // скорость работы шины USB
        char ModuleSerialNumber[9]; // серийный номер модуля
        char AvrVersion[5]; // версия драйвера AVR
        RTUSB3000::DSP_INFO di; // структура, содержащая информацию о версии драйвера DSP
        RTUSB3000::FLASH fi; // структура информации в ППЗУ модуля
        RTUSB3000::INPUT_PARS ip; // структура параметров работы АЦП
        double ReadRate = 1.0; // частота  ввода данных
        const static WORD MaxVirtualSoltsQuantity = 4;

        if (RtGetDllVersion() == CURRENT_VERSION_RTUSBAPI){
            qDebug() << "Dll version is correct";
        }
        else{
            qDebug() << "Dll version isn`t correct";
            qDebug("Dll version isn`t correct");
            return false;
        }

        char ss[]="usb3000";
        const PCHAR s = &ss[0];

        pModule = static_cast<IRTUSB3000 *> (RtCreateInstance(s));
        if(pModule == NULL){
            qDebug() << "Can`t create usb3000 instance";
            return false;
        }else{
            qDebug() << "Create usb3000 instance";

        }
        // }
        WORD i;
        // проверим версию используемой библиотеки Rtusbapi.dll
        DllVersion = RtGetDllVersion();
        if (DllVersion != CURRENT_VERSION_RTUSBAPI) {
            qDebug() << "Rtusbapi.dll Version --> bad";
            char String[128];
            sprintf(String, " Rtusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
                    DllVersion >> 0x10, DllVersion & 0xFFFF,
                    CURRENT_VERSION_RTUSBAPI >> 0x10, CURRENT_VERSION_RTUSBAPI & 0xFFFF);
            return false;
        } else{
            qDebug(" Rtusbapi.dll Version --> OK\n");
            qDebug() << "Rtusbapi.dll Version --> ok";
        }

        // получим указатель на интерфейс модуля USB3000
        char sss[] = "usb3000";
        char *s1 = sss;
        //PCHAR* p = reinterpret_cast<PCHAR*>(s1);
        pModule = static_cast<IRTUSB3000 *> (RtCreateInstance(s1));
        if (!pModule){
            qDebug(" Module Interface --> Bad\n");
            return false;
        }
        else
            qDebug(" Module Interface --> OK\n");

        // попробуем обнаружить модуль USB3000 в первых 127 виртуальных слотах
        for (i = 0x0; i < MaxVirtualSoltsQuantity; i++) if (pModule->OpenDevice(i)) break;
        // что-нибудь обнаружили?

        if (i == MaxVirtualSoltsQuantity){
            qDebug(" Can't find module USB3000 in first 127 virtual slots!\n");
            return false;
        }
        else
            qDebug(" OpenDevice(%u) --> OK\n", i);
        qDebug(" FUck");


        // прочитаем название обнаруженного модуля
        if (!pModule->GetModuleName(ModuleName)) {
            qDebug(" GetModuleName() --> Bad\n");
            return false;
        }
        else
            qDebug(" GetModuleName() --> OK\n");

        // проверим, что это 'USB3000'
        if (strcmp(ModuleName, "USB3000")){
            qDebug(" The module is not 'USB3000'\n");
            return false;
        }
        else
            qDebug(" The module is 'USB3000'\n");

        // узнаем текущую скорость работы шины USB20
        if (!pModule->GetUsbSpeed(&UsbSpeed)) {
            qDebug(" GetUsbSpeed() --> Bad\n");
            return false;
        } else
            qDebug(" GetUsbSpeed() --> OK\n");
        // теперь отобразим версию драйвера AVR
        qDebug(" USB Speed is %s\n", UsbSpeed ? "HIGH (480 Mbit/s)" : "FULL (12 Mbit/s)");

        // прочитаем серийный номер модуля
        if (!pModule->GetModuleSerialNumber(ModuleSerialNumber)){
            qDebug(" GetModuleSerialNumber() --> Bad\n");
            return false;
        }
        else
            qDebug(" GetModuleSerialNumber() --> OK\n");
        // теперь отобразим серийный номер модуля
        qDebug(" Module Serial Number is %s\n", ModuleSerialNumber);

        // прочитаем версию драйвера AVR
        if (!pModule->GetAvrVersion(AvrVersion)){
            qDebug(" GetAvrVersion() --> Bad\n");
            return false;
        }
        else
            qDebug(" GetAvrVersion() --> OK\n");
        // теперь отобразим версию драйвера AVR
        qDebug(" Avr Driver Version is %s\n", AvrVersion);

        // код драйвера DSP возьмём из соответствующего ресурса штатной DLL библиотеки
        if (!pModule->LOAD_DSP()){
            qDebug(" LOAD_DSP() --> Bad\n");
            return false;
        }
        else
            qDebug(" LOAD_DSP() --> OK\n");

        // проверим загрузку модуля
        if (!pModule->MODULE_TEST()){
            qDebug(" MODULE_TEST() --> Bad\n");
            return false;
        }
        else
            qDebug(" MODULE_TEST() --> OK\n");

        // получим версию загруженного драйвера DSP
        if (!pModule->GET_DSP_INFO(&di)){
            qDebug(" GET_DSP_INFO() --> Bad\n");
            return false;
        }
        else
            qDebug(" GET_DSP_INFO() --> OK\n");
        // теперь отобразим версию загруженного драйвера DSP
        qDebug(" DSP Driver version is %1u.%1u\n", di.DspMajor, di.DspMinor);

        // обязательно проинициализируем поле size структуры RTUSB3000::FLASH
        fi.size = sizeof (RTUSB3000::FLASH);
        // получим информацию из ППЗУ модуля
        if (!pModule->GET_FLASH(&fi)){
            qDebug(" GET_FLASH() --> Bad\n");
            return false;
        }else
            qDebug(" GET_FLASH() --> OK\n");

        // обязательно проинициализируем поле size структуры RTUSB3000::INPUT_PARS
        ip.size = sizeof (RTUSB3000::INPUT_PARS);
        // получим текущие параметры работы АЦП
        if (!pModule->GET_INPUT_PARS(&ip)){
            qDebug(" GET_INPUT_PARS() --> Bad\n");
            return false;
        }else
            qDebug(" GET_INPUT_PARS() --> OK\n");

        // установим желаемые параметры ввода данных
        ip.CorrectionEnabled = true; // разрешим корректировку данных
        ip.InputClockSource = RTUSB3000::INTERNAL_INPUT_CLOCK; // будем использовать внутренние тактовые испульсы для ввода данных
        //	ip.InputClockSource = RTUSB3000::EXTERNAL_INPUT_CLOCK;	// будем использовать внешние тактовые испульсы для ввода данных
        ip.SynchroType = RTUSB3000::NO_SYNCHRO; // не будем использовать никакую синхронизацию при вводе данных
        //	ip.SynchroType = RTUSB3000::TTL_START_SYNCHRO;	// будем использовать цифровую синхронизацию старта при вводе данных
        ip.ChannelsQuantity = CHANNELS_QUANTITY; // четыре активных канала
        for (i = 0x0; i < CHANNELS_QUANTITY; i++) ip.ControlTable[i] = (WORD) (i);
        ip.InputRate = ReadRate; // частота работы АЦП в кГц
        ip.InterKadrDelay = 0.0;
        // будем использовать фирменные калибровочные коэффициенты, которые храняться в ППЗУ модуля
        for (i = 0x0; i < 8; i++) {
            ip.AdcOffsetCoef[i] = fi.AdcOffsetCoef[i];
            ip.AdcScaleCoef[i] = fi.AdcScaleCoef[i];
        }
        // передадим требуемые параметры работы ввода данных в драйвер DSP модуля
        if (!pModule->SET_INPUT_PARS(&ip)){
            qDebug(" SET_INPUT_PARS() --> Bad\n");
            return false;
        }else
            qDebug(" SET_INPUT_PARS() --> OK\n");

        // отобразим параметры модуля на экране монитора
        qDebug(" \n");
        qDebug(" Module USB3000 (S/N %s) is ready ... \n", fi.SerialNumber);
        qDebug(" Adc parameters:\n");
        qDebug("   InputClockSource is %s\n", ip.InputClockSource ? "EXTERNAL" : "INTERNAL");
        qDebug("   SynchroType is %s\n", ip.SynchroType ? "TTL_START_SYNCHRO" : "NO_SYNCHRO");
        qDebug("   ChannelsQuantity = %2d\n", ip.ChannelsQuantity);
        qDebug("   AdcRate = %8.3f kHz\n", ip.InputRate);
        //qDebug("   InterKadrDelay = %2.4f ms\n", ip.InterKadrDelay);
        qDebug("   ChannelRate = %8.3f kHz\n", ip.ChannelRate);
        qDebug("\n Press any key to terminate this program...\n");
        // цикл перманентного выполнения функции ADC_KADR и
        // отображения полученных данных на экране диплея
    }catch(...){
        qDebug()<<"Exception";
    }
    //emit сигнал о том что поток создан
    return true;

}

TSUsbReadingType ADCDataReader::readingType() const
{
    return m_readingType;
}

void ADCDataReader::setReadingType(const TSUsbReadingType &readingType)
{
    m_readingType = readingType;
}

void ADCDataReader::startADC()
{
    if ( initDevice() ){
;
    }
}

void ADCDataReader::stopADC()
{

}

void ADCDataReader::acquireData()
{
    WORD i;
    // номер запроса на сбор данных
    WORD RequestNumber;
    // идентификатор массива их двух событий
    HANDLE ReadEvent[2];
    // массив OVERLAPPED структур из двух элементов
    OVERLAPPED ReadOv[2];
    DWORD BytesTransferred[2];
    //	DWORD TimeOut;

    // остановим ввод данных и одновременно прочистим соответствующий канал bulk USB
    if(!pModule->STOP_READ()){
        ThreadErrorNumber = 0x6;
        IsThreadComplete = true;
        return 0;
    }

    // создадим два события
    ReadEvent[0] = CreateEvent(NULL, FALSE , FALSE, NULL);
    memset(&ReadOv[0], 0, sizeof(OVERLAPPED)); ReadOv[0].hEvent = ReadEvent[0];
    ReadEvent[1] = CreateEvent(NULL, FALSE , FALSE, NULL);
    memset(&ReadOv[1], 0, sizeof(OVERLAPPED)); ReadOv[1].hEvent = ReadEvent[1];

    // таймаут ввода данных
    //	TimeOut = (DWORD)(DataStep/ReadRate + 1000);

    // делаем предварительный запрос на ввод данных
    RequestNumber = 0x0;
    if(!pModule->ReadData(ReadBuffer, &DataStep, &BytesTransferred[RequestNumber], &ReadOv[RequestNumber]))
        if(GetLastError() != ERROR_IO_PENDING){
            CloseHandle(ReadEvent[0]);
            CloseHandle(ReadEvent[1]);
            ThreadErrorNumber = 0x2;
            IsThreadComplete = true;
            return 0;
        }

    // теперь запускаем ввод данных
    if(pModule->START_READ()){
        // цикл сбора данных
        for(i = 0x1; i < NBlockRead; i++){

            RequestNumber ^= 0x1;
            // сделаем запрос на очередную порции данных
            if(!pModule->ReadData(ReadBuffer + i*DataStep, &DataStep, &BytesTransferred[RequestNumber], &ReadOv[RequestNumber]))
                if(GetLastError() != ERROR_IO_PENDING){
                    ThreadErrorNumber = 0x2;
                    break;
                }

            // ждём окончания операции сбора очередной порции данных
            if(!WaitingForRequestCompleted(&ReadOv[RequestNumber^0x1]))
                break;
            //			if(WaitForSingleObject(ReadEvent[!RequestNumber], TimeOut) == WAIT_TIMEOUT)
            //				            		{ ThreadErrorNumber = 0x3; break; }

            if(ThreadErrorNumber)
                break;
            else if(kbhit()) {
                ThreadErrorNumber = 0x1;
                break;
            }
            else
                Sleep(20);
            Counter++;
        }

        // ждём окончания операции сбора последней порции данных
        if(!ThreadErrorNumber)
        {
            RequestNumber ^= 0x1;
            WaitingForRequestCompleted(&ReadOv[RequestNumber^0x1]);
            //			if(WaitForSingleObject(ReadEvent[!RequestNumber], TimeOut) == WAIT_TIMEOUT) ThreadErrorNumber = 0x3;
            Counter++;
        }
    }
    else { ThreadErrorNumber = 0x5; }

    // остановим ввод данных
    if(!pModule->STOP_READ())
        ThreadErrorNumber = 0x6;
    // если надо, то прервём незавершённый асинхронный запрос
    if(!CancelIo(pModule->GetModuleHandle()))
        ThreadErrorNumber = 0x7;
    // освободим все идентификаторы событий
    for(i = 0x0; i < 0x2; i++)
        CloseHandle(ReadEvent[i]);
    // небольшая задержка
    Sleep(100);
    // установим флажок окончания потока сбора данных
    IsThreadComplete = true;
    // теперь можно воходить из потока сбора данных
    return 0;
}



