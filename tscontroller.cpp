#include "tscontroller.h"
#include "ui_tsview.h"
#include "ui_tsprogressdialog.h"
#include "ui_tsvolsignalwidget.h"
#include <QtSql/QSqlRecord>
#include <QDebug>
#include "tools/tsvalidationtools.h"
#include <QSqlDatabase>
#include <QTranslator>
#include <QApplication>
#include <QTime>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QDialog>
#include <tsanalitics.h>
#include <QSettings>
#include <ui_tsprintview.h>
#include <QPrintEngine>
#include <tstempanalitic.h>
#include <QMessageBox>
using namespace std;


TSController::TSController(QWidget *parent):QMainWindow(parent)//,ui(new Ui::TSView)
{
    qDebug()<<"TSController::TSController";
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("CP-1251"));
    QSettings * settings = new QSettings("settings.ini",QSettings::IniFormat);

    m_adc_reader = new ADCDataReader();

    ui.setupUi(this);

    m_plotter_widjet = new PlotterWidjet();
    ui.PlotterAllwidget->layout()->addWidget(qobject_cast<QWidget*>(m_plotter_widjet));

    m_calib_plotter = new CalibrationPkotterWidjet();
    ui.VolumePlotterWidget->layout()->addWidget(qobject_cast<QWidget*>(m_calib_plotter));

    m_thread = new QThread();


    m_volumes_calc = new VolumeValuesCalc();

    m_signal_analyzer = new SignalAnalyzer();

    m_volumes_calc->moveToThread(m_thread);
    m_signal_analyzer->moveToThread(m_thread);

    connect(m_signal_analyzer, SIGNAL(Inhalations(QVector<ing>)), m_volumes_calc, SLOT(setIngs(QVector<ing>)));


    connect(m_volumes_calc, SIGNAL(signalParameters(parameters)), this, SLOT(setSignalParameters(parameters)));

    connect(&m_exam_cntrl, SIGNAL(selectedExam(VTT_Data)), m_signal_analyzer, SLOT(setFullPatientData(VTT_Data)));


    connect(&m_raw_data_parser, SIGNAL(sendNewData(QVector<int>,QVector<int>,QVector<int>)), m_signal_analyzer, SLOT(addMultiplyRawData(QVector<int>,QVector<int>,QVector<int>)));


    m_thread->start();

    curveBuffer.setReference(settings);

    recordingStarted = false;

    ui.managmentBox->setVisible(false);
    ui.vertLabel->setVisible(false);
    ui.horLabel->setVisible(false);
    ui.tempInScaleSlider->setVisible(false);
    ui.tempOutScaleSlider->setVisible(false);
    ui.volumeScaleSlider->setVisible(false);
    ui.breakExamButton->setVisible(false);

    if( !m_exam_cntrl.init() ){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Ошибка"));
        QString error = QString("Произошла ошибка.\nОбратитесь к разработчикам.\nКод: 00001.\nПрограмма будет завершена. ")+m_exam_cntrl.getErrorPatientsConnection().text().toStdString().c_str();

        msgBox.setText(error);
        msgBox.exec();
        QApplication::exit(0);
    }


    connect(ui.saveProfileButton,SIGNAL(clicked()),this,SLOT(savePatientProfile()));
    connect(ui.ignoreCalibrateButton,SIGNAL(clicked()),this,SLOT(rejectColibration()));
    connect(ui.volumeCalibrateButton,SIGNAL(clicked()),this,SLOT(calibrateVolume()));
    connect(ui.startExam,SIGNAL(clicked()),this,SLOT(startExam()));

    connect(&curveBuffer,SIGNAL(updateAverageData(int,int,int,int)),this,SLOT(showAverageData(int,int,int,int)));
    connect(ui.fName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui.sName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui.fdName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui.date,SIGNAL(editingFinished()),this,SLOT(completePatientId()));

    connect(ui.newExamButton,SIGNAL(clicked()),this,SLOT(createNewExam()));
    connect(ui.stopExam,SIGNAL(clicked()),this,SLOT(stopExam()));



    connect(&m_exam_cntrl, SIGNAL(selectedExam(VTT_Data)), this, SLOT(selectedExamination(VTT_Data)));

    connect(&m_exam_cntrl, SIGNAL(selectedExam(VTT_Data)), m_plotter_widjet, SLOT(setFullPatientGrapgicsData(VTT_Data)));

    connect(&m_exam_cntrl, SIGNAL(reset()), &curveBuffer, SLOT(clean()));

    connect(ui.breakExamButton,SIGNAL(clicked()),this,SLOT(breakExam()));
    //connect(ui.resultsButton,SIGNAL(clicked()),this,SLOT(processDataParams()));

    connect(ui.printButton,SIGNAL(clicked()),this,SLOT(printReport()));

    ui.resultsButton->setEnabled(true);

    ui.examsTableView->setEditTriggers(QTableView::NoEditTriggers);;
    ui.mainBox->setCurrentIndex(0);
    m_exam_cntrl.setm_ui(&ui);
}

