#include "tscontroller.h"
#include "ui_tsview.h"
#include "ui_tsprogressdialog.h"
#include "ui_tsvolsignalwidget.h"
#include <QtSql/QSqlRecord>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include "tools/tsvalidationtools.h"
#include <QSqlDatabase>
#include <QTranslator>
#include <QApplication>
#include <QResizeEvent>
#include <QSqlError>
#include <QSqlQuery>
#include <QTime>
#include <QPrinter>
#include <QPrintDialog>

#include <QDate>
#include <math.h>
#include <QDir>
#include <fstream>
#include <QDialog>
#include <tsanalitics.h>
#include <QSettings>
#include <tsanalitics.h>
#include <QTableWidget>
#include <ui_tsprintview.h>
#include <QPrintEngine>
#include <tstempanalitic.h>
#include <QMessageBox>
using namespace std;


TSController::TSController(QWidget *parent):QMainWindow(parent),ui(new Ui::TSView)
{
    qDebug()<<"TSController::TSController";
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("CP-1251"));
    QSettings settings("settings.ini",QSettings::IniFormat);

    ui->setupUi(this);

    currentAction = NoAction;
    openUser = false;

    m_plotter.setCurveBuffer(&curveBuffer);

    curveBuffer.setReference(&settings);
    recordingStarted = false;


    m_plotter.setTempInInterval(curveBuffer.getTempInInterval());
    m_plotter.setTempInAdaptive(1.0);
    m_plotter.setTempOutInterval(curveBuffer.getTempOutInterval());
    m_plotter.setTempOutAdaptive(1.0);


    ui->managmentBox->setVisible(false);
    ui->vertLabel->setVisible(false);
    ui->horLabel->setVisible(false);
    ui->tempInScaleSlider->setVisible(false);
    ui->tempOutScaleSlider->setVisible(false);
    ui->volumeScaleSlider->setVisible(false);
    ui->breakExamButton->setVisible(false);
    patientsConnection = QSqlDatabase::addDatabase("QSQLITE","Patients");
    patientsConnection.setDatabaseName("commondb\\common.db");
    if(!patientsConnection.open())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Ошибка"));
        QString error = QString("Произошла ошибка.\nОбратитесь к разработчикам.\nКод: 00001.\nПрограмма будет завершена. ")+patientsConnection.lastError().text().toStdString().c_str();

        msgBox.setText(error);
        msgBox.exec();
        QApplication::exit(0);
    }
    patientsModel = new TSPatients(patientsConnection);
    connect(ui->createButton,SIGNAL(clicked()),this,SLOT(editPatientProfile()));
    connect(ui->saveProfileButton,SIGNAL(clicked()),this,SLOT(savePatientProfile()));
    connect(ui->ignoreCalibrateButton,SIGNAL(clicked()),this,SLOT(rejectColibration()));
    connect(ui->volumeCalibrateButton,SIGNAL(clicked()),this,SLOT(calibrateVolume()));
    connect(ui->startExam,SIGNAL(clicked()),this,SLOT(startExam()));
    //connect(&plotingTimer,SIGNAL(timeout()),this,SLOT(plotNow()));
    connect(ui->openButton,SIGNAL(clicked()),this,SLOT(openPatientList()));
    connect(ui->nameFilterEdit,SIGNAL(textChanged(QString)),this,SLOT(completePatientName(QString)));
    connect(ui->patientsTableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openPatientProfile(QModelIndex)));
    connect(&curveBuffer,SIGNAL(updateAverageData(int,int,int,int)),this,SLOT(showAverageData(int,int,int,int)));
    connect(ui->fName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui->sName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui->fdName,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui->date,SIGNAL(editingFinished()),this,SLOT(completePatientId()));
    connect(ui->editProfileButton,SIGNAL(clicked()),this,SLOT(editPatientProfile()));
    connect(ui->rejectProfileButton,SIGNAL(clicked()),this,SLOT(rejectPatientProfile()));
    connect(ui->horizontalScrollBar,SIGNAL(valueChanged(int)),this,SLOT(scrollGraphics(int)));
    connect(ui->newExamButton,SIGNAL(clicked()),this,SLOT(createNewExam()));
    connect(ui->stopExam,SIGNAL(clicked()),this,SLOT(stopExam()));

    connect(ui->examsTableView,SIGNAL(doubleClicked(QModelIndex)),this, SLOT(openExam(QModelIndex)));
    connect(ui->examsTableView,SIGNAL(deleteRequest(int)),this,SLOT(deleteExam(int)));

    connect(ui->tempInScaleSlider,SIGNAL(valueChanged(int)), &m_plotter, SLOT(scaleTempIn(int)));
    connect(ui->tempOutScaleSlider,SIGNAL(valueChanged(int)), &m_plotter,SLOT(scaleTempOut(int)));
    connect(ui->volumeScaleSlider,SIGNAL(valueChanged(int)), &m_plotter,SLOT(scaleVolume(int)));
    connect(ui->horScaleSlider,SIGNAL(valueChanged(int)), &m_plotter,SLOT(scaleForHorizontal(int)));
    connect(ui->horScaleSlider,SIGNAL(rangeChanged(int,int)),this,SLOT(changeScrollBarAfterScaling(int,int)));
    connect(ui->tempInScroll,SIGNAL(valueChanged(int)), &m_plotter,SLOT(changeTempInScrollValue(int)));
    connect(ui->breakExamButton,SIGNAL(clicked()),this,SLOT(breakExam()));
    connect(ui->resultsButton,SIGNAL(clicked()),this,SLOT(processDataParams()));
    connect(ui->patientsTableView,SIGNAL(deleteRequest(int)),this,SLOT(deletePatient(int)));

    connect(ui->printButton,SIGNAL(clicked()),this,SLOT(printReport()));
    ui->resultsButton->setEnabled(true);
    ui->backPatientProfileButton->installEventFilter(this);
    ui->backPatientListButton->installEventFilter(this);
    ui->openButton->installEventFilter(this);
    ui->backCallibrateButton->installEventFilter(this);
    ui->backExamButton->installEventFilter(this);

    connect(&m_adc_reader, SIGNAL(sendACQData(std::vector<std::vector<short> >)), &m_raw_data_parser, SLOT(setACQData(std::vector<std::vector<short> >)));
    connect(&m_raw_data_parser, SIGNAL(sendNewData(QVector<double>,QVector<double>,QVector<double>)), &curveBuffer, SLOT(appendData(QVector<double>,QVector<double>,QVector<double>)));

    ui->examsTableView->setEditTriggers(QTableView::NoEditTriggers);;
    //connect(reader,SIGNAL(done()),&d,SLOT(accept()));
    // connect(reader,SIGNAL(changeProgress(int)),dui.progressBar,SLOT(setValue(int)));
    ui->mainBox->setCurrentIndex(0);
    m_plotter.setUi(ui);
    m_plotter.setParentWindow(this);
    connect(&m_plotter, SIGNAL(stopACQU()), &m_adc_reader, SLOT(stopACQ()));
}

