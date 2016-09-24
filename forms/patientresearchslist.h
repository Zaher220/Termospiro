#ifndef PATIENTRESEARCHSLIST_H
#define PATIENTRESEARCHSLIST_H

#include <QWidget>

namespace Ui {
class PatientResearchsList;
}

class PatientResearchsList : public QWidget
{
    Q_OBJECT

public:
    explicit PatientResearchsList(QWidget *parent = 0);
    ~PatientResearchsList();

private:
    Ui::PatientResearchsList *ui;
};

#endif // PATIENTRESEARCHSLIST_H
