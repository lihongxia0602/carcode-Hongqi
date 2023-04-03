#ifndef READWRITECSV_H
#define READWRITECSV_H

#include "datatypedef.h"
class readwritecsv
{
public:
    readwritecsv();
    ~readwritecsv();

public:
     void writeCSVFile(QString path,const char *str_data);
     QList<QStringList> ReadFromCSV(QString PathName);
     void writeCSVFileSpec(QString path,QList<TestResultEntry> testresultlist);
};

#endif // READWRITECSV_H
