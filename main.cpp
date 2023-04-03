#include "mainwindow.h"
//#include "mainwindow_report.h"
#include <QApplication>
#include "logindialog.h"
#include "autorun.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
   // MainWindow w;
   // Platform::autoRun();
   // w.show();
   // a.installNativeEventFilter(&w);
    LoginDialog login;
    login.show();
   // a.installNativeEventFilter(&w);
    return a.exec();
  //  LoginForm login;
   // if (login.exec() == QDialog::Accepted)//调用login.exec()，阻塞主控制流，直到完成返回
   // {
     //  w.show();
     //  a.installNativeEventFilter(&w);
     //  return a.exec();
   // } else {
      //  return 0;
    //}

}