TSController::~TSController()
{
    delete m_adc_reader;
    qDebug()<<"TSController::~TSController";
}

void TSController::savePatientProfile()
{
    m_exam_cntrl.savePatientProfile();
}

void TSController::calibrateVolume(){
    QSettings settings("settings.ini", QSettings::IniFormat);
    QDialog d(this);
    Ui::TSProgressDialog dui;
    dui.setupUi(&d);

    d.setWindowTitle(tr("Предупреждение"));
    dui.information->setText(tr("Идет подготовка..."));
    dui.acceptButton->setVisible(false);

    auto data = m_adc_reader->getSamplesSinc(0,100);
    int sum = 0;
    double avg = 0;
    for(int i = 0; i < data.size(); i++){
        sum+=data[i];
    }
    if (data.size() != 0)
        avg = sum / data.size();
    curveBuffer.setVolumeColibration(avg, true);

    settings.setValue("volZero",curveBuffer.volumeColibration());
    //dui.information->setText(tr("Подготовка завершена.\nНажмите ОК и качайте шприцем."));
    dui.information->setText(tr("Подготовка завершена.\nНажмите Пропустить."));
    dui.progressBar->setVisible(false);
    dui.acceptButton->setVisible(true);


    //вот сдесть нужно подождать пока всё измериться

    d.exec();

    dui.progressBar->setVisible(false);
    dui.acceptButton->setVisible(true);

    delete m_adc_reader;
    /* m_adc_reader = new ADCDataReader();

    connect(m_adc_reader, SIGNAL(sendACQData(AdcDataMatrix)), &m_raw_data_parser, SLOT(setACQData(AdcDataMatrix)));
    connect(&m_raw_data_parser, SIGNAL(sendNewData(IntegerVector, IntegerVector, IntegerVector)), &curveBuffer, SLOT(appendData(IntegerVector, IntegerVector, IntegerVector)));

    connect(m_adc_reader, SIGNAL(finished()), &d, SLOT(accept()));


    connect(&m_plotter, SIGNAL(changeprogress(int)), dui.progressBar, SLOT(setValue(int)));

    m_adc_reader->setSamples_number(1536);
    m_plotter.startCPlottingTimer(100);
    m_adc_reader->startADC(0);

    connect(m_adc_reader, SIGNAL(finished()), this, SLOT(stopCalibration()));
    dui.progressBar->setVisible(true);
    dui.acceptButton->setVisible(true);
    d.exec();*/
}

