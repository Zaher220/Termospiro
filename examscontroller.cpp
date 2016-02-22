#include "examscontrollerl.h"

ExamsController::ExamsController(QObject *parent) : QObject(parent)
{

}

void ExamsController::CreatePatientProfile()
{
//    if(!patientsModel->insertRecord(-1,record))
//    {
//        qDebug()<<patientsModel->lastError().text();
//        return;
//    }
//    record = patientsModel->record(patientsModel->rowCount()-1);
//    openPrivateDB(record);
//    QSqlQuery q(examinationsConnection);
//    q.prepare("CREATE TABLE `examinations` ( `id` INTEGER PRIMARY KEY AUTOINCREMENT,`date` DATE,`time` TIME,`indication` TEXT, `diagnosis` TEXT,`nurse` VARCHAR(50),`doctor` VARCHAR(50), `tempOut` TEXT,`tempIn` TEXT,`volume`  TEXT, `volZero` INT,volIn INT, volOut INT);");
//    if(!q.exec())
//    {
//        qDebug()<<q.lastError().text();
//    }
//    patientsModel->setFilter("id="+record.value("id").toString());
//    m_ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "
//                                 +record.value("fname").toString()+" "+record.value("fdname").toString());
//    examinationsModel = new TSExaminations(examinationsConnection);
//    m_ui->examsTableView->setModel(examinationsModel);
//    m_ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
//    m_ui->examsTableView->setColumnHidden(0,true);
//    for(int i=2;i<9;i++)
//    {
//        m_ui->examsTableView->setColumnHidden(i,true);
//    }
//    m_ui->mainBox->setCurrentIndex(3);
//    m_ui->examsTableView->horizontalHeader()->setDefaultSectionSize(m_ui->examsTableView->width()/2-1);

}

void ExamsController::savePatientProfile()
{/*
    qDebug()<<"TSController::savePatientProfile";
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Ошибка"));
    msgBox.setText(tr("Неправильный ввод данных."));*/
    QSqlRecord record;
    record = patientsModel->record();
    record.setValue("sname", m_ui->sName->text().toUpper());
    record.setValue("fname", m_ui->fName->text().toUpper());
    record.setValue("fdname", m_ui->fdName->text().toUpper());
    record.setValue("code", m_ui->idEdit->text().toUpper());
    record.setValue("mvl", m_ui->mvl->text().toUpper());
    switch(m_ui->mGenderRadio->isChecked())
    {
    case 0: {record.setValue("genger",tr("ж")); break;}
    case 1: {record.setValue("genger",tr("ж")); break;}
    }
    QStringList d = m_ui->date->text().split("-");
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
        m_ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "
                                     +record.value("fname").toString()+" "+record.value("fdname").toString());
        examinationsModel = new TSExaminations(examinationsConnection);
        m_ui->examsTableView->setModel(examinationsModel);
        m_ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_ui->examsTableView->setColumnHidden(0,true);
        for(int i=2;i<9;i++)
        {
            m_ui->examsTableView->setColumnHidden(i,true);
        }
        m_ui->mainBox->setCurrentIndex(3);
        m_ui->examsTableView->horizontalHeader()->setDefaultSectionSize(m_ui->examsTableView->width()/2-1);
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
        m_ui->mainBox->setCurrentIndex(2);
        currentAction = NoAction;
        break;
    }
    default: break;
    }
}

void ExamsController::openPatientList()
{
    //qDebug()<<"TSController::openPatientList";
    patientsModel->setFilter("");
    patientsModel->select();
    m_ui->patientsTableView->setModel(patientsModel);
    m_ui->patientsTableView->setColumnHidden(0, true);
    m_ui->patientsTableView->setColumnHidden(5, true);
    m_ui->patientsTableView->setColumnHidden(6, true);
    m_ui->patientsTableView->setColumnHidden(8, true);
    m_ui->patientsTableView->setColumnHidden(9, true);
    m_ui->patientsTableView->setColumnHidden(10, true);
    m_ui->mainBox->setCurrentIndex(2);
    m_ui->patientsTableView->horizontalHeader()->setDefaultSectionSize((m_ui->patientsTableView->width()-2)/5);
    m_ui->patientsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->patientsTableView->setEditTriggers(QTableView::NoEditTriggers);

}

void ExamsController::setm_ui(Ui::TSView *ui)
{
    m_ui = ui;
}



void ExamsController::openPrivateDB(QSqlRecord record)
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
        /*QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Ошибка"));
        msgBox.setText(tr("Произошла ошибка. Обратитесь к разработчикам. Код: 00002"));
        msgBox.exec();*/
        m_ui->mainBox->setCurrentIndex(0);
        return;
    }
}

void ExamsController::setAction(CurrentAction act)
{
    currentAction = act;
}

bool ExamsController::init()
{
    patientsConnection = QSqlDatabase::addDatabase("QSQLITE","Patients");
    patientsConnection.setDatabaseName("commondb\\common.db");
    if(!patientsConnection.open())
    {

        return false;
    }
    patientsModel = new TSPatients(patientsConnection);
    return true;
}

QSqlError ExamsController::getErrorPatientsConnection()
{
    return patientsConnection.lastError();
}

QSqlError ExamsController::getErrorexaminationsConnection()
{
    return examinationsConnection.lastError();
}

void ExamsController::writeExamToDB(QVector<int> volume, QVector<int> tempIn, QVector<int> tempOut, double volIn, double volOut)
{

    QSqlRecord record = examinationsModel->record();

    QString val;

    //QVector<int> volume = curveBuffer.volumeVector();
    for( int i = 0; i < volume.size(); i++ ){
        val.append(QString::number(volume[i])+";");
    }

    record.setValue("volume",val);
    val.clear();

    //QVector<int> tempIn = curveBuffer.tempInVector();
    for( int i = 0; i < tempIn.size(); i++ ){
        val.append(QString::number(tempIn[i])+";");
    }

    record.setValue("tempIn",val);
    val.clear();

    //QVector<int> tempOut = curveBuffer.tempOutVector();
    for( int i = 0; i < tempOut.size(); i++ ){
        val.append(QString::number(tempOut[i])+";");
    }

    record.setValue("tempOut",val);
    record.setValue("volZero",0);//curveBuffer.volumeColibration());
    record.setValue("volIn", volIn);
    record.setValue("volOut", volOut);
    record.setValue("date", QDate::currentDate().toString("yyyy-MM-dd"));
    record.setValue("time", QTime::currentTime().toString("hh:mm"));
    if(!examinationsModel->insertRecord(-1,record))
        qDebug()<<"exam insertError";
}
