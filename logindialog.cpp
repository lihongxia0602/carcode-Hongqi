#include "logindialog.h"
#include "ui_logindialog.h"
#include <QCompleter>
#include<QDebug>
#include <QSqlTableModel>
#include "database.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "registerdialog.h"
LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("登录界面");
    pdb = DataBase::getDatabaseInstance();
    QStringList strings;
    qDebug() <<"allusers:"<< pdb->selectAllUsers();
    QStringList selectAllusers = pdb->selectAllUsers();
    for(QString string:selectAllusers) {
        if(string.compare("administrator") != 0) {
            strings << string;
        }
    }
   // strings <<pdb->selectAllUsers();
    QCompleter *completer = new QCompleter(strings, this);
    ui->comboBox_user->clear();
    ui->comboBox_user->addItems(strings);
   // ui->comboBox_user->setEditable(true);
    ui->comboBox_user->setCompleter(completer);
    ui->comboBox_user->setCurrentText("tester");
    qDebug()<<__FUNCTION__<<"is running !";
    ui->password->setEchoMode(QLineEdit::Password);//密码方式显示文本
    QDialog::setModal( true );//设置模态，让该窗口一直在最前端
    setAttribute(Qt::WA_DeleteOnClose,true);//关闭后释放内存
    ui->password->setFocus();//设置焦点
    ui->checkBox_rem->hide();
    ui->checkBox_auto->hide();
    if(ui->comboBox_user->currentText() == "administrator") {
        ui->modifyButton->setEnabled(false);
        ui->registerButton->setEnabled(true);
    }else {
        qDebug() <<"allusers ----";
        ui->registerButton->setEnabled(false);
        ui->modifyButton->setEnabled(false);
    }
}

void LoginDialog::on_comboBox_user_currentTextChanged(){
    qDebug() << "on_comboBox_user_currentTextChanged" << ui->comboBox_user->currentText();
    if(ui->comboBox_user->currentText() == "administrator") {
        ui->modifyButton->setEnabled(false);
        ui->registerButton->setEnabled(true);
    }else {
        ui->registerButton->setEnabled(false);
        ui->modifyButton->setEnabled(false);
    }
}

void LoginDialog:: on_loginButton_clicked(){
    //1.获取用户名和密码
    qDebug() << "on_loginButton_clicked";
    QString name = ui->comboBox_user->currentText();
    QString pwd = ui->password->text();
    if((!name.isEmpty())&&(!pwd.isEmpty()))
    {
    bool ret1 = pdb->findUsr(name);
    if(ret1)
    {
        qDebug() << "on_loginButton_clicked ret1";
        bool ret2 = pdb->judge(name, pwd);
        if (ret2)
        {
            qDebug() << "on_loginButton_clicked ret2";
            //accept();//关闭窗体，并设置返回值为Accepted
            //登录成功显示主窗口
            MainWindow *mw = new MainWindow(name,pwd);

            //建立连接
           // connect(this, SIGNAL(sendUsrname(QString)), mw, SLOT(recUsrname(QString)));

            //发射信号
           // emit sendUsrname(name);

          //  mw->setAttribute(Qt::WA_DeleteOnClose);
            //this->close();
            this->hide();
            mw->show();
        }
        else
        {
            QMessageBox::information(this, "提示", "登入失败！用户名或密码错误", QMessageBox::Ok);
            ui->password->clear();
            ui->password->setFocus();//设置焦点
        }
    }
    }
    else
    {
        QMessageBox::information(this, "提示", "登入失败！未注册，请先注册", QMessageBox::Ok);
    }
}
void LoginDialog:: on_registerButton_clicked(){
    //RegisterDialog *reDialog = new RegisterDialog();
   // this->hide();
  //  reDialog->show();
    RegisterDialog dlg;
    connect ( &dlg, SIGNAL (send(QString ,QString )),this, SLOT(receive(QString ,QString )) );
    dlg.exec();

}

void LoginDialog::on_modifyButton_clicked(){
    if(ui->password->text() == NULL) {
        QMessageBox::information(this, "提示", "密码不能为空", QMessageBox::Ok);
        return;
    }
     if(pdb->updatePwd(ui->comboBox_user->currentText(),ui->password->text())) {
          QMessageBox::information(this, "提示", "密码修改成功", QMessageBox::Ok);
     } else {
          QMessageBox::information(this, "提示", "密码修改失败", QMessageBox::Ok);
     }

}

void LoginDialog::receive(QString username,QString password){
     qDebug()<< "username:"<< username << "--password:"<<password;
     ui->comboBox_user->addItem(username);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