TSController::~TSController()
{
    qDebug()<<"TSController::~TSController";
    delete ui;
}

void TSController::editPatientProfile()
{
    qDebug()<<"TSController::editPatientProfile";
    switch(ui->mainBox->currentIndex())
    {
    case 0:
    {
        currentAction = CreatePatientProfileAction;
        ui->fName->clear();
        ui->sName->clear();
        ui->fdName->clear();
        ui->mvl->clear();
        ui->date->clear();
        ui->idEdit->clear();
        ui->mGenderRadio->setChecked(true);
        break;
    }
    case 3:
    {
        currentAction = EditPatientProfileAction;
        QSqlRecord record = patientsModel->record(0);
        ui->fName->setText(record.value("fname").toString());
        ui->sName->setText(record.value("sname").toString());
        ui->fdName->setText(record.value("fdname").toString());
        ui->mvl->setText(record.value("mvl").toString());
        QStringList d = record.value("birth_date").toString().split("-");
        QString date = d.at(2)+"-"+d.at(1)+"-"+d.at(0);
        ui->date->setText(date);
        if(record.value("gender").toString()==tr("м"))
            ui->mGenderRadio->setChecked(true);
        else
            ui->fGenderRadio->setChecked(true);
        break;
    }
    default: break;
    }
    ui->mainBox->setCurrentIndex(1);
    openUser = 0;
}

