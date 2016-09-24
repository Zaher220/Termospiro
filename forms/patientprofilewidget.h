#ifndef PATIENTPROFILEWIDGET_H
#define PATIENTPROFILEWIDGET_H

#include <QWidget>

namespace Ui {
class PatientProfileWidget;
}

class PatientProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientProfileWidget(QWidget *parent = 0);
    ~PatientProfileWidget();

private:
    Ui::PatientProfileWidget *ui;
};

#endif // PATIENTPROFILEWIDGET_H
