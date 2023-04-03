#ifndef READWRITEXML_H
#define READWRITEXML_H

#include <QDomComment>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <datatypedef.h>

namespace Ui {
class ReadWriteXml;
}

class ReadWriteXml : public QObject
{
    Q_OBJECT

public:
    explicit ReadWriteXml(QObject *parent = nullptr);
    ~ReadWriteXml();

public:
    /**
     * @brief readXml   读取指定文件
     */
    void readXml(QString fileNamePath);

    /**
     * @brief writeXml  写入指定文件
     */
    void writeXml(QString filepath,int totalTestNum, int totalPassNum,QList<TestResultEntry> list);
    void writeXml(QString filepath,TestResultEntry entry);
    void updateSettingXml(QString fileNamePath, ParamSettingEntry entry);
    QString getProjectName();
    QString getTotalTestNum();
    QString getTotalPassNum();
    QStringList getProjectList();
    QString getHwVersion();
    QString getSwVersion();
    QString getProductNum();
    QString getTofDistance();
    QString getNotTofDistance();
    QString get9VNormalMinCur();
    QString get9VNormalMaxCur();
    QString get13VNormalMaxCur();
    QString get13VNormalMinCur();
    QString get15VNormalMinCur();
    QString get15VNormalMaxCur();
    QString get9VSleepMinCur();
    QString get9VSleepMaxCur();
    QString get13VSleepMaxCur();
    QString get13VSleepMinCur();
    QString get15VSleepMinCur();
    QString get15VSleepMaxCur();
    QString get9VIdleMinCur();
    QString get9VIdleMaxCur();
    QString get13VIdleMaxCur();
    QString get13VIdleMinCur();
    QString get15VIdleMinCur();
    QString get15VIdleMaxCur();
    QString getFloatVol();
private:
    QFile m_file;
    QString mTotalTestNum;
    QString mTotalPassNum;
    QStringList mProjectList;
    QString mProjectName;
    QString mHwVersion;
    QString mSwVersion;
    QString mProductNum;
    QString mTofDistance;
    QString mNotTofDistance;
    QString m9VNormalMinCur;
    QString m9VNormalMaxCur;
    QString m13VNormalMaxCur;
    QString m13VNormalMinCur;
    QString m15VNormalMinCur;
    QString m15VNormalMaxCur;
    QString m9VSleepMinCur;
    QString m9VSleepMaxCur;
    QString m13VSleepMaxCur;
    QString m13VSleepMinCur;
    QString m15VSleepMinCur;
    QString m15VSleepMaxCur;
    QString m9VIdleMinCur;
    QString m9VIdleMaxCur;
    QString m13VIdleMaxCur;
    QString m13VIdleMinCur;
    QString m15VIdleMinCur;
    QString m15VIdleMaxCur;
    QString mFloatVol;

};

#endif // READWRITEXML_H