void TSController::savePatientProfile()
{
    qDebug()<<"TSController::savePatientProfile";
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Ошибка"));
    msgBox.setText(tr("Неправильный ввод данных."));
    QSqlRecord record;
    record = patientsModel->record();
    record.setValue("sname",ui->sName->text().toUpper());
    record.setValue("fname",ui->fName->text().toUpper());
    record.setValue("fdname", ui->fdName->text().toUpper());
    record.setValue("code",ui->idEdit->text().toUpper());
    record.setValue("mvl",ui->mvl->text().toUpper());
    switch(ui->mGenderRadio->isChecked())
    {
    case 0: {record.setValue("genger",tr("ж")); break;}
    case 1: {record.setValue("genger",tr("ж")); break;}
    }
    QStringList d = ui->date->text().split("-");
    QString date = d.at(2)+"-"+d.at(1)+"-"+d.at(0);
    record.setValue("birth_date",date);

    switch(currentAction)
    {
    case CreatePatientProfileAction:
    {
        if(!patientsModel->insertRecord(-1,record))
        {
            qDebug()<<patientsModel->lastError().text();
            return;
        }
        record = patientsModel->record(patientsModel->rowCount()-1);
        openPrivateDB(record);
        QSqlQuery q(examinationsConnection);
        q.prepare("CREATE TABLE `examinations` ( `id` INTEGER PRIMARY KEY AUTOINCREMENT,`date` DATE,`time` TIME,`indication` TEXT, `diagnosis` TEXT,`nurse` VARCHAR(50),`doctor` VARCHAR(50), `tempOut` TEXT,`tempIn` TEXT,`volume`  TEXT, `volZero` INT,volIn INT, volOut INT);");
        if(!q.exec())
        {
            qDebug()<<q.lastError().text();
        }
        patientsModel->setFilter("id="+record.value("id").toString());
        ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "
                                      +record.value("fname").toString()+" "+record.value("fdname").toString());
        examinationsModel = new TSExaminations(examinationsConnection);
        ui->examsTableView->setModel(examinationsModel);
        ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->examsTableView->setColumnHidden(0,true);
        for(int i=2;i<9;i++)
        {
            ui->examsTableView->setColumnHidden(i,true);
        }
        ui->mainBox->setCurrentIndex(3);
        ui->examsTableView->horizontalHeader()->setDefaultSectionSize(ui->examsTableView->width()/2-1);
        break;
    }
    case EditPatientProfileAction:
    {

        if(!patientsModel->setRecord(0,record))
        {
            qDebug()<<patientsModel->lastError().text();
            return;
        }
        patientsModel->submitAll();
        patientsModel->select();
        ui->mainBox->setCurrentIndex(2);
        currentAction = NoAction;
        break;
    }
    default: break;
    }
}

void TSController::rejectPatientProfile()
{
    qDebug()<<"TSController::rejectPatientProfile";
    switch(currentAction)
    {
    case CreatePatientProfileAction:
    {
        currentAction = NoAction;
        ui->mainBox->setCurrentIndex(0);
        break;
    }
    case EditPatientProfileAction:
    {
        currentAction = NoAction;
        ui->mainBox->setCurrentIndex(3);
        break;
    }
    default: break;
    }
}

