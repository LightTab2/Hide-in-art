#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class helpWindow; }
QT_END_NAMESPACE

class HelpWindow : public QWidget
{
    Q_OBJECT

public:
    HelpWindow(QWidget *parent = nullptr);
    ~HelpWindow();

private:
    Ui::helpWindow *ui;
};
