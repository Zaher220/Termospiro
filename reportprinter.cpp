#include "reportprinter.h"

/*ReportPrinter::ReportPrinter(QWidget *parent) : QObject(parent)
{

}*/

void ReportPrinter::printReport()
{
    //    qDebug()<<"TSController::printReport";
   /* QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Предварительный просмотр"));

    int endIndex = curveBuffer->getLenght();

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
    prTempOut.setPen(curveBuffer->toutColor);
    prVolume.setPen(QColor(255,0,0));
    int* tinInt = curveBuffer->getTempInInterval();
    int* toutInt = curveBuffer->getTempOutInterval();
    int* volInt = curveBuffer->getVolumeInterval();
    float tempInK = 1;
    float tempOutK = 1;
    float tempInZ = h;
    float tempOutZ = h;
    double tempInAdaptive = (float)myH/(tinInt[1]-tinInt[0]);
    double tempOutAdaptive = (float)myH/(toutInt[1]-toutInt[0]);
    double printerVolumeAdaptive = (float)myH/(volInt[1]-volInt[0]);
    //volumeAdaptive = (float)myH/(volInt[1]-volInt[0]);
    tempInZ = h + ceil((float)(tinInt[1]+tinInt[0]) * m_plotter.getTempInAdaptive()*tempInK/2);
    tempOutZ = h + ceil((float)(toutInt[1]+toutInt[0]) * m_plotter.getTempOutAdaptive()*tempOutK/2);
    float volumeK =1;


    QVector<int> volume = curveBuffer->volumeVector();
    QVector<int> tempIn = curveBuffer->tempInVector();
    QVector<int> tempOut = curveBuffer->tempOutVector();
    i=0;
    int k=ceil((float)curveBuffer->getLenght()/pf.gTempIn->width());
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
    }*/
}

ReportPrinter::ReportPrinter(QObject *parent)
{

}

void ReportPrinter::setCurveBuffer(TSCurveBuffer *value)
{
    curveBuffer = value;
}