void TSController::calibrateVolume(){
    QSettings settings("settings.ini",QSettings::IniFormat);
    QDialog d(this);
    Ui::TSProgressDialog dui;
    //d.setWindowFlags(Qt::SubWindow);
    dui.setupUi(&d);

    connect(&m_adc_reader, SIGNAL(done()), &d, SLOT(accept()));
    connect(&m_adc_reader, SIGNAL(changeProgress(int)), dui.progressBar, SLOT(setValue(int)));

    //controller->setWindowFlags(Qt::WindowTitleHint | Qt::WindowMinimizeButtonHint|Qt::SubWindow);
    d.setWindowTitle(tr("Предупреждение"));
    dui.information->setText(tr("Идет подготовка..."));
    dui.acceptButton->setVisible(false);

    m_adc_reader.setSamples_number(1250);
    m_adc_reader.startACQ();

    if(d.exec()==1){
        settings.setValue("volZero",curveBuffer.volumeColibration());
        dui.information->setText(tr("Подготовка завершена.\nНажмите ОК и качайте шприцем."));
        dui.progressBar->setVisible(false);
        dui.acceptButton->setVisible(true);
    }
    m_adc_reader.stopACQ();


    d.exec();
    //connect(&cPlotingTimer,SIGNAL(timeout()),this,SLOT(plotCalibration()));

    m_adc_reader.startACQ();
    m_plotter.startCPlottingTimer(100);
}

void TSController::rejectColibration()
{
    //qDebug()<<"TSController::rejectColibration";
    QSettings settings("settings.ini",QSettings::IniFormat);
    curveBuffer.setVolumeColibration(settings.value("volZero").toInt(),false);
    qDebug()<<"volColibr: "<<curveBuffer.volumeColibration();
    qDebug()<<"setVolumeConverts rejectColibration "<<settings.value("volInLtr").toInt()<<" "<<settings.value("volOutLtr").toInt();
    curveBuffer.setVolumeConverts(settings.value("volInLtr").toInt(), settings.value("volOutLtr").toInt());
    ui->mainBox->setCurrentIndex(5);
    curveBuffer.setEnd(0);
    ui->startExam->setEnabled(true);
    ui->stopExam->setEnabled(true);

    m_plotter.initPaintDevices();
    m_plotter.plotNow();

    ui->managmentSpaser->setGeometry(QRect(0,0,350,2));
    ui->managmentBox->setVisible(true);
    ui->managmentBox->setEnabled(true);
}

void TSController::startExam()
{
    ui->volumeInfoLabel->setText(tr("ДО=0")
                                 +tr(" Л\nЧД=0"));
    ui->tinInfoLabel->setText("Tin=0 'C");
    ui->toutInfolabel->setText("Tout=0 'C");
    ui->volumeInfoLabel->setVisible(true);
    ui->tinInfoLabel->setVisible(true);
    ui->toutInfolabel->setVisible(true);
    curveBuffer.clean();

    m_adc_reader.startACQ();

    myTimer.start();
    recordingStarted = true;

    m_plotter.setTempInScaleRate(1.0/5000);
    m_plotter.setTempOutScaleRate(1.0/5000);
    m_plotter.setVolumeScaleRate(1.0/5000);
    m_plotter.setHorizontalStep(1.0);
    m_plotter.initPaintDevices();
    m_plotter.startPlottingTimer(100);

    mvlDialog = new QDialog(this);
    volWidget = new Ui::TSVolSignalWidget();
    volWidget->setupUi(mvlDialog);
    volWidget->MVL->setText("0%");
    mvlDialog->setModal(false);
    mvlDialog->show();
    ui->startExam->setEnabled(false);
    ui->horizontalScrollBar->setEnabled(false);
}

