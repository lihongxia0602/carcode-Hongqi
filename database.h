#ifndef DATABASE_H
#define DATABASE_H
#include <QSqlDatabase>
#include <QVector>
#include "user.h"
class DataBase
{
public:
    static DataBase *getDatabaseInstance();  //静态的获取数据库对象的函数

    QStringList selectAllUsers();
    //插入一个用户信息
    bool insertUsr(User &usr);

    //根据name查找用户
    bool findUsr(QString name);

    //匹配name和pwd
    bool judge(QString name, QString pwd);

    bool updatePwd(QString name, QString pwd);
    bool deleteAllData();
    bool deleteUserData(QString username);

private:
    DataBase();

    QSqlDatabase db_;
};

#endif // DATABASE_H
