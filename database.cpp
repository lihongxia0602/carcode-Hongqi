#include <QSql>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "database.h"
#include "user.h"
//https://www.cnblogs.com/xia-weiwen/p/6806709.html

DataBase *DataBase::getDatabaseInstance()
{
    static DataBase sqldb;
    return &sqldb;
}

QStringList DataBase::selectAllUsers(){
    QStringList strings;
    QString select_all_sql = "select name from t_usr";
    if(!db_.open()) {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }
    QSqlQuery sql_query(db_);
    sql_query.prepare(select_all_sql);
    if(!sql_query.exec())
    {
        qDebug()<<sql_query.lastError();
    }
    else
    {
        while(sql_query.next())
        {
            QString name = sql_query.value(0).toString();
            qDebug()<<"name:" << name;
            strings << name;
        }
    }

    db_.close();
    return strings;
}

//插入一个用户信息
bool DataBase::insertUsr(User &usr)
{
    QString sql = QString("insert into t_usr values('%1', '%2')")
            .arg(usr.name())
            .arg(usr.pwd());

    // 1.打开数据
    if (!db_.open())
    {
        qDebug() << "open:" << db_.lastError().text();
        db_.close();
        return false;
    }

    // 2.执行sql
    QSqlQuery query(db_);
    if (!query.exec(sql))
    {
        qDebug() << "insert:" << query.lastError().text();
        db_.close();
        return false;
    }

    //关闭数据库
    db_.close();
    return true;

}

bool DataBase::updatePwd(QString name, QString pwd){
    QString sql = QString("select pwd from t_usr where name = :name;");
    if(!db_.open()) {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }

    //执行插入操作
    QSqlQuery query(db_);
    query.prepare(sql);
    query.bindValue(":name",QVariant(name));
    query.exec();

    if(query.next())
    {
        QString password = query.value(0).toString();
        qDebug() << "password:" <<password << "pwd:"<< pwd;
        if(password != pwd) {
             QString update_sql = "update t_usr set pwd = :pwd where name = :name;";
             query.prepare(update_sql);
             query.bindValue(":name", name);
             query.bindValue(":pwd", pwd);
             if(!query.exec())
             {
                 qDebug() << query.lastError();
             }
             else
             {
                 qDebug() << "updated!";
                 db_.close();
                 return true;
             }
        }
    }
    //关闭数据库
    db_.close();
    return false;
}
//根据name查找用户
bool DataBase::findUsr(QString name)
{
    QString sql = QString("select name from t_usr where name = :name;");
    //打开数据库
    if(!db_.open())
    {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }

    //执行插入操作
    QSqlQuery query(db_);
    query.prepare(sql);
    query.bindValue(":name",QVariant(name));
    query.exec();

    //查询成功
    if(query.next())
    {
        if(name == query.value(0).toString()) //用户存在
        {
           db_.close();
           return true;
        }
    }

    //关闭数据库
    db_.close();

    return false;
}


//匹配name和pwd
bool DataBase::judge(QString name, QString pwd)
{
    QString sql = QString("select name, pwd from t_usr where name = :name;");
    //打开数据库
    if(!db_.open())
    {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }

    //执行插入操作
    QSqlQuery query(db_);
    query.prepare(sql);
    query.bindValue(":name",QVariant(name));
    query.exec();

    //匹配密码
    if(query.next())
    {
        if(!(pwd == query.value(1).toString()))
        {
          db_.close();
          return false;
        }
    }

    //关闭数据库
    db_.close();

    return true;
}

bool DataBase::deleteAllData() {
    QString sql = QString("DROP TABLE t_usr");
    //打开数据库
    if(!db_.open())
    {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }

    //执行插入操作
    QSqlQuery query(db_);
    query.prepare(sql);
    if(!query.exec()){
        qDebug() << query.lastError();
    } else {
        qDebug()<<"deleted all datas";
    }
    db_.close();
    return true;
}

bool DataBase::deleteUserData(QString username) {
    QString sql = QString("delete from t_usr where name=:name");
    //打开数据库
    if(!db_.open())
    {
        qDebug() << "open fail" << db_.lastError().text();
        exit(-1);
    }

    //执行插入操作
    QSqlQuery query(db_);
    query.prepare(sql);
    query.bindValue(":name",QVariant(username));
    if(!query.exec()){
        qDebug() << query.lastError();
    }else {
        qDebug()<<"deleted current user:" << username ;
    }
    db_.close();
    return true;
}

DataBase::DataBase()
{
    //先判断连接是否存在
    if(QSqlDatabase::contains("dbConnection"))
    {
        db_ = QSqlDatabase::addDatabase("dbConnection");
    }
    else
    {
        //1.加载数据库驱动
        db_ = QSqlDatabase::addDatabase("QSQLITE", "dbConnection");
        qDebug () << "db_.connectionName():"<< db_.connectionName();//打印连接名称

        //2.设置数据库名
        db_.setDatabaseName("user.db");
        qDebug () << "db_.connectionName():"<< db_.databaseName();
    }

    //3.打开数据库
       if(db_.open())
       {
           qDebug() << "open success";
       }
       else
       {
           qDebug() << "open:" << db_.lastError().text();
           db_.close();
           return;
       }

       //4.操作数据库：执行sql语句(integer类型的主键会自增)
       QString sql = "create table if not exists t_usr(name varchar(50) PRIMARY KEY UNIQUE NOT NULL, pwd varchar(20))";
      // QString sql = "create table t_usr(name VARCHAR(30) PRIMARY KEY UNIQUE NOT NULL,passward VARCHAR(30))";
       QSqlQuery query(db_);
       //query.exec("DROP TABLE t_usr");        //先清空一下表
       if (query.exec(sql))
       {
           qDebug() << "create success.";
       }
       User *user_test = new User("tester", "yishu");
       if(!findUsr(user_test->name())) {
           qDebug() << "!findUsr(user_test->name()";
           insertUsr(*user_test);
       }
       User* user_root = new User("administrator","yishu2022");
       if(!findUsr(user_root->name())) {
           qDebug() << "!findUsr(user_root->name()";
           insertUsr(*user_root);
       }
       db_.close();
}
