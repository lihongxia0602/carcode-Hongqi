#ifndef MAINWINDOW_REPORT_H
#define MAINWINDOW_REPORT_H

#include <QMainWindow>

namespace Ui {
class MainWindow_report;
}

class MainWindow_report : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow_report(QWidget *parent = nullptr);
    ~MainWindow_report();

private:
    Ui::MainWindow_report *ui;
};

#endif // MAINWINDOW_REPORT_H
