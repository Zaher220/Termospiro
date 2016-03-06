#include "examscontrollerl.h"

ExamsController::ExamsController(QObject *parent) : QObject(parent)
{

}

void ExamsController::CreatePatientProfile()
{
    QSqlRecord record;
    record = patientsModel->record();
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
    openUser = true;
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

void ExamsController::completePatientName(QString string)
{//qDebug()<<"TSController::completePatientName";
    string.toUpper();
    patientsModel->setFilter("sname like '"+string+"%' or code like '"+string+"%'");
    patientsModel->select();

}

void ExamsController::openPatientProfile(QModelIndex ind)
{
    //qDebug()<<"TSController::openPatientProfile";
    QSqlRecord record;
    if( ind.row() == -1 && ind.column() == -1 ){
        record = patientsModel->record(patientsModel->rowCount()-1);
    }
    else{
        record = patientsModel->record(ind.row());
    }

    m_ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "+record.value("fname").toString()+
                                    " "+record.value("fdname").toString());
    openPrivateDB(record);

    patientsModel->setFilter("id="+record.value("id").toString());
    examinationsModel = new TSExaminations(examinationsConnection);
    m_ui->examsTableView->setModel(examinationsModel);
    m_ui->examsTableView->setColumnHidden(0,true);
    for(int i=3;i<10;i++){
        m_ui->examsTableView->setColumnHidden(i,true);
    }
    m_ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->mainBox->setCurrentIndex(3);
    m_ui->examsTableView->horizontalHeader()->setDefaultSectionSize((m_ui->examsTableView->width()-2)/3);
}

void ExamsController::deleteExam(int index)
{
    //    qDebug()<<"TSController::deleteExam";
    examinationsModel->removeRow(index);
}

