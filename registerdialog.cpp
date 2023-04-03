#include "registerdialog.h"
#include "ui_registerdialog.h"
#include <QMessageBox>
RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    pdb = DataBase::getDatabaseInstance();
}

void RegisterDialog::on_confirmButton_clicked(){
    QString name=ui->lineEdit_user_add->text();
    QString pwd=ui->lineEdit_pwd_add->text();
    //******************************************************************
    if((!name.isEmpty())&&(!pwd.isEmpty()))
    {
    User user(name, pwd);
    bool ret = pdb->insertUsr(user);
    if (ret)
    {
        QMessageBox::information(this,
                                 "提示",
                                 "注册成功！",
                                 QMessageBox::Ok);
        //发射信号
        emit send(name,pwd);
    }
    }
    else
    {
        QMessageBox::information(this,
                                 "提示",
                                 "注册失败！",
                                 QMessageBox::Ok);
    }
    this->close();
}
void RegisterDialog::on_cancelButton_clicked(){
     this->close();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}