void TSController::stopExam()
{
    //qDebug()<<"TSController::stopExam";
    if(recordingStarted)
    {
        m_plotter.stopPlottingTimer();

        m_adc_reader.stopACQ();

        qDebug()<<"Stop exam";
        QSqlRecord record = examinationsModel->record();
        int n = curveBuffer.end();
        QString val;
        int i;
        int *mass = curveBuffer.volume();
        for(i=0;i<n;i++){
            val.append(QString::number(mass[i])+";");
        }
        record.setValue("volume",val);
        val.clear();
        mass = curveBuffer.tempIn();
        for(i=0;i<n;i++)
        {
            val.append(QString::number(mass[i])+";");
        }
        record.setValue("tempIn",val);
        val.clear();
        mass = curveBuffer.tempOut();
        for(i=0;i<n;i++)
        {
            val.append(QString::number(mass[i])+";");
        }
        record.setValue("tempOut",val);
        record.setValue("volZero",0);//curveBuffer.volumeColibration());
        record.setValue("volIn",curveBuffer.volumeConverts().at(1));
        record.setValue("volOut",curveBuffer.volumeConverts().at(0));
        record.setValue("date",QDate::currentDate().toString("yyyy-MM-dd"));
        record.setValue("time",QTime::currentTime().toString("hh:mm"));
        if(!examinationsModel->insertRecord(-1,record))
            qDebug()<<"exam insertError";
        ui->horizontalScrollBar->setEnabled(true);

    }

    ui->horizontalScrollBar->setEnabled(true);
    mvlDialog->close();
    recordingStarted = false;
}

void TSController::openPatientList()
{
    //qDebug()<<"TSController::openPatientList";
    patientsModel->setFilter("");
    patientsModel->select();
    ui->patientsTableView->setModel(patientsModel);
    ui->patientsTableView->setColumnHidden(0, true);
    ui->patientsTableView->setColumnHidden(5, true);
    ui->patientsTableView->setColumnHidden(6, true);
    ui->patientsTableView->setColumnHidden(8, true);
    ui->patientsTableView->setColumnHidden(9, true);
    ui->patientsTableView->setColumnHidden(10, true);
    ui->mainBox->setCurrentIndex(2);
    ui->patientsTableView->horizontalHeader()->setDefaultSectionSize((ui->patientsTableView->width()-2)/5);
    ui->patientsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->patientsTableView->setEditTriggers(QTableView::NoEditTriggers);
    openUser = true;
}

void TSController::completePatientName(QString string)
{
    //qDebug()<<"TSController::completePatientName";
    string.toUpper();
    patientsModel->setFilter("sname like '"+string+"%' or code like '"+string+"%'");
    patientsModel->select();
}

void TSController::openPatientProfile(QModelIndex ind)
{
    //qDebug()<<"TSController::openPatientProfile";
    QSqlRecord record;
    if( ind.row() == -1 && ind.column() == -1 ){
        record = patientsModel->record(patientsModel->rowCount()-1);
    }
    else{
        record = patientsModel->record(ind.row());
    }

    ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "+record.value("fname").toString()+
                                  " "+record.value("fdname").toString());
    openPrivateDB(record);

    patientsModel->setFilter("id="+record.value("id").toString());
    examinationsModel = new TSExaminations(examinationsConnection);
    ui->examsTableView->setModel(examinationsModel);
    ui->examsTableView->setColumnHidden(0,true);
    for(int i=3;i<10;i++){
        ui->examsTableView->setColumnHidden(i,true);
    }
    ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mainBox->setCurrentIndex(3);
    ui->examsTableView->horizontalHeader()->setDefaultSectionSize((ui->examsTableView->width()-2)/3);
}