void TSController::calibrationFinished()
{

    //auto vol = curveBuffer.volumeVector();
    /*cPlotingTimer.stop();
    QSettings settings("settings.ini",QSettings::IniFormat);
    QDialog d(parentWindow);
    Ui::TSProgressDialog dui;
    dui.setupUi(&d);
    d.setWindowTitle(tr("Предупреждение"));
    QVector<int> vol = curveBuffer->volumeVector();
    tsanalitics ta;
    qDebug()<<"curvebuff end "<<curveBuffer->end();
    for(int i=0;i<curveBuffer->end();i++){
        ta.append(vol[i]);
    }
    ta.approximate();
    qDebug()<<"get min "<<ta.getMin()<<" ; get max "<<ta.getMax();
    settings.setValue("volOutLtr",ta.getMin());
    settings.setValue("volInLtr",ta.getMax());
    qDebug()<<"setVolumeConverts plotCalibration "<<ta.getMin()<<" "<<ta.getMax();
    curveBuffer->setVolumeConverts(ta.getMax(),ta.getMin());

    qDebug()<<"reading is finished";

    emit stopACQU();
    //m_adc_reader->stopACQ();

    //curveBuffer->clean();
    settings.sync();
    dui.progressBar->setVisible(false);
    dui.acceptButton->setVisible(true);
    dui.information->setText(tr("Калибровка успешно завершена.\nНажмите ОК для продолжения."));
    if(d.exec()==1){
        ui->mainBox->setCurrentIndex(5);
        ui->managmentSpaser->setGeometry(QRect(0,0,350,2));
        ui->managmentBox->setVisible(true);
        ui->managmentBox->setEnabled(true);
        ui->startExam->setEnabled(true);
        ui->stopExam->setEnabled(true);
        curveBuffer->setEnd(0);
        initPaintDevices();
        plotNow();
    }
    curveBuffer->setEnd(0);
    maxcVol = 0;
    ui->volumeInfoLabel->setVisible(false);
    ui->tinInfoLabel->setVisible(false);
    ui->toutInfolabel->setVisible(false);
*/

}

void TSController::stopCalibration()
{
    m_adc_reader->stopADC();
    delete m_adc_reader;
    QSettings settings("settings.ini", QSettings::IniFormat);
    QDialog d(this);
    Ui::TSProgressDialog dui;
    dui.setupUi(&d);
    d.setWindowTitle(tr("Предупреждение"));

    QVector<int> vol = curveBuffer.volumeVector();
    tsanalitics ta;
    qDebug()<<"curvebuff end "<<curveBuffer.end();
    for(int i=0; i<vol.size(); i++){
        ta.append(vol[i]);
    }
    ta.approximate();
    qDebug()<<"get min "<<ta.getMin()<<" ; get max "<<ta.getMax();
    settings.setValue("volOutLtr",ta.getMin());
    settings.setValue("volInLtr",ta.getMax());
    qDebug()<<"setVolumeConverts plotCalibration "<<ta.getMin()<<" "<<ta.getMax();
    curveBuffer.setVolumeConverts(ta.getMax(),ta.getMin());

    settings.sync();
    dui.progressBar->setVisible(false);
    dui.acceptButton->setVisible(true);
    dui.information->setText(tr("Калибровка успешно завершена.\nНажмите ОК для продолжения."));
    if(d.exec()==1){
        ui.mainBox->setCurrentIndex(5);
        ui.managmentSpaser->setGeometry(QRect(0, 0, 350, 2));
        ui.managmentBox->setVisible(true);
        ui.managmentBox->setEnabled(true);
        ui.startExam->setEnabled(true);
        ui.stopExam->setEnabled(true);
        curveBuffer.setEnd(0);
    }
    curveBuffer.setEnd(0);
    ui.volumeInfoLabel->setVisible(false);
    ui.tinInfoLabel->setVisible(false);
    ui.toutInfolabel->setVisible(false);

    qDebug()<<"Calib fin";
}

