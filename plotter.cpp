#include "plotter.h"

Plotter::Plotter(QObject *parent) : QObject(parent)
{
    connect(&plotingTimer,SIGNAL(timeout()),this,SLOT(plotNow()));
    connect(&cPlotingTimer,SIGNAL(timeout()),this,SLOT(plotCalibration()));
    scaleScroll[0]=1;
    scaleScroll[1]=3;
    scaleScroll[2]=5;
    scaleScroll[3]=7;
    scaleScroll[4]=9;
}

void Plotter::resetPlotting()
{
    if ( pcVolume.isActive() )
        pcVolume.end();
    bcVolume = QPixmap(cW,cH);
    pcVolume.begin(&bcVolume);
}

void Plotter::plotNow()
{
    //qDebug()<<"TSController::plotNow";
    int endIndex = curveBuffer->end();
    if(endIndex == 17999)
        plotingTimer.stop();
    int h = ui->gVolume->height()/2;

    int step = h/10;
    startIndex = endIndex - (W-35);
    if(startIndex < 0) startIndex = 0;
    if(h%10>=5)
    {
        h+=step/2;
    }
    pVolume.fillRect(0,0,W,H,Qt::white);
    pTempIn.fillRect(0,0,W,H,Qt::white);
    pTempOut.fillRect(0,0,W,H,Qt::white);
    int i;
    pVolume.setPen(QColor(225,225,225));
    pTempIn.setPen(QColor(225,225,225));
    pTempOut.setPen(QColor(225,225,225));
    for(i=step;i<h;i+=step)
    {
        pVolume.drawLine(0,h+i,W,h+i);
        pTempIn.drawLine(0,h+i,W,h+i);
        pTempOut.drawLine(0,h+i,W,h+i);
        pVolume.drawLine(0,h-i,W,h-i);
        pTempIn.drawLine(0,h-i,W,h-i);
        pTempOut.drawLine(0,h-i,W,h-i);
    }
    for(i=10;i<W;i+=10)
    {
        pVolume.drawLine(i,0,i,h<<1);
        pTempIn.drawLine(i,0,i,h<<1);
        pTempOut.drawLine(i,0,i,h<<1);
    }
    pVolume.setPen(QColor(0,0,0));
    pTempIn.setPen(QColor(0,0,0));
    pTempOut.setPen(curveBuffer->toutColor);

    pVolume.setPen(QColor(255,0,0));
    int* tinInt = curveBuffer->getTempInInterval();
    int* toutInt = curveBuffer->getTempOutInterval();
    int* volInt = curveBuffer->getVolumeInterval();
    float tempInK = 1;//tempInScaleRate*h;//*abs(tempInInterval[0]-tempInInterval[1])/abs(tinInt[1]-tinInt[0]);
    float tempOutK = 1;//tempOutScaleRate*h;
    float tempInZ = h;// + tempInZerPos*h;
    float tempOutZ = h;// + tempInZerPos*h;
    tempInAdaptive = (float)H/
            (tinInt[1]-tinInt[0]);
    tempOutAdaptive = (float)H/
            (toutInt[1]-toutInt[0]);
    volumeAdaptive = (float)H/(volInt[1]-volInt[0]);
    tempInZ = h + ceil((float)(tinInt[1]+tinInt[0])*tempInAdaptive*tempInK/2);
    tempOutZ = h + ceil((float)(toutInt[1]+toutInt[0])*tempOutAdaptive*tempOutK/2);
    float volumeK = 1;// volumeScaleRate*h;
    int k = 1/horizontalStep;
    i=0;
    QVector<int> volume(curveBuffer->volumeVector());
    QVector<int> tempin(curveBuffer->tempInVector());
    QVector<int> tempout(curveBuffer->tempOutVector());
    for(int j = 0; j < W - 35; j++){
        if( i + startIndex >= k * endIndex )
            break;
        if( i + startIndex + k >= volume.size() )
            break;
        pVolume.drawLine(
                    j,
                    h - volumeK * volumeAdaptive * volume[i + startIndex],
                j + 1,
                h - volumeK * volumeAdaptive * volume[i + startIndex + k]
                );
        pTempIn.drawLine(
                    j,
                    tempInZ-tempInK*tempInAdaptive*tempin[i+startIndex],
                j+1,
                tempInZ-tempInK*tempInAdaptive*tempin[i+startIndex+k]
                );
        pTempOut.drawLine(
                    j,
                    tempOutZ-tempOutK*tempOutAdaptive*tempout[i+startIndex],
                j+1,
                tempOutZ-tempOutK*tempOutAdaptive*tempout[i+startIndex+k]
                );

        i += k;
    }
    ui->gVolume->setPixmap(bVolume);
    ui->gTempIn->setPixmap(bTempIn);
    ui->gTempOut->setPixmap(bTempOut);
    if(recordingStarted)
    {
        ui->horizontalScrollBar->setEnabled(false);
        ui->horizontalScrollBar->setMaximum(startIndex/10);
        ui->horizontalScrollBar->setValue(startIndex/10);
    }
}

