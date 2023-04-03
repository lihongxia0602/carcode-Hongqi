#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "database.h"
namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = 0);
    ~RegisterDialog();

signals:
    void send(QString name,QString pwd);

private slots:
    void on_confirmButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::RegisterDialog *ui;
    DataBase* pdb = NULL;
};

#endif // REGISTERDIALOG_H