void TSController::setSignalParameters(parameters params)
{
    //    qDebug()<<"TSController::processDataParams";
    qDebug()<<"this is result button !";
    QTableWidget *qtw = ui.resultsTable;
    qtw->setColumnCount(2);
    qtw->setRowCount(12);
    qtw->verticalHeader()->setVisible(false);
    qtw->setHorizontalHeaderLabels(QString(tr("Параметр;Значение")).split(";"));
    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    float AvgExpirationSpeed=0, MaxExpirationSpeed=0, AvgExpirationTime=0, AvgInspirationTime=0,
            AvgRoundTime=0, AvgTempIn=0, AvgTempOut=0, AvgTempInMinusAvgTempOut=0,  BreathingVolume=0, MVL=0, MinuteVentilation=0;
    float InspirationFrequency = 0;

    //    QVector<int> volume = curveBuffer.volumeVector();
    //    QVector<int> temp_in = curveBuffer.tempInVector();
    //    QVector<int> temp_out = curveBuffer.tempOutVector();

    //    VolumeSolver* vs = new VolumeSolver(volume,temp_in,temp_out);

    AvgExpirationSpeed = params.av_speed;//vs->getAverageExpirationSpeed();
    qtw->setItem(1,0,getQTableWidgetItem(tr("Средняя скорость выдоха(л/с)")));
    qtw->setItem(1,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)AvgExpirationSpeed)))));

    MaxExpirationSpeed = params.max_speed;//vs->getMaxExpirationSpeed();
    qtw->setItem(2,0,getQTableWidgetItem(tr("Максимальная скорость выдоха(л/с)")));
    qtw->setItem(2,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)MaxExpirationSpeed)))));

    AvgExpirationTime = params.av_out_time;//vs->getAverageExpirationTime();
    qtw->setItem(3,0,getQTableWidgetItem(tr("Среднее время выдоха(с)")));
    qtw->setItem(3,1,getQTableWidgetItem((QString::number((float)AvgExpirationTime))));

    AvgInspirationTime = params.av_in_time;//vs->getAverageInspirationTime();
    qtw->setItem(4,0,getQTableWidgetItem(tr("Среднее время вдоха(с)")));
    qtw->setItem(4,1,getQTableWidgetItem((QString::number((float)AvgInspirationTime))));

    AvgRoundTime = params.av_cycle_time;//vs->getAverageCycleTime();
    qtw->setItem(5,0,getQTableWidgetItem(tr("Средняя время цикла(с)")));
    qtw->setItem(5,1,getQTableWidgetItem((QString::number((float)AvgRoundTime))));

    InspirationFrequency = params.freq;//vs->getInspirationFrequancyInOneMinute();
    qtw->setItem(6,0,getQTableWidgetItem(tr("Частота дыхания(ед/мин)")));
    qtw->setItem(6,1,getQTableWidgetItem((QString::number(InspirationFrequency))));

    MinuteVentilation = params.av_speed;//vs->getMinuteVentilation();
    qtw->setItem(8,0,getQTableWidgetItem(tr("Минутная вентиляция легких(л)")));
    qtw->setItem(8,1,getQTableWidgetItem(QString::number(curveBuffer.volToLtr(MinuteVentilation))));

    BreathingVolume = params.one_volume;//vs->getAverageInspiratonVentilation();
    qtw->setItem(7,0,getQTableWidgetItem(tr("Дыхательный объем(л)")));
    qtw->setItem(7,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)BreathingVolume)))));

    MVL = params.all_volume;//vs->getTotalVentilation();
    qtw->setItem(9,0,getQTableWidgetItem(tr("Суммарная вентиляция легких(л)")));
    qtw->setItem(9,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)MVL)))));

    //ga->clear();

    /*AvgTempIn = vs->getAverageInspirationTempetature();
    qtw->setItem(10,0,getQTableWidgetItem(tr("Средняя температура вдоха( 'C)")));
    qtw->setItem(10,1,getQTableWidgetItem(QString::number(curveBuffer.tempInToDeg(AvgTempIn))));
    //gai->clear();

    AvgTempOut = vs->getAverageExpirationTempetature();
    qtw->setItem(11,0,getQTableWidgetItem(tr("Средняя температура выдоха( 'C)")));
    qtw->setItem(11,1,getQTableWidgetItem(QString::number(curveBuffer.tempOutToDeg(AvgTempOut))));

    AvgTempInMinusAvgTempOut = AvgTempOut-AvgTempIn;
    qtw->setItem(12,0,getQTableWidgetItem(tr("Средняя Твдоха-Средняя Твыдоха( 'C)")));
    qtw->setItem(12,1,getQTableWidgetItem(curveBuffer.tempOutToDeg(AvgTempOut)-curveBuffer.tempInToDeg(AvgTempIn)));
*/
    qtw->removeRow(0);
    qtw->show();
    //delete vs;
}