void Plotter::plotCalibration()
{
    // qDebug()<<"TSController::plotCalibration";
    int endIndex = curveBuffer->getLenght();
    QVector<int> m_volume(curveBuffer->volumeVector());
    if(endIndex < 1350){
        pcVolume.fillRect(0,0,cW,cH,Qt::white);
        int h = cH/2;
        int step = h/10;
        if(h%10>=5){
            h+=step/2;
        }
        int i;
        pcVolume.setPen(QColor(225,225,225));
        for(i=step;i<h;i+=step){
            pcVolume.drawLine(0,h+i,cW,h+i);
            pcVolume.drawLine(0,h-i,cW,h-i);
        }
        for(i=10;i<cW;i+=10){
            pcVolume.drawLine(i,0,i,cH);
        }
        if(endIndex>50){
            for(int i=endIndex-50;i<endIndex;i++){
                if(abs(m_volume[i])>maxcVol) maxcVol = abs(m_volume[i]);
            }
        }

        float K = (float)(h-10)/maxcVol;
        step = ceil((float)1200/cW);
        pcVolume.setPen(QColor(0,0,0));
        int j=0;
        for(i=0;i<cW-1;i++){
            if(i>=endIndex||j+step>=endIndex)break;
            pcVolume.drawLine(i,h-K*m_volume[j],i+1,h-K*m_volume[j+step]);
            j+=step;
        }
        ui->calibrateVolumeAnimation->setPixmap(bcVolume);
    }
    int k=0;
    if (endIndex!=0)
        k=(1350*100)/endIndex;
    emit changeprogress(k);
    /*else{
        cPlotingTimer.stop();
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
        //m_adc_reader.stopACQ();

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
    }*/

}

void Plotter::startPlottingTimer(int volume)
{
    plotingTimer.start(volume);
}

void Plotter::stopPlottingTimer()
{
    plotingTimer.stop();
}
QWidget *Plotter::getParentWindow() const
{
    return parentWindow;
}

void Plotter::setParentWindow(QWidget *value)
{
    parentWindow = value;
}

double Plotter::getTempOutAdaptive() const
{
    return tempOutAdaptive;
}

void Plotter::setTempOutAdaptive(double value)
{
    tempOutAdaptive = value;
}




double Plotter::getTempInAdaptive() const
{
    return tempInAdaptive;
}

void Plotter::setTempInAdaptive(double value)
{
    tempInAdaptive = value;
}

int *Plotter::getTempOutInterval() const
{
    return tempOutInterval;
}

void Plotter::setTempOutInterval(int *value)
{
    tempOutInterval = value;
}

int *Plotter::getTempInInterval() const
{
    return tempInInterval;
}

void Plotter::setTempInInterval(int *value)
{
    tempInInterval = value;
}

double Plotter::getTempOutScaleRate() const
{
    return tempOutScaleRate;
}

void Plotter::setTempOutScaleRate(double value)
{
    tempOutScaleRate = value;
}

double Plotter::getTempInScaleRate() const
{
    return tempInScaleRate;
}

void Plotter::setTempInScaleRate(double value)
{
    tempInScaleRate = value;
}

double Plotter::getVolumeScaleRate() const
{
    return volumeScaleRate;
}

void Plotter::setVolumeScaleRate(double value)
{
    volumeScaleRate = value;
}

int Plotter::getCW() const
{
    return cW;
}

void Plotter::setCW(int value)
{
    cW = value;
}

int Plotter::getCH() const
{
    return cH;
}

void Plotter::setCH(int value)
{
    cH = value;
}

double Plotter::getHorizontalStep() const
{
    return horizontalStep;
}

void Plotter::setHorizontalStep(double value)
{
    horizontalStep = value;
}

int Plotter::getW() const
{
    return W;
}

void Plotter::setW(int value)
{
    W = value;
}

int Plotter::getMaxcVol() const
{
    return maxcVol;
}

