#include "ADCDataReader.h"

DWORD WINAPI runACQ(void* Param)
{
    ADCDataReader* This = (ADCDataReader*)Param;
    return This->ServiceReadThread();
}

ADCDataReader::ADCDataReader(QObject *parent):QObject(parent)
{
    ReadBuffer1 = new SHORT[NBlockRead * DataStep];
    ReadBuffer2 = new SHORT[NBlockRead * DataStep];
    hMutex = CreateMutex(NULL, FALSE, NULL);
}

ADCDataReader::~ADCDataReader()
{
    if (!is_acq_started)
        stopACQ();
    delete[] ReadBuffer1;
    delete[] ReadBuffer2;
}

bool ADCDataReader::initADC()
{
    WORD i;

    printf(" **********************************************\n");
    printf(" Data Reading Console Example for USB3000 unit \n");
    printf(" **********************************************\n\n");

    // проверим версию используемой библиотеки Rtusbapi.dll
    if ((DllVersion = RtGetDllVersion()) != CURRENT_VERSION_RTUSBAPI){
        char String[128];
        sprintf(String, " Rtusbapi.dll Version Error!!!\n   Current: %1u.%1u. Required: %1u.%1u",
                DllVersion >> 0x10, DllVersion & 0xFFFF,
                CURRENT_VERSION_RTUSBAPI >> 0x10, CURRENT_VERSION_RTUSBAPI & 0xFFFF);

        TerminateApplication(String, false);
        return false;
    }
    else
        printf(" Rtusbapi.dll Version --> OK\n");

    // получим указатель на интерфейс модуля USB3000
    char ss[] = "usb3000";
    char *s = ss;
    pModule = static_cast<IRTUSB3000 *>(RtCreateInstance(s));
    if (!pModule){
        TerminateApplication(" Module Interface --> Bad\n");
        return false;
    }
    else
        printf(" Module Interface --> OK\n");

    // попробуем обнаружить модуль USB3000 в первых 127 виртуальных слотах
    for (i = 0x0; i < MaxVirtualSoltsQuantity; i++) if (pModule->OpenDevice(i)) break;
    // что-нибудь обнаружили?
    if (i == MaxVirtualSoltsQuantity){
        TerminateApplication(" Can't find module USB3000 in first 127 virtual slots!\n");
        return false;
    }
    else
        printf(" OpenDevice(%u) --> OK\n", i);

    // попробуем получить дескриптор (handle) устройства
    ModuleHandle = pModule->GetModuleHandle();
    if (ModuleHandle == INVALID_HANDLE_VALUE){
        TerminateApplication(" GetModuleHandle() --> Bad\n");
        return false;
    }
    else printf(" GetModuleHandle() --> OK\n");

    // прочитаем название обнаруженного модуля
    if (!pModule->GetModuleName(ModuleName)) {
        TerminateApplication(" GetModuleName() --> Bad\n");
        return false;
    }
    else
        printf(" GetModuleName() --> OK\n");

    // проверим, что это 'USB3000'
    if (strcmp(ModuleName, "USB3000")) {
        TerminateApplication(" The module is not 'USB3000'\n");
        return false;
    }
    else
        printf(" The module is 'USB3000'\n");

    // узнаем текущую скорость работы шины USB20
    if (!pModule->GetUsbSpeed(&UsbSpeed)){
        TerminateApplication(" GetUsbSpeed() --> Bad\n");
        return false;
    }
    else
        printf(" GetUsbSpeed() --> OK\n");
    // теперь отобразим версию драйвера AVR
    printf(" USB Speed is %s\n", UsbSpeed ? "HIGH (480 Mbit/s)" : "FULL (12 Mbit/s)");

    // прочитаем серийный номер модуля
    if (!pModule->GetModuleSerialNumber(ModuleSerialNumber)){
        TerminateApplication(" GetModuleSerialNumber() --> Bad\n");
        return false;
    }
    else
        printf(" GetModuleSerialNumber() --> OK\n");
    // теперь отобразим серийный номер модуля
    printf(" Module Serial Number is %s\n", ModuleSerialNumber);

    // прочитаем версию драйвера AVR
    if (!pModule->GetAvrVersion(AvrVersion)) {
        TerminateApplication(" GetAvrVersion() --> Bad\n");
        return false;
    }
    else
        printf(" GetAvrVersion() --> OK\n");
    // теперь отобразим версию драйвера AVR
    printf(" Avr Driver Version is %s\n", AvrVersion);

    // код драйвера DSP возьмём из соответствующего ресурса штатной DLL библиотеки
    if (!pModule->LOAD_DSP()) {
        TerminateApplication(" LOAD_DSP() --> Bad\n");
        return false;
    }
    else
        printf(" LOAD_DSP() --> OK\n");

    // проверим загрузку модуля
    if (!pModule->MODULE_TEST()) {
        TerminateApplication(" MODULE_TEST() --> Bad\n");
        return false;
    }
    else
        printf(" MODULE_TEST() --> OK\n");

    // получим версию загруженного драйвера DSP
    if (!pModule->GET_DSP_INFO(&di)) {
        TerminateApplication(" GET_DSP_VERSION() --> Bad\n");
        return false;
    }
    else
        printf(" GET_DSP_VERSION() --> OK\n");
    // теперь отобразим версию загруженного драйвера DSP
    printf(" DSP Driver version is %1u.%1u\n", di.DspMajor, di.DspMinor);

    // обязательно проинициализируем поле size структуры RTUSB3000::FLASH
    fi.size = sizeof(RTUSB3000::FLASH);
    // получим информацию из ППЗУ модуля
    if (!pModule->GET_FLASH(&fi)) {
        TerminateApplication(" GET_MODULE_DESCR() --> Bad\n");
        return false;
    }
    else
        printf(" GET_MODULE_DESCR() --> OK\n");

    // обязательно проинициализируем поле size структуры RTUSB3000::INPUT_PARS
    ip.size = sizeof(RTUSB3000::INPUT_PARS);
    // получим текущие параметры работы АЦП
    if (!pModule->GET_INPUT_PARS(&ip)){
        TerminateApplication(" GET_INPUT_PARS() --> Bad\n");
        return false;
    }
    else
        printf(" GET_INPUT_PARS() --> OK\n");

    // установим желаемые параметры АЦП
    ip.CorrectionEnabled = true;				// разрешим корректировку вводимых данных
    ip.InputClockSource = RTUSB3000::INTERNAL_INPUT_CLOCK;	// будем использовать внутренние тактовые испульсы для ввода данных
    //	ip.InputClockSource = RTUSB3000::EXTERNAL_INPUT_CLOCK;	// будем использовать внешние тактовые испульсы для ввода данных
    ip.SynchroType = RTUSB3000::NO_SYNCHRO;	// не будем использовать никакую синхронизацию при вводе данных
    //	ip.SynchroType = RTUSB3000::TTL_START_SYNCHRO;	// будем использовать цифровую синхронизацию старта при вводе данных
    ip.ChannelsQuantity = ChannaleQuantity;					// четыре активных канала
    for (i = 0x0; i < ip.ChannelsQuantity; i++)
        ip.ControlTable[i] = (WORD)(i);
    ip.InputRate = ReadRate;					// частота работы АЦП в кГц
    ip.InterKadrDelay = 0.0;					// межкадровая задержка - пока всегда устанавливать в 0.0
    ip.InputFifoBaseAddress = 0x0;  			// базовый адрес FIFO буфера АЦП
    ip.InputFifoLength = 0x3000;	 			// длина FIFO буфера АЦП
    ip.InputFifoLength = 0x400;
    // будем использовать фирменные калибровочные коэффициенты, которые храняться в ППЗУ модуля
    for (i = 0x0; i < 8; i++) {
        ip.AdcOffsetCoef[i] = fi.AdcOffsetCoef[i];
        ip.AdcScaleCoef[i] = fi.AdcScaleCoef[i];
    }
    // передадим требуемые параметры работы АЦП в модуль
    if (!pModule->SET_INPUT_PARS(&ip)){
        TerminateApplication(" SET_INPUT_PARS() --> Bad\n");
        return false;
    }
    else
        printf(" SET_INPUT_PARS() --> OK\n");

    // отобразим на экране дисплея параметры работы модуля USB3000
    printf(" \n");
    printf(" Module USB3000 (S/N %s) is ready ... \n", ModuleSerialNumber);
    printf(" Adc parameters:\n");
    printf("   InputClockSource is %s\n", ip.InputClockSource ? "EXTERNAL" : "INTERNAL");
    printf("   SynchroType is %s\n", ip.SynchroType ? "TTL_START_SYNCHRO" : "NO_SYNCHRO");
    printf("   ChannelsQuantity = %2d\n", ip.ChannelsQuantity);
    printf("   AdcRate = %8.3f kHz\n", ip.InputRate);
    printf("   InterKadrDelay = %2.4f ms\n", ip.InterKadrDelay);
    printf("   ChannelRate = %8.3f kHz\n", ip.ChannelRate);
    return true;
}