void TSController::rejectColibration()
{
    //qDebug()<<"TSController::rejectColibration";
    QSettings settings("settings.ini",QSettings::IniFormat);
    curveBuffer.setVolumeColibration(settings.value("volZero").toInt(),false);
    qDebug()<<"volColibr: "<<curveBuffer.volumeColibration();
    qDebug()<<"setVolumeConverts rejectColibration "<<settings.value("volInLtr").toInt()<<" "<<settings.value("volOutLtr").toInt();
    curveBuffer.setVolumeConverts(settings.value("volInLtr").toInt(), settings.value("volOutLtr").toInt());
    ui.mainBox->setCurrentIndex(5);
    curveBuffer.setEnd(0);
    ui.startExam->setEnabled(true);
    ui.stopExam->setEnabled(true);

    ui.managmentSpaser->setGeometry(QRect(0,0,350,2));
    ui.managmentBox->setVisible(true);
    ui.managmentBox->setEnabled(true);
}

void TSController::startExam()
{
    ui.volumeInfoLabel->setText(tr("ДО=0")
                                +tr(" Л\nЧД=0"));
    ui.tinInfoLabel->setText("Tin=0 'C");
    ui.toutInfolabel->setText("Tout=0 'C");
    ui.volumeInfoLabel->setVisible(true);
    ui.tinInfoLabel->setVisible(true);
    ui.toutInfolabel->setVisible(true);
    curveBuffer.clean();

    m_adc_reader = new ADCDataReader();

    connect(m_adc_reader, SIGNAL(newData(ADCData)), &m_raw_data_parser, SLOT(setACQData(ADCData)));
    //connect(&m_raw_data_parser, SIGNAL(sendNewData(IntegerVector, IntegerVector, IntegerVector)), &curveBuffer, SLOT(appendData(IntegerVector, IntegerVector, IntegerVector)));
    //connect(m_adc_reader, SIGNAL(newData( ADCData )), this, SLOT(ExamADCNewData( ADCData )) );

    m_adc_reader->startADC(-1);//1800000;//FIXME похоже попытка мерить время АЦП - это не верно

    //myTimer.start();
    //m_plotter.setRecordingStarted(true);
    recordingStarted = true;

    /*mvlDialog = new QDialog(this);
       volWidget = new Ui::TSVolSignalWidget();
       volWidget->setupUi(mvlDialog);
       volWidget->MVL->setText("0%");
       mvlDialog->setModal(false);
       mvlDialog->show();
       ui.startExam->setEnabled(false);*/
}

void TSController::stopExam()
{
    //qDebug()<<"TSController::stopExam";
    if(recordingStarted)
    {
        m_adc_reader->stopADC();

        qDebug()<<"Stop exam";
        m_exam_cntrl.writeExamToDB(curveBuffer.volumeVector(), curveBuffer.tempInVector(), curveBuffer.tempOutVector(),
                                   curveBuffer.volumeConverts().at(1), curveBuffer.volumeConverts().at(0));
    }

    mvlDialog->close();
    recordingStarted = false;
}

void TSController::showAverageData(int avgTempIn, int avgTempOut, int avgDO, int avgCHD){
    ui.volumeInfoLabel->setText(tr("ДО=")+QString::number(curveBuffer.volToLtr(avgDO),'g',2)+tr(" Л\nЧД=")+QString::number(avgCHD));
    ui.tinInfoLabel->setText("Tin="+QString::number(curveBuffer.tempInToDeg(avgTempIn),'g',2)+" 'C");
    ui.toutInfolabel->setText("Tout="+QString::number(curveBuffer.tempInToDeg(avgTempOut),'g',2)+" 'C");
    int mvl = (curveBuffer.volToLtr(avgDO)*avgCHD)*100 / m_exam_cntrl.getMVL();
    if(recordingStarted&&curveBuffer.end()>10)
        volWidget->MVL->setText(QString::number(mvl)+"%");
}

void TSController::completePatientId()
{
    // qDebug()<<"TSController::completePatientId";
    QString id;
    if(ui.sName->text().length()>0)
        id.append(ui.sName->text().toUpper().at(0));
    if(ui.fName->text().length()>0)
        id.append(ui.fName->text().toUpper().at(0));
    if(ui.fdName->text().length()>0)
        id.append(ui.fdName->text().toUpper().at(0));
    id.append(ui.date->text().replace("-",""));
    ui.idEdit->setText(id);
}

