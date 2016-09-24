#ifndef PATIENTSLISTWIDGET_H
#define PATIENTSLISTWIDGET_H

#include <QWidget>

namespace Ui {
class PatientsListWidget;
}

class PatientsListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientsListWidget(QWidget *parent = 0);
    ~PatientsListWidget();

private:
    Ui::PatientsListWidget *ui;
};

#endif // PATIENTSLISTWIDGET_H