DWORD WINAPI ADCDataReader::ServiceReadThread()
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
    if (!pModule->STOP_READ()) {
        ThreadErrorNumber = 0x6;
        IsThreadComplete = true;
        return 0;
    }

    // создадим два события
    ReadEvent[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
    memset(&ReadOv[0], 0, sizeof(OVERLAPPED)); ReadOv[0].hEvent = ReadEvent[0];
    ReadEvent[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
    memset(&ReadOv[1], 0, sizeof(OVERLAPPED)); ReadOv[1].hEvent = ReadEvent[1];

    // таймаут ввода данных
    //	TimeOut = (DWORD)(DataStep/ReadRate + 1000);

    // делаем предварительный запрос на ввод данных
    RequestNumber = 0x0;
    if (!pModule->ReadData(ReadBuffer, &DataStep, &BytesTransferred[RequestNumber], &ReadOv[RequestNumber]))
        if (GetLastError() != ERROR_IO_PENDING) {
            CloseHandle(ReadEvent[0]);
            CloseHandle(ReadEvent[1]);
            ThreadErrorNumber = 0x2;
            IsThreadComplete = true;
            return 0;
        }

    // теперь запускаем ввод данных
    if (pModule->START_READ())
    {
        // цикл сбора данных
        i = 0x1;
        m_samples_count = 0;
        while (is_acq_started || m_samples_count >= m_samples_number){
            //for (i = 0x1; i < NBlockRead; i++)
            //{

            RequestNumber ^= 0x1;
            // сделаем запрос на очередную порции данных
            if (!pModule->ReadData(ReadBuffer + i*DataStep, &DataStep, &BytesTransferred[RequestNumber], &ReadOv[RequestNumber]))
                if (GetLastError() != ERROR_IO_PENDING) {
                    ThreadErrorNumber = 0x2;
                    break;
                }

            // ждём окончания операции сбора очередной порции данных
            if (!WaitingForRequestCompleted(&ReadOv[RequestNumber ^ 0x1]))
                break;
            printf("ACQ is ok\n");
            //if (i == NBlockRead - 1){
            if (i == 2){
                WaitForSingleObject(hMutex, INFINITE);
                for (int k = 0; k < DataStep*i; k += MaxVirtualSoltsQuantity){
                    data[0].push_back(ReadBuffer[k]);
                    data[1].push_back(ReadBuffer[k+1]);
                    data[2].push_back(ReadBuffer[k+2]);
                    data[3].push_back(ReadBuffer[k+3]);
                }
                if (ReadBuffer == ReadBuffer1){
                    ReadBuffer = ReadBuffer2;
                    i = 0x1;
                }
                else{
                    ReadBuffer = ReadBuffer1;
                    i = 0x1;
                }
                m_samples_count = data[0].size();
                ReleaseMutex(hMutex);

            }

            //			if(WaitForSingleObject(ReadEvent[!RequestNumber], TimeOut) == WAIT_TIMEOUT)
            //				            		{ ThreadErrorNumber = 0x3; break; }

            /*if (ThreadErrorNumber)
                break;
                else if (kbhit()) {
                ThreadErrorNumber = 0x1;
                break;
                }
                else Sleep(20);*/
            Counter++;
            i++;
            //}
        }

        // ждём окончания операции сбора последней порции данных
        if (!ThreadErrorNumber)
        {
            RequestNumber ^= 0x1;
            WaitingForRequestCompleted(&ReadOv[RequestNumber ^ 0x1]);
            //			if(WaitForSingleObject(ReadEvent[!RequestNumber], TimeOut) == WAIT_TIMEOUT) ThreadErrorNumber = 0x3;
            Counter++;
        }
    }
    else {
        ThreadErrorNumber = 0x5;
    }

    // остановим ввод данных
    if (!pModule->STOP_READ())
        ThreadErrorNumber = 0x6;
    // если надо, то прервём незавершённый асинхронный запрос
    if (!CancelIo(pModule->GetModuleHandle()))
        ThreadErrorNumber = 0x7;
    // освободим все идентификаторы событий
    for (i = 0x0; i < 0x2; i++)
        CloseHandle(ReadEvent[i]);
    // небольшая задержка
    Sleep(100);
    // установим флажок окончания потока сбора данных
    IsThreadComplete = true;
    // теперь можно воходить из потока сбора данных
    is_acq_started = false;
    return 0;
}


bool ADCDataReader::WaitingForRequestCompleted(OVERLAPPED *ReadOv)
{
    DWORD ReadBytesTransferred;

    while (true)
    {
        if (GetOverlappedResult(ModuleHandle, ReadOv, &ReadBytesTransferred, FALSE))
            break;
        else if (GetLastError() != ERROR_IO_INCOMPLETE) {
            ThreadErrorNumber = 0x3;
            return false;
        }
        /*else
            if (kbhit()){
            ThreadErrorNumber = 0x1;
            return false;
            }*/
        else
            Sleep(20);
    }
    return true;
}
int ADCDataReader::getSamples_number() const
{
    return m_samples_number;
}

void ADCDataReader::setSamples_number(int samples_number)
{
    m_samples_number = samples_number;
}


//------------------------------------------------------------------------
// Отобразим сообщение с ошибкой
//------------------------------------------------------------------------
void ADCDataReader::ShowThreadErrorMessage(void)
{
    switch (ThreadErrorNumber)
    {
    case 0x0:
        break;

    case 0x1:
        // если программа была злобно прервана, предъявим ноту протеста
        printf("\n READ Thread: The program was terminated! :(((\n");
        break;

    case 0x2:
        printf("\n READ Thread: ReadData() --> Bad :(((\n");
        break;

    case 0x3:
        printf("\n READ Thread: Read Request --> Bad :(((\n");
        //			printf("\n READ Thread: Timeout is occured :(((\n");
        break;

    case 0x4:
        printf("\n READ Thread: Buffer Data Error! :(((\n");
        break;

    case 0x5:
        printf("\n READ Thread: START_READ() --> Bad :(((\n");
        break;

    case 0x6:
        printf("\n READ Thread: STOP_READ() --> Bad! :(((\n");
        break;

    case 0x7:
        printf("\n READ Thread: Can't complete input and output (I/O) operations! :(((");
        break;

    default:
        printf("\n READ Thread: Unknown error! :(((\n");
        break;
    }

    return;
}

//------------------------------------------------------------------------
// вывод сообщения и, если нужно, аварийный выход из программы
//------------------------------------------------------------------------
void ADCDataReader::TerminateApplication(QString ErrorString, bool TerminationFlag)
{
    // подчищаем интерфейс модуля
    if (pModule)
    {
        // освободим интерфейс модуля
        if (!pModule->ReleaseInstance())
            printf(" ReleaseInstance() --> Bad\n");
        else
            printf(" ReleaseInstance() --> OK\n");
        // обнулим указатель на интерфейс модуля
        pModule = NULL;
    }

    // освобождаем занятые ресурсы
    /*if (ReadBuffer) {
        delete[] ReadBuffer;
        ReadBuffer = NULL;
        }*/
    // освободим идентификатор потока сбора данных
    if (hReadThread) {
        CloseHandle(hReadThread);
        hReadThread = NULL;
    }
    // освободим идентификатор файла данных
    /*if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }*/

    // выводим текст сообщения
    if ( ErrorString.length() > 0 )
        printf("%s",ErrorString.toStdString().c_str());
    /*if (ErrorString)
        printf(ErrorString);*/

    // если нужно - аварийно завершаем программу
    /*if (TerminationFlag)
        exit(1);*/
    else return;
}

void ADCDataReader::startACQ()
{
    initADC();

    // сбросим флаг ошибок потока ввода данных
    ThreadErrorNumber = 0x0;

    // попробуем выделить память под буфер для вводимых с модуля данных
    //ReadBuffer = new SHORT[NBlockRead * DataStep];

    ReadBuffer = ReadBuffer1;

    if (!ReadBuffer)
        TerminateApplication(" Cannot allocate memory for ReadBuffer\n");

    // Создаем и запускаем поток сбора ввода данных из модуля
    //	hReadThread = CreateThread(0, 0x2000, &ADCDataReader::ServiceReadThread, 0, 0, &ReadTid);
    is_acq_started = true;
    hReadThread = CreateThread(0, 0x2000, runACQ, (void *)this, 0, &ReadTid);
    if (!hReadThread)
        TerminateApplication("Can't start input data thread!");

    // ждем завершения работы нужного потока
    printf("\n");

}

void ADCDataReader::stopACQ()
{
    is_acq_started = false;

    /*while (!IsThreadComplete){
        if (OldCounter != Counter){
            printf(" Counter %3u from %3u\r", Counter, NBlockRead);
            OldCounter = Counter;
        }
        else
            Sleep(20);
    }*/

    // ждём окончания работы потока ввода данных
    WaitForSingleObject(hReadThread, INFINITE);
    // две пустые строчки
    printf("\n\n");
    /*FILE *out;
    out = fopen("out.txt", "w");
    for (int i = 0; i < NBlockRead * DataStep; i++){
        fprintf(out, "%d ", ReadBuffer[i]);
        if (i % 4 == 0 && i != 0)
            fprintf(out, "\n");
    }*/
    // если не было ошибок ввода данных - запишем полученные данные в файл
    /*if (!ThreadErrorNumber)
    {
    // откроем файл для записи полученных данных
    hFile = CreateFile("Test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) TerminateApplication(" Open file 'Test.dat' --> Failed!!!\n");
    else printf(" CreateFile(Test.dat) --> Ok\n");

    // теперь запишем в файл полученные данные
    DWORD FileBytesWritten = 0x0;
    if (!WriteFile(hFile,							// handle to file to write to
    ReadBuffer,						// pointer to data to write to file
    2 * NBlockRead*DataStep,		// number of bytes to write
    &FileBytesWritten,			// pointer to number of bytes written
    NULL			  					// pointer to structure needed for overlapped I/O
    ))  TerminateApplication(" WriteFile(Test.dat) --> Failed!!!");
    else printf(" WriteFile(Test.dat) --> Ok\n");
    }*/

    // если была ошибка - сообщим об этом
    if (ThreadErrorNumber) {
        TerminateApplication(NULL, false);
        ShowThreadErrorMessage();
    }
    else {
        printf("\n");
        TerminateApplication("\n The program was completed successfully!!!\n", false);
    }
}

bool ADCDataReader::isReady()
{
    if( initADC()){
        TerminateApplication("On test init");
        return true;
    }else
        return false;

}

std::vector<std::vector<short>> ADCDataReader::getACQData()
{
    std::vector<std::vector<short>> tmp_data;
    WaitForSingleObject(hMutex, INFINITE);
    tmp_data = data;
    for (int i = 0; i < data.size(); i++)
        data[i].clear();
    ReleaseMutex(hMutex);
    return tmp_data;
}