#ifndef USER_H
#define USER_H
#include <QString>
class User
{
public:
    User(QString name="", QString pwd="");

    QString name() const;
    void setName(const QString &name);

    QString pwd() const;
    void setPwd(const QString &pwd);

public:
    QString name_;
    QString pwd_;
};

#endif // USER_H
