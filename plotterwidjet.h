#ifndef PLOTTERWIDJET_H
#define PLOTTERWIDJET_H

#include <QWidget>
#include "qcustomplot.h"
//#include "ui_plotterwidjet.h"
namespace Ui {
class PlotterWidjet;
}

class PlotterWidjet : public QWidget
{
    Q_OBJECT

public:
    explicit PlotterWidjet(QWidget *parent = 0);
    ~PlotterWidjet();
    void appendData(QVector<QVector<double > > data);
    void reset();
private slots:
    void on_PlotHorizontalScrollBar_valueChanged(int value);
    void xAxisChanged(QCPRange range);
private:
    void initGraphs(QCustomPlot *graph, QPen pen);
    Ui::PlotterWidjet *ui;
    QVector<double> m_vol, m_tin, m_tout;
    int N = 10;//точек для показа
    int K = 0;//всего точек на графике
    double l = 0;//Коэффициент трансформации

};

#endif // PLOTTERWIDJET_H