void TSController::createNewExam(){
    if ( m_adc_reader->isReady() == true ){
        qDebug()<<"TSController::createNewExam";
        ui.mainBox->setCurrentIndex(4);
        curveBuffer.clean();
        curveBuffer.setEnd(0);
        calibrateVolume();
    }else{
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Внимание"));
        msgBox.setText(tr("Ошибка"));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setInformativeText(tr("Прибор не подключен, или не установлен драйвер"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }
}

QTableWidgetItem* TSController::getQTableWidgetItem(QVariant text){
    //    qDebug()<<"TSController::getQTableWidgetItem";
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setData(0,text);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    return item;
}

void TSController::breakExam()
{
    //    qDebug()<<"TSController::breakExam";
    m_adc_reader->stopADC();
}

void TSController::processDataParams(){
    //    qDebug()<<"TSController::processDataParams";
    qDebug()<<"this is result button !";
    QTableWidget *qtw = ui.resultsTable;
    qtw->setColumnCount(2);
    qtw->setRowCount(12);
    qtw->verticalHeader()->setVisible(false);
    qtw->setHorizontalHeaderLabels(QString(tr("Параметр;Значение")).split(";"));
    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    float AvgExpirationSpeed=0, MaxExpirationSpeed=0, AvgExpirationTime=0, AvgInspirationTime=0,
            AvgRoundTime=0, AvgTempIn=0, AvgTempOut=0, AvgTempInMinusAvgTempOut=0,  BreathingVolume=0, MVL=0, MinuteVentilation=0;
    float InspirationFrequency = 0;

    QVector<int> volume = curveBuffer.volumeVector();
    QVector<int> temp_in = curveBuffer.tempInVector();
    QVector<int> temp_out = curveBuffer.tempOutVector();

    VolumeSolver* vs = new VolumeSolver(volume,temp_in,temp_out);

    AvgExpirationSpeed = vs->getAverageExpirationSpeed();
    qtw->setItem(1,0,getQTableWidgetItem(tr("Средняя скорость выдоха(л/с)")));
    qtw->setItem(1,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)AvgExpirationSpeed)))));

    MaxExpirationSpeed = vs->getMaxExpirationSpeed();
    qtw->setItem(2,0,getQTableWidgetItem(tr("Максимальная скорость выдоха(л/с)")));
    qtw->setItem(2,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)MaxExpirationSpeed)))));

    AvgExpirationTime = vs->getAverageExpirationTime();
    qtw->setItem(3,0,getQTableWidgetItem(tr("Среднее время выдоха(с)")));
    qtw->setItem(3,1,getQTableWidgetItem((QString::number((float)AvgExpirationTime))));

    AvgInspirationTime = vs->getAverageInspirationTime();
    qtw->setItem(4,0,getQTableWidgetItem(tr("Среднее время вдоха(с)")));
    qtw->setItem(4,1,getQTableWidgetItem((QString::number((float)AvgInspirationTime))));

    AvgRoundTime = vs->getAverageCycleTime();
    qtw->setItem(5,0,getQTableWidgetItem(tr("Средняя время цикла(с)")));
    qtw->setItem(5,1,getQTableWidgetItem((QString::number((float)AvgRoundTime))));

    InspirationFrequency = vs->getInspirationFrequancyInOneMinute();
    qtw->setItem(6,0,getQTableWidgetItem(tr("Частота дыхания(ед/мин)")));
    qtw->setItem(6,1,getQTableWidgetItem((QString::number(InspirationFrequency))));

    MinuteVentilation = vs->getMinuteVentilation();
    qtw->setItem(8,0,getQTableWidgetItem(tr("Минутная вентиляция легких(л)")));
    qtw->setItem(8,1,getQTableWidgetItem(QString::number(curveBuffer.volToLtr(MinuteVentilation))));

    BreathingVolume = vs->getAverageInspiratonVentilation();
    qtw->setItem(7,0,getQTableWidgetItem(tr("Дыхательный объем(л)")));
    qtw->setItem(7,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)BreathingVolume)))));

    MVL = vs->getTotalVentilation();
    qtw->setItem(9,0,getQTableWidgetItem(tr("Суммарная вентиляция легких(л)")));
    qtw->setItem(9,1,getQTableWidgetItem(QString::number(fabs(curveBuffer.volToLtr((int)MVL)))));

    //ga->clear();

    AvgTempIn = vs->getAverageInspirationTempetature();
    qtw->setItem(10,0,getQTableWidgetItem(tr("Средняя температура вдоха( 'C)")));
    qtw->setItem(10,1,getQTableWidgetItem(QString::number(curveBuffer.tempInToDeg(AvgTempIn))));
    //gai->clear();

    AvgTempOut = vs->getAverageExpirationTempetature();
    qtw->setItem(11,0,getQTableWidgetItem(tr("Средняя температура выдоха( 'C)")));
    qtw->setItem(11,1,getQTableWidgetItem(QString::number(curveBuffer.tempOutToDeg(AvgTempOut))));

    AvgTempInMinusAvgTempOut = AvgTempOut-AvgTempIn;
    qtw->setItem(12,0,getQTableWidgetItem(tr("Средняя Твдоха-Средняя Твыдоха( 'C)")));
    qtw->setItem(12,1,getQTableWidgetItem(curveBuffer.tempOutToDeg(AvgTempOut)-curveBuffer.tempInToDeg(AvgTempIn)));

    qtw->removeRow(0);
    qtw->show();
    delete vs;
}