void TSController::showAverageData(int avgTempIn, int avgTempOut, int avgDO, int avgCHD){
    ui->volumeInfoLabel->setText(tr("ДО=")+QString::number(curveBuffer.volToLtr(avgDO),'g',2)+tr(" Л\nЧД=")+QString::number(avgCHD));
    ui->tinInfoLabel->setText("Tin="+QString::number(curveBuffer.tempInToDeg(avgTempIn),'g',2)+" 'C");
    ui->toutInfolabel->setText("Tout="+QString::number(curveBuffer.tempInToDeg(avgTempOut),'g',2)+" 'C");
    int mvl = (curveBuffer.volToLtr(avgDO)*avgCHD)*100/patientsModel->record(0).value("mvl").toDouble();
    if(recordingStarted&&curveBuffer.end()>10)
        volWidget->MVL->setText(QString::number(mvl)+"%");
}

void TSController::completePatientId()
{
    // qDebug()<<"TSController::completePatientId";
    QString id;
    if(ui->sName->text().length()>0)
        id.append(ui->sName->text().toUpper().at(0));
    if(ui->fName->text().length()>0)
        id.append(ui->fName->text().toUpper().at(0));
    if(ui->fdName->text().length()>0)
        id.append(ui->fdName->text().toUpper().at(0));
    id.append(ui->date->text().replace("-",""));
    ui->idEdit->setText(id);
}

void TSController::scrollGraphics(int value)
{
    // qDebug()<<"TSController::scrollGraphics";
    if(curveBuffer.end() != 0)
        curveBuffer.setEnd(m_plotter.getW()-35+value*m_plotter.getHorizontalStep()*10);
    m_plotter.plotNow();
}

