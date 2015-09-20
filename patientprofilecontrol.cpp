#include "patientprofilecontrol.h"

PatientProfileControl::PatientProfileControl(QObject *parent) : QObject(parent)
{

}

void PatientProfileControl::openPatientProfile(QModelIndex ind)
{
    //qDebug()<<"TSController::openPatientProfile";
    QSqlRecord record;
    if( ind.row() == -1 && ind.column() == -1 )
    {
        record = patientsModel->record(patientsModel->rowCount()-1);
    }
    else
    {
        record = patientsModel->record(ind.row());
    }

    ui->patientPageLabel->setText(tr("Пациент: ")+record.value("sname").toString()+" "+record.value("fname").toString()+
                                  " "+record.value("fdname").toString());
    openPrivateDB(record);

    patientsModel->setFilter("id="+record.value("id").toString());
    examinationsModel = new TSExaminations(examinationsConnection);
    ui->examsTableView->setModel(examinationsModel);
    ui->examsTableView->setColumnHidden(0,true);
    for(int i=3;i<10;i++)
    {
        ui->examsTableView->setColumnHidden(i,true);
    }
    ui->examsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mainBox->setCurrentIndex(3);
    ui->examsTableView->horizontalHeader()->setDefaultSectionSize((ui->examsTableView->width()-2)/3);

}

void PatientProfileControl::editPatientProfile()
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

void PatientProfileControl::savePatientProfile()
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

void PatientProfileControl::rejectPatientProfile()
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