void TSController::printReport()
{
    //    qDebug()<<"TSController::printReport";
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Предварительный просмотр"));

    int endIndex=curveBuffer.getLenght();

    float listh=printer.widthMM()*printer.resolution()/25.4-60;
    float listw=printer.heightMM()*printer.resolution()/25.4-60;
    printer.setPageMargins(5,5,5,5,QPrinter::Millimeter);
    printer.setOrientation(QPrinter::Landscape);
    printer.setResolution(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    Ui::Form pf;

    pf.setupUi(&wpf);
    pf.mainBox->setMaximumSize((int)listw,(int)listh);
    pf.mainBox->setMinimumSize((int)listw,(int)listh);
    pf.resultsTable->setMinimumWidth(40+(int)listw/3);
    pf.resultsTable->setRowCount(13);
    pf.resultsTable->setColumnCount(2);
    pf.resultsTable->verticalHeader()->setVisible(false);
    pf.resultsTable->setHorizontalHeaderLabels(QString(tr("Параметр; Значение")).split(";"));
    pf.resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    int i=0, j=0;
    for(i=0; i < ui.resultsTable->rowCount(); i++){
        for(j=0; j < 2; j++){
            pf.resultsTable->setItem(i,j,getQTableWidgetItem(ui.resultsTable->item(i,j)->text()));
        }
    }
    int myH=0,myW=0;
    wpf.resize(pf.mainBox->size());
    wpf.show();

    myH = pf.gVolume->height();
    myW = pf.gVolume->width();

    QPixmap pmTempIn(myW,myH);
    QPixmap pmTempOut(myW,myH);
    QPixmap pmVolume(myW,myH);

    QPainter prTempIn;
    QPainter prTempOut;
    QPainter prVolume;
    prTempIn.begin(&pmTempIn);
    prTempOut.begin(&pmTempOut);
    prVolume.begin(&pmVolume);

    int h = pf.gVolume->height()/2;
    int step = h/10;
    if(h%10>=5)
    {
        h+=step/2;
    }
    prVolume.fillRect(0,0,myW,myH,Qt::white);
    prTempIn.fillRect(0,0,myW,myH,Qt::white);
    prTempOut.fillRect(0,0,myW,myH,Qt::white);

    prVolume.setPen(QColor(225,225,225));
    prTempIn.setPen(QColor(225,225,225));
    prTempOut.setPen(QColor(225,225,225));
    for(i=step;i<h;i+=step)
    {
        prVolume.drawLine(0,h+i,myW,h+i);
        prTempIn.drawLine(0,h+i,myW,h+i);
        prTempOut.drawLine(0,h+i,myW,h+i);
        prVolume.drawLine(0,h-i,myW,h-i);
        prTempIn.drawLine(0,h-i,myW,h-i);
        prTempOut.drawLine(0,h-i,myW,h-i);
    }
    for(i=10;i<myW;i+=10)
    {
        prVolume.drawLine(i,0,i,h<<1);
        prTempIn.drawLine(i,0,i,h<<1);
        prTempOut.drawLine(i,0,i,h<<1);
    }
    prVolume.setPen(QColor(0,0,0));
    prTempIn.setPen(QColor(0,0,0));
    prTempOut.setPen(curveBuffer.toutColor);
    prVolume.setPen(QColor(255,0,0));
    int* tinInt = curveBuffer.getTempInInterval();
    int* toutInt = curveBuffer.getTempOutInterval();
    int* volInt = curveBuffer.getVolumeInterval();
    float tempInK = 1;
    float tempOutK = 1;
    float tempInZ = h;
    float tempOutZ = h;
    double tempInAdaptive = (float)myH/(tinInt[1]-tinInt[0]);
    double tempOutAdaptive = (float)myH/(toutInt[1]-toutInt[0]);
    double printerVolumeAdaptive = (float)myH/(volInt[1]-volInt[0]);
    //volumeAdaptive = (float)myH/(volInt[1]-volInt[0]);
    //!!!!!!tempInZ = h + ceil((float)(tinInt[1]+tinInt[0])*m_plotter.getTempInAdaptive()*tempInK/2);
    //!!!!!!tempOutZ = h + ceil((float)(toutInt[1]+toutInt[0])*m_plotter.getTempOutAdaptive()*tempOutK/2);
    float volumeK =1;


    QVector<int> volume = curveBuffer.volumeVector();
    QVector<int> tempIn = curveBuffer.tempInVector();
    QVector<int> tempOut = curveBuffer.tempOutVector();
    i=0;
    int k=ceil((float)curveBuffer.getLenght()/pf.gTempIn->width());
    for(j=0;j<myW-35;j+=1)
    {
        if(i>=k*endIndex)break;
        prVolume.drawLine(
                    j,h-volumeK*printerVolumeAdaptive*volume[i],j+1,h-volumeK*printerVolumeAdaptive*volume[i+k]
                );
        prTempIn.drawLine(j,tempInZ-tempInK*tempInAdaptive*tempIn[i]
                          ,j+1,tempInZ-tempInK*tempInAdaptive*tempIn[i+k]);
        prTempOut.drawLine(j,tempOutZ-tempOutK*tempOutAdaptive*tempOut[i]
                           ,j+1,tempOutZ-tempOutK*tempOutAdaptive*tempOut[i+k]);
        i+=k;
    }
    pf.gVolume->setPixmap(pmVolume);
    pf.gTempIn->setPixmap(pmTempIn);
    pf.gTempOut->setPixmap(pmTempOut);
    pf.PatientName->setText(m_exam_cntrl.getSName()+" "+m_exam_cntrl.getFName());

    wpf.hide();
    if (dialog->exec() == QDialog::Accepted){
        QPainter painter;
        painter.begin(&printer);
        pf.mainBox->render(&painter);
    }
}

void TSController::closeEvent(QCloseEvent *e){
    m_adc_reader->stopADC();
    e->accept();
}

void TSController::selectedExamination(VTT_Data data)
{
    curveBuffer.appendData(data.volume, data.tempin, data.tempout);
    curveBuffer.setVolumeColibration(data.volZero, false);

    qDebug()<<"setVolumeConverts openExam "<<data.volOut<<" "<<data.volIn;
    /* curveBuffer.setVolumeConverts(record.value("volOut").toInt(),
                                   record.value("volIn").toInt());*///перепутано
    curveBuffer.setVolumeConverts(data.volIn, data.volOut);
    ui.startExam->setEnabled(false);
    ui.stopExam->setEnabled(false);
    ui.mainBox->setCurrentIndex(5);

    ui.managmentSpaser->setGeometry(QRect(0,0,350,2));
    ui.managmentBox->setVisible(true);
    ui.managmentBox->setEnabled(true);

    processDataParams();
}

void TSController::on_backCallibrateButton_clicked()
{
    ui.mainBox->setCurrentIndex(3);
    curveBuffer.clean();
}

void TSController::getZeroSognalLevels()
{
    //    m_adc_reader->setSamples_number(384);
    //    m_adc_reader->startADC(0);
    //    sleep(3000);
    //    m_adc_reader->stopADC();
}

