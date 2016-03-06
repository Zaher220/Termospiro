#ifndef CALIBRATIONPKOTTERWIDJET_H
#define CALIBRATIONPKOTTERWIDJET_H

#include <QWidget>
#include "qcustomplot.h"

namespace Ui {
class CalibrationPkotterWidjet;
}

class CalibrationPkotterWidjet : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrationPkotterWidjet(QWidget *parent = 0);
    ~CalibrationPkotterWidjet();
public slots:
    void appendData(QVector<double> data);
    void reset();
private:
    Ui::CalibrationPkotterWidjet *ui;
    QVector<double> m_data;
};

#endif // CALIBRATIONPKOTTERWIDJET_H
