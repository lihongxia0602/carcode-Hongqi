#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "database.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void sendUsrname(QString name);
private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_modifyButton_clicked();
    void on_comboBox_user_currentTextChanged();
    void receive(QString username,QString password);

private:
    Ui::LoginDialog *ui;
    DataBase* pdb = NULL;
};

#endif // LOGINDIALOG_H