void Plotter::setMaxcVol(int value)
{
    maxcVol = value;
}

void Plotter::startCPlottingTimer(int volume)
{
    cPlotingTimer.start(volume);
}

void Plotter::scaleTempIn(int value)
{
    //qDebug()<<"TSController::scaleTempIn";
    if(value>0)
    {
        ui->tempInScroll->setMinimum((-1)*scaleScroll[value-1]);
        ui->tempInScroll->setMaximum(scaleScroll[value-1]);
        ui->tempInScroll->setValue(0);
    }
    else
    {
        ui->tempInScroll->setMinimum(0);
        ui->tempInScroll->setMaximum(0);
        ui->tempInScroll->setValue(0);
    }
    value*=2;
    if(value<0)
    {
        tempInScaleRate = (-1.0)/value/5000;
    }
    if(value == 0)
    {
        tempInScaleRate = 1.0/5000;
    }
    if(value>0)
    {
        tempInScaleRate = (float)value/5000;
    }
    plotNow();
}

void Plotter::scaleTempOut(int value)
{
    //qDebug()<<"TSController::scaleTempOut";
    if(value>0)
    {
        ui->tempOutScroll->setMinimum((-1)*scaleScroll[value-1]);
        ui->tempOutScroll->setMaximum(scaleScroll[value-1]);
        ui->tempOutScroll->setValue(0);
    }
    else
    {
        ui->tempOutScroll->setMinimum(0);
        ui->tempOutScroll->setMaximum(0);
        ui->tempOutScroll->setValue(0);
    }
    value*=2;
    if(value<0)
    {
        tempOutScaleRate = (-1.0)/value/5000;
    }
    if(value == 0)
    {
        tempOutScaleRate = 1.0/5000;
    }
    if(value>0)
    {
        tempOutScaleRate = (float)value/5000;
    }
    plotNow();
}

void Plotter::scaleVolume(int value)
{
    // qDebug()<<"TSController::scaleVolume";
    value*=2;
    if(value<0)
    {
        volumeScaleRate = (-4.0)/value/5000;
    }
    if(value == 0)
    {
        volumeScaleRate = 4.0/5000;
    }
    if(value>0)
    {
        volumeScaleRate = (float)value*4/5000;
    }
    plotNow();
}

void Plotter::changeTempInScrollValue(int value)
{
    ///qDebug()<<"TSController::changeTempInScrollValue";
    tempInZerPos = (-1)*value;
    plotNow();
}

TSCurveBuffer *Plotter::getCurveBuffer() const
{
    return curveBuffer;
}

void Plotter::setCurveBuffer(TSCurveBuffer *value)
{
    curveBuffer = value;
}

void Plotter::scrollGraphics(int value)
{
    if(curveBuffer->end() != 0)
        curveBuffer->setEnd(W-35+value*horizontalStep*10);
    plotNow();
}

void Plotter::scaleForHorizontal(int value)
{
    //qDebug()<<"TSController::scaleForHorizontal";
    value*=2;
    if(value!=0)
    {
        horizontalStep = (-1.0)/value;
        curveBuffer->setEnd(W-35);
    }
    else
    {
        horizontalStep = 1.0;
        curveBuffer->setEnd(W-35);
    }
    plotNow();
}

Ui::TSView *Plotter::getUi() const
{
    return ui;
}

void Plotter::setUi(Ui::TSView *value)
{
    ui = value;
}

void Plotter::initPaintDevices()
{
    //qDebug()<<"TSController::initPaintDevices";
    H = ui->gVolume->height();
    W = ui->gVolume->width();
    if(pVolume.isActive()) pVolume.end();
    if(pTempIn.isActive()) pTempIn.end();
    if(pTempOut.isActive()) pTempOut.end();
    bVolume = QPixmap(W,H);
    bTempIn = QPixmap(W,H);
    bTempOut = QPixmap(W,H);
    pVolume.begin(&bVolume);
    pTempIn.begin(&bTempIn);
    pTempOut.begin(&bTempOut);
    screenLimit = W;
    startIndex = 0;
}

void Plotter::on_horizontalScrollBar_sliderPressed()
{
    qDebug()<<"Hellow";
}
bool Plotter::getRecordingStarted() const
{
    return recordingStarted;
}

void Plotter::setRecordingStarted(bool value)
{
    recordingStarted = value;
}

void Plotter::stopPlottimgTimer()
{
    cPlotingTimer.stop();
}
