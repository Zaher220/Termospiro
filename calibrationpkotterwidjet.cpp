#include "calibrationpkotterwidjet.h"
#include "ui_calibrationpkotterwidjet.h"

CalibrationPkotterWidjet::CalibrationPkotterWidjet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalibrationPkotterWidjet)
{
    ui->setupUi(this);

    ui->CalibrationPlottingWidget->addGraph();
    ui->CalibrationPlottingWidget->graph()->setPen(QPen(Qt::blue));
    ui->CalibrationPlottingWidget->graph()->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->CalibrationPlottingWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

}

CalibrationPkotterWidjet::~CalibrationPkotterWidjet()
{
    delete ui;
}

void CalibrationPkotterWidjet::appendData(QVector<double> data)
{
    if( data.size() != 3 )
        return;
    m_data.append(data);
    QVector<double> x(m_data.size());

    for(int i = 0; i < x.size(); i++){
        x[i] = i;
    }

    ui->CalibrationPlottingWidget->graph(0)->addData(x,m_data);
    ui->CalibrationPlottingWidget->yAxis->rescale(true);
    ui->CalibrationPlottingWidget->xAxis->rescale(true);
    ui->CalibrationPlottingWidget->replot();
}

void CalibrationPkotterWidjet::reset()
{
    m_data.clear();
    ui->CalibrationPlottingWidget->graph(0)->clearData();
}