void TSController::createNewExam(){
    if ( m_adc_reader.isReady() == true ){
        qDebug()<<"TSController::createNewExam";
        ui->mainBox->setCurrentIndex(4);
        m_plotter.setCH(ui->calibrateVolumeAnimation->height());
        m_plotter.setCW(ui->calibrateVolumeAnimation->width());
        m_plotter.resetPlotting();
        curveBuffer.clean();
        curveBuffer.setEnd(0);
        curveBuffer.setLenght(0);
        m_plotter.setMaxcVol(0);
        m_plotter.plotCalibration();
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

void TSController::openExam(QModelIndex ind)
{
    // qDebug()<<"TSController::openExam";
    QSqlRecord record = examinationsModel->record(ind.row());
    int volume[18000],tempin[18000], tempout[18000];
    int i;
    QStringList list = record.value("volume").toString().split(";");
    for(i=0;i<list.count();i++)
    {
        volume[i] = list.at(i).toInt();
    }
    list = record.value("tempIn").toString().split(";");
    for(i=0;i<list.count();i++)
    {
        tempin[i] = list.at(i).toInt();
    }
    list = record.value("tempOut").toString().split(";");
    for(i=0;i<list.count();i++)
    {
        tempout[i] = list.at(i).toInt();
    }
    curveBuffer.setValues(volume,tempin,tempout,list.count());
    curveBuffer.setVolumeColibration(record.value("volZero").toInt(),false);

    qDebug()<<"setVolumeConverts openExam "<<record.value("volOut").toInt()<<" "<<record.value("volIn").toInt();
    /* curveBuffer.setVolumeConverts(record.value("volOut").toInt(),
                                   record.value("volIn").toInt());*///перепутано
    curveBuffer.setVolumeConverts(record.value("volIn").toInt(), record.value("volOut").toInt());
    ui->startExam->setEnabled(false);
    ui->stopExam->setEnabled(false);
    ui->mainBox->setCurrentIndex(5);

    ui->horizontalScrollBar->setMaximum((list.count()-ui->gVolume->width())/10);
    ui->horizontalScrollBar->setValue(0);
    ui->horizontalScrollBar->setEnabled(true);
    m_plotter.initPaintDevices();
    curveBuffer.setEnd(m_plotter.getW()-35);

    m_plotter.setTempInScaleRate(1.0/5000);
    m_plotter.setTempOutScaleRate(1.0/5000);
    m_plotter.setVolumeScaleRate(4.0/5000);
    m_plotter.setHorizontalStep(1.0);

    ui->managmentSpaser->setGeometry(QRect(0,0,350,2));
    ui->managmentBox->setVisible(true);
    ui->managmentBox->setEnabled(true);

    m_plotter.plotNow();
    processDataParams();
    qDebug()<<"TableWidget: "<<ui->resultsTable->width()<<" "<<ui->resultsTable->height();
    qDebug()<<"Button: "<<ui->startExam->width()<<" "<<ui->startExam->height();
}

void TSController::resizeEvent(QResizeEvent *evt)
{
    //qDebug()<<"TSController::resizeEvent";
    m_plotter.initPaintDevices();
    m_plotter.plotNow();
}


void TSController::changeScrollBarAfterScaling(int before, int after)
{
    // qDebug()<<"TSController::changeScrollBarAfterScaling";
    if(after%2)
        return;
    int val = ui->horizontalScrollBar->value();
    if(before>after)
    {
        ui->horizontalScrollBar->setMaximum(ui->horizontalScrollBar->maximum()/2);
        ui->horizontalScrollBar->setValue(val/2);
    }
    else
    {
        ui->horizontalScrollBar->setMaximum(ui->horizontalScrollBar->maximum()*2);
        ui->horizontalScrollBar->setValue(val*2);
    }
}

bool TSController::eventFilter(QObject *obj, QEvent *e)
{
    //qDebug()<<"TSController::eventFilter";

    QMouseEvent *evt;
    if(e->type() == QEvent::MouseButtonPress)
        evt = static_cast<QMouseEvent*>(e);
    if(obj == ui->backPatientProfileButton && evt->button()==Qt::LeftButton)
    {
        if(currentAction == CreatePatientProfileAction){
            ui->mainBox->setCurrentIndex(0);
            patientsModel->setFilter("");
            patientsModel->select();
        }
        if(currentAction == NoAction){
            ui->mainBox->setCurrentIndex(2);
            patientsModel->setFilter("");
            patientsModel->select();
        }
        ui->horizontalScrollBar->setEnabled(false);
    }
    //if (obj == ui->backPatientListButton && evt->button()==Qt::LeftButton)
    if (obj == ui->backPatientListButton && evt->button()==Qt::LeftButton)
    {
        ui->mainBox->setCurrentIndex(0);
       // patientsModel->setFilter("");
        patientsModel->select();
        ui->horizontalScrollBar->setEnabled(false);
    }
    if (obj == ui->openButton && evt->button()==Qt::LeftButton)
    {
        ui->mainBox->setCurrentIndex(0);
        /*patientsModel->setFilter("");
        patientsModel->select();*/
        ui->horizontalScrollBar->setEnabled(false);
    }
    if(obj == ui->backCallibrateButton && evt->button()==Qt::LeftButton)
    {
        ui->mainBox->setCurrentIndex(3);
        curveBuffer.clean();
    }
    if(obj == ui->backExamButton && evt->button()==Qt::LeftButton)
    {
        if(!openUser){
            ui->mainBox->setCurrentIndex(4);
            ui->managmentSpaser->setGeometry(QRect(0,0,2,2));
            ui->managmentBox->setVisible(false);
        }
        else{
            qDebug()<<examinationsModel->filter();
            ui->mainBox->setCurrentIndex(3);
            ui->managmentSpaser->setGeometry(QRect(0,0,2,2));
            ui->managmentBox->setVisible(false);
        }
        curveBuffer.clean();
    }
    return QObject::eventFilter(obj,e);
}

void TSController::openPrivateDB(QSqlRecord record)
{
    //qDebug()<<"TSController::openPrivateDB";
    examinationsConnection = QSqlDatabase::addDatabase("QSQLITE","ExamConnection");
    QDir d;
    if(!d.cd("pravatedb"))
    {
        d.mkpath("privatedb");
    }
    d.setPath(d.path()+"\\privatedb");
    examinationsConnection.setDatabaseName(d.path()+"\\"+record.value("id").toString()+"_"+
                                           record.value("code").toString()+".db");
    if(!examinationsConnection.open())
    {
        qDebug()<<examinationsConnection.lastError().text();
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Ошибка"));
        msgBox.setText(tr("Произошла ошибка. Обратитесь к разработчикам. Код: 00002"));
        msgBox.exec();
        ui->mainBox->setCurrentIndex(0);
        return;
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
    m_plotter.stopPlottingTimer();
    m_adc_reader.stopACQ();
}

void TSController::processDataParams(){
    //    qDebug()<<"TSController::processDataParams";
    qDebug()<<"this is result button !";
    QTableWidget *qtw = ui->resultsTable;
    qtw->setColumnCount(2);
    qtw->setRowCount(12);
    qtw->verticalHeader()->setVisible(false);
    qtw->setHorizontalHeaderLabels(QString(tr("Параметр;Значение")).split(";"));
    qtw->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    float AvgExpirationSpeed=0, MaxExpirationSpeed=0, AvgExpirationTime=0, AvgInspirationTime=0,
            AvgRoundTime=0, AvgTempIn=0, AvgTempOut=0, AvgTempInMinusAvgTempOut=0,  BreathingVolume=0, MVL=0, MinuteVentilation=0;
    float InspirationFrequency=0;
    int *vo = curveBuffer.volume();
    int *ti = curveBuffer.tempIn();
    int *to = curveBuffer.tempOut();
    QVector<int> volume,temp_in,temp_out;
    for(int i=0;i<curveBuffer.getLenght();i++){
        volume.push_back(vo[i]);
        temp_in.push_back(ti[i]);
        temp_out.push_back(to[i]);
    }
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

void TSController::deletePatient(int index){
    //    qDebug()<<"TSController::deletePatient";
    QSqlRecord record = patientsModel->record(index);
    QString fileName = "privatedb\\";
    fileName.append(record.value("id").toString()+"_"
                    +record.value("code").toString()+".db");
    QFile file(fileName);
    if(file.exists()){
        if(file.remove()){
            qDebug()<<"file: "<<fileName<<" is deleted succesfuly";
        }
        file.close();
    }
    patientsModel->removeRow(index);
}

void TSController::deleteExam(int index){
    //    qDebug()<<"TSController::deleteExam";
    examinationsModel->removeRow(index);
}

void TSController::printReport()
{
    //    qDebug()<<"TSController::printReport";
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Предварительный просмотр"));

    int endIndex=curveBuffer.lenght;

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
    for(i=0; i < ui->resultsTable->rowCount(); i++){
        for(j=0; j < 2; j++){
            pf.resultsTable->setItem(i,j,getQTableWidgetItem(ui->resultsTable->item(i,j)->text()));
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
    tempInZ = h + ceil((float)(tinInt[1]+tinInt[0])*m_plotter.getTempInAdaptive()*tempInK/2);
    tempOutZ = h + ceil((float)(toutInt[1]+toutInt[0])*m_plotter.getTempOutAdaptive()*tempOutK/2);
    float volumeK =1;
    int *volume = curveBuffer.volume();
    int *tempIn = curveBuffer.tempIn();
    int *tempOut = curveBuffer.tempOut();
    i=0;
    int k=ceil((float)curveBuffer.lenght/pf.gTempIn->width());
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
    pf.PatientName->setText(patientsModel->record(0).value("sname").toString()+" "+patientsModel->record(0).value("fname").toString());

    wpf.hide();
    if (dialog->exec() == QDialog::Accepted){
        QPainter painter;
        painter.begin(&printer);
        pf.mainBox->render(&painter);
    }
}

void TSController::closeEvent(QCloseEvent *e){
    m_adc_reader.stopACQ();
    e->accept();
}

void TSController::on_openButton_clicked()
{
    ui->mainBox->setCurrentIndex(0);
    patientsModel->setFilter("");
    patientsModel->select();
    ui->horizontalScrollBar->setEnabled(false);
}
