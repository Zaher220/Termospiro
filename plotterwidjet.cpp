#include "plotterwidjet.h"
#include "ui_plotterwidjet.h"

PlotterWidjet::PlotterWidjet(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotterWidjet)
{
    ui->setupUi(this);
    // The following plot setup is mostly taken from the plot demos:

    initGraphs(ui->PlotWidjet_1, QPen(Qt::blue));
    initGraphs(ui->PlotWidjet_2, QPen(Qt::red));
    initGraphs(ui->PlotWidjet_3, QPen(Qt::green));
    connect(ui->PlotWidjet_1->xAxis, SIGNAL(rangeChanged(QCPRange)), this, SLOT(xAxisChanged(QCPRange)));

    ui->PlotWidjet_1->axisRect()->setupFullAxesBox(true);
    ui->PlotWidjet_1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->PlotWidjet_2->axisRect()->setupFullAxesBox(true);
    ui->PlotWidjet_2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    ui->PlotWidjet_3->axisRect()->setupFullAxesBox(true);
    ui->PlotWidjet_3->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    /* ui->PlotWidjet_1->addGraph();
    ui->PlotWidjet_1->graph()->setPen(QPen(Qt::blue));
    ui->PlotWidjet_1->graph()->setBrush(QBrush(QColor(0, 0, 255, 20)));
    ui->PlotWidjet_1->addGraph();
    ui->PlotWidjet_1->graph()->setPen(QPen(Qt::red));
    QVector<double> x(500), y0(500), y1(500);
    for (int i=0; i<500; ++i)
    {
      x[i] = (i/499.0-0.5)*10;
      y0[i] = qExp(-x[i]*x[i]*0.25)*qSin(x[i]*5)*5;
      y1[i] = qExp(-x[i]*x[i]*0.25)*5;
    }
    ui->PlotWidjet_1->graph(0)->setData(x, y0);
    ui->PlotWidjet_1->graph(1)->setData(x, y1);
    ui->PlotWidjet_1->axisRect()->setupFullAxesBox(true);
    ui->PlotWidjet_1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);*/
}

PlotterWidjet::~PlotterWidjet()
{
    delete ui;
}

void PlotterWidjet::on_PlotHorizontalScrollBar_valueChanged(int value)
{
    //if (  (qAbs(ui->PlotWidjet_1->xAxis->range().center() - value) / K > l  )){
        ui->PlotWidjet_1->xAxis->setRange(value , ui->PlotWidjet_1->xAxis->range().size(), Qt::AlignCenter);
        ui->PlotWidjet_1->replot();

        ui->PlotWidjet_2->xAxis->setRange(value , ui->PlotWidjet_2->xAxis->range().size(), Qt::AlignCenter);
        ui->PlotWidjet_2->replot();

        ui->PlotWidjet_3->xAxis->setRange(value , ui->PlotWidjet_3->xAxis->range().size(), Qt::AlignCenter);
        ui->PlotWidjet_3->replot();
    //}
}

void PlotterWidjet::xAxisChanged(QCPRange range)
{
    ui->PlotHorizontalScrollBar->setValue(range.center());
    //ui->PlotHorizontalScrollBar->setValue(qRound(range.center()/l)); // adjust position of scroll bar slider
    //ui->PlotHorizontalScrollBar->setPageStep(qRound(range.size()/l)); // adjust size of scroll bar slider
    //qDebug()<<"qRound(range.center()*l)"<<qRound(range.center()/l);
}
QVector<double > convertToDoubleVector(QVector<int> data){
    QVector<double > result;
    for(int i=0;i<data.size();i++){
        result.push_back(static_cast<double>(data.at(i)));
    }
    return result;
}

void PlotterWidjet::setFullPatientGrapgicsData(VTT_Data data)
{
    this->reset();
    QVector<QVector<double> > graph;
    graph.push_back(convertToDoubleVector(data.volume));

    graph.push_back(convertToDoubleVector(data.tempin));

    graph.push_back(convertToDoubleVector(data.tempout));
    this->appendData(graph);
}

void PlotterWidjet::appendData(QVector<QVector<double> > data)
{
    if( data.size() != 3 )
        return;
    
    m_vol.append(data[0]);
    m_tin.append(data[1]);
    m_tout.append(data[2]);

    K = m_vol.size();
    l = (double)N/K;
    qDebug()<<"l="<<l;

    QVector<double> x(m_vol.size());

    for(int i = 0; i < x.size(); i++){
        x[i] = i;
    }

    ui->PlotWidjet_1->graph(0)->addData(x,m_vol);
    ui->PlotWidjet_2->graph(0)->addData(x,m_tin);
    ui->PlotWidjet_3->graph(0)->addData(x,m_tout);

    qDebug()<<"m_vol.size()="<<m_vol.size();

    ui->PlotWidjet_1->yAxis->rescale(true);
    ui->PlotWidjet_2->yAxis->rescale(true);
    ui->PlotWidjet_3->yAxis->rescale(true);

    ui->PlotWidjet_1->xAxis->setRange(m_vol.size() - N, m_vol.size());
    ui->PlotWidjet_2->xAxis->setRange(m_vol.size() - N, m_vol.size());
    ui->PlotWidjet_3->xAxis->setRange(m_vol.size() - N, m_vol.size());

    ui->PlotHorizontalScrollBar->setRange(0, m_vol.size());
    qDebug()<<"ui->PlotHorizontalScrollBar min"<<ui->PlotHorizontalScrollBar->minimum();
qDebug()<<"ui->PlotHorizontalScrollBar max"<<ui->PlotHorizontalScrollBar->maximum();
qDebug()<<"ui->PlotHorizontalScrollBar value"<<ui->PlotHorizontalScrollBar->value();

    ui->PlotWidjet_1->replot();
    ui->PlotWidjet_2->replot();
    ui->PlotWidjet_3->replot();
}

void PlotterWidjet::reset()
{
    m_vol.clear();
    m_tin.clear();
    m_tout.clear();
    ui->PlotWidjet_1->graph(0)->clearData();
    ui->PlotWidjet_2->graph(0)->clearData();
    ui->PlotWidjet_3->graph(0)->clearData();
}

void PlotterWidjet::initGraphs(QCustomPlot *graph, QPen pen)
{
    graph->addGraph();
    graph->graph()->setPen(pen);
    graph->graph()->setBrush(QBrush(QColor(0, 0, 255, 20)));
    graph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}