void ExamsController::deletePatient(int index)
{
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

void ExamsController::openExam(QModelIndex ind)
{
    // qDebug()<<"TSController::openExam";
    QSqlRecord record = examinationsModel->record(ind.row());
    //int volume[18000],tempin[18000], tempout[18000];
    QVector<int> volume,tempin, tempout;
    int i;
    QStringList list = record.value("volume").toString().split(";");
    for(i = 0; i < list.count(); i++)
    {
        volume.push_back(list.at(i).toInt());
    }
    list = record.value("tempIn").toString().split(";");
    for(i=0;i<list.count();i++)
    {
        tempin.push_back(list.at(i).toInt());
    }
    list = record.value("tempOut").toString().split(";");
    for(i=0;i<list.count();i++)
    {
        tempout.push_back(list.at(i).toInt());
    }
    //curveBuffer.setValues(volume, tempin, tempout, list.count());

    VTT_Data data;
    data.volume = volume;
    data.tempin = tempin;
    data.tempout = tempout;
    data.volZero = record.value("volZero").toInt();
    data.volIn = record.value("volIn").toInt();
    data.volOut = record.value("volOut").toInt();
    emit selectedExam(data);
}

void ExamsController::EditPatientProfile()
{
    qDebug()<<"TSController::editPatientProfile";
    switch(m_ui->mainBox->currentIndex())
    {
    case 0:
    {
        currentAction = CreatePatientProfileAction;
        m_ui->fName->clear();
        m_ui->sName->clear();
        m_ui->fdName->clear();
        m_ui->mvl->clear();
        m_ui->date->clear();
        m_ui->idEdit->clear();
        m_ui->mGenderRadio->setChecked(true);
        break;
    }
    case 3:
    {
        currentAction = EditPatientProfileAction;
        QSqlRecord record = patientsModel->record(0);
        m_ui->fName->setText(record.value("fname").toString());
        m_ui->sName->setText(record.value("sname").toString());
        m_ui->fdName->setText(record.value("fdname").toString());
        m_ui->mvl->setText(record.value("mvl").toString());
        QStringList d = record.value("birth_date").toString().split("-");
        QString date = d.at(2)+"-"+d.at(1)+"-"+d.at(0);
        m_ui->date->setText(date);
        if(record.value("gender").toString()==tr("м"))
            m_ui->mGenderRadio->setChecked(true);
        else
            m_ui->fGenderRadio->setChecked(true);
        break;
    }
    default: break;
    }
    m_ui->mainBox->setCurrentIndex(1);
    openUser = 0;
}

void ExamsController::rejectPatientProfile()
{
    qDebug()<<"TSController::rejectPatientProfile";
    switch(currentAction)
    {
    case CreatePatientProfileAction:
    {
        currentAction = NoAction;
        m_ui->mainBox->setCurrentIndex(0);
        break;
    }
    case EditPatientProfileAction:
    {
        currentAction = NoAction;
        m_ui->mainBox->setCurrentIndex(3);
        break;
    }
    default: break;
    }
}

void ExamsController::on_backPatientProfileButton_clicked(bool)
{
    if(currentAction == CreatePatientProfileAction){
        m_ui->mainBox->setCurrentIndex(0);
        patientsModel->setFilter("");
        patientsModel->select();
    }
    if(currentAction == NoAction){
        m_ui->mainBox->setCurrentIndex(2);
        patientsModel->setFilter("");
        patientsModel->select();
    }
    m_ui->mainBox->setCurrentIndex(2);
    patientsModel->setFilter("");
    patientsModel->select();
    //m_ui->horizontalScrollBar->setEnabled(false);
    qDebug()<<"Hellow click";
}

void ExamsController::on_backPatientListButton_clicked(bool)
{

    m_ui->mainBox->setCurrentIndex(0);
    patientsModel->setFilter("");
    patientsModel->select();
    //m_ui->horizontalScrollBar->setEnabled(false);
}

void ExamsController::on_openButton_clicked(bool)
{

    m_ui->mainBox->setCurrentIndex(0);
    patientsModel->setFilter("");
    patientsModel->select();
    //m_ui->horizontalScrollBar->setEnabled(false);
}

void ExamsController::setm_ui(Ui::TSView *ui)
{
    m_ui = ui;

    connect(m_ui->nameFilterEdit, SIGNAL(textChanged(QString)), this, SLOT(completePatientName(QString)));
    connect(m_ui->patientsTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openPatientProfile(QModelIndex)));
    connect(m_ui->examsTableView, SIGNAL(deleteRequest(int)), this, SLOT(deleteExam(int)));
    connect(m_ui->patientsTableView, SIGNAL(deleteRequest(int)), this, SLOT(deletePatient(int)));
    connect(m_ui->backPatientProfileButton, SIGNAL(clicked(bool)), this, SLOT(on_backPatientProfileButton_clicked(bool)));
    connect(m_ui->backPatientListButton, SIGNAL(clicked(bool)), this, SLOT(on_backPatientListButton_clicked(bool)));
    connect(m_ui->openButton, SIGNAL(clicked(bool)), this, SLOT(on_openButton_clicked(bool)));
    connect(m_ui->examsTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openExam(QModelIndex)));

    connect(m_ui->editProfileButton, SIGNAL(clicked(bool)), this, SLOT(EditPatientProfile()));
    connect(m_ui->backExamButton, SIGNAL(clicked(bool)), this, SLOT(on_backExamButton_clicked()));

    connect(m_ui->createButton, SIGNAL(clicked()), this, SLOT(EditPatientProfile()));

    connect(m_ui->editProfileButton, SIGNAL(clicked()), this, SLOT(EditPatientProfile()));
    connect(m_ui->openButton, SIGNAL(clicked(bool)), this, SLOT(openPatientList()));
    connect(m_ui->rejectProfileButton, SIGNAL(clicked()), this, SLOT(rejectPatientProfile()));

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
        msgBox.exec();
        */
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

double ExamsController::getMVL()
{
    return patientsModel->record(0).value("mvl").toDouble();
}

QString ExamsController::getFName()
{
    return patientsModel->record(0).value("fname").toString();
}

QString ExamsController::getSName()
{
    return patientsModel->record(0).value("sname").toString();
}

void ExamsController::on_backExamButton_clicked()
{
    if(openUser){
        qDebug()<<examinationsModel->filter();
    }
    m_ui->mainBox->setCurrentIndex(3);
    m_ui->managmentSpaser->setGeometry(QRect(0,0,2,2));
    m_ui->managmentBox->setVisible(false);
    emit reset();
}

void ExamsController::writeExamToDB(QVector<int> volume, QVector<int> tempIn, QVector<int> tempOut, double volIn, double volOut)
{

    QSqlRecord record = examinationsModel->record();

    QString val;

    for( int i = 0; i < volume.size(); i++ ){
        val.append(QString::number(volume[i])+";");
    }

    record.setValue("volume",val);
    val.clear();

    for( int i = 0; i < tempIn.size(); i++ ){
        val.append(QString::number(tempIn[i])+";");
    }

    record.setValue("tempIn",val);
    val.clear();

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
