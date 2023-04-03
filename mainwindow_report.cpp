#include "mainwindow_report.h"
#include "ui_mainwindow_report.h"

MainWindow_report::MainWindow_report(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow_report)
{
    ui->setupUi(this);
}

MainWindow_report::~MainWindow_report()
{
    delete ui;
}
