#include "user.h"

User::User(QString name,
           QString pwd) :
    name_(name), pwd_(pwd)
{

}

QString User::name() const
{
    return name_;
}

void User::setName(const QString &name)
{
    name_ = name;
}
QString User::pwd() const
{
    return pwd_;
}

void User::setPwd(const QString &pwd)
{
    pwd_ = pwd;
}
