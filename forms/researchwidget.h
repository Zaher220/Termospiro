#ifndef RESEARCHWIDGET_H
#define RESEARCHWIDGET_H

#include <QWidget>

namespace Ui {
class ResearchWidget;
}

class ResearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResearchWidget(QWidget *parent = 0);
    ~ResearchWidget();

private:
    Ui::ResearchWidget *ui;
};

#endif // RESEARCHWIDGET_H
