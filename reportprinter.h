#ifndef REPORTPRINTER_H
#define REPORTPRINTER_H

#include <QObject>
#include <QWidget>
#include <QPrinter>
#include <QPrintDialog>
#include "ui_tsprintview.h"
#include <QPainter>
#include "tscurvebuffer.h"

class ReportPrinter : public QWidget
{
    Q_OBJECT
public:
    explicit ReportPrinter(QObject *parent = 0);

    void setCurveBuffer(TSCurveBuffer *value);

signals:

public slots:
    void printReport();
private:
    QWidget wpf;
    TSCurveBuffer *curveBuffer = nullptr;
};

#endif // REPORTPRINTER_H
