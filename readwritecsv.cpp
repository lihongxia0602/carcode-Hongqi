#include "readwritecsv.h"
#include <QFile>
#include <QDebug>
readwritecsv::readwritecsv()
{

}
readwritecsv::~readwritecsv() {

}
void readwritecsv::writeCSVFile(QString path,const char *str_data){
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)){//杩藉姞鍐欏叆 娣诲姞缁撴潫绗r\n
       //  QMessageBox::warning(this,tr("错误"),tr("打开文件失败,数据保存失败"));
        return ;
    }else{
        file.write(str_data);
        file.flush();
        file.seek(0);
    }
    file.close();
}

QList<QStringList> readwritecsv::ReadFromCSV(QString PathName){
    QList<QStringList> list_ret;
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Excel file"), qApp->applicationDirPath (),
          //                                             tr("Files (*.csv)"));
    QFile file(PathName);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Open Excel file failed!";
        return list_ret;
    }
    QStringList list;
    list.clear();
    QTextStream in(&file);
    in.setCodec("utf-8");      //这行的目的是支持读取中文信息
    in.seek(0);
    qDebug() << "------in.atEnd():" << in.atEnd();
    //遍历行
    for(int i = 0; !in.atEnd(); i++)
    {
        //qDebug() << "i:" << i;
        QString fileLine = in.readLine();
        list = fileLine.split(",", QString::SkipEmptyParts);    //根据","开分隔开每行的列
        list_ret.append(list);

    }
        //qDebug() << list_ret;
   // qDebug()<< list_ret.size();
    bool isContain = false;
    for(int j =0;j< list_ret.size(); j++) {
        qDebug() <<"====="<< list_ret.at(j);
        if(list_ret.at(j).contains("产品SN")) {
            qDebug("=============containsn");
            isContain = true;
            break;
        }
    }
    if(list_ret.size() == 0 || !isContain) {
        QString str_csv;
        str_csv.clear();
        str_csv.append("产品SN");
        str_csv.append(",");
        str_csv.append("测试时间"),
        str_csv.append(",");
        str_csv.append("测试结果");
        str_csv.append(",");
        str_csv.append("硬件版本号");
        str_csv.append(",");
        str_csv.append("软件版本号");
        str_csv.append(",");
        str_csv.append("未踩踏距离");
        str_csv.append(",");
        str_csv.append("踩踏距离");
        str_csv.append(",");
        str_csv.append("工作9V电流");
        str_csv.append(",");
        str_csv.append("工作13V电流");
        str_csv.append(",");
        str_csv.append("工作15V电流");
        str_csv.append(",");
        str_csv.append("待机9V电流");
        str_csv.append(",");
        str_csv.append("待机13V电流");
        str_csv.append(",");
        str_csv.append("待机16V电流");
        str_csv.append(",");
        str_csv.append("静态9V电流");
        str_csv.append(",");
        str_csv.append("静态13V电流");
        str_csv.append(",");
        str_csv.append("静态16V电流");
        str_csv.append(",");
        str_csv.append("投影直径");
        str_csv.append(",");
        str_csv.append("投影照度");
        str_csv.append(",");
        str_csv.append("投影畸变\n");
        //writeCSVFile(str_csv.toUtf8());
        file.write(str_csv.toUtf8());
        file.flush();
        file.seek(0);
    }

    file.close();
    return list_ret;
}

void readwritecsv::writeCSVFileSpec(QString path,QList<TestResultEntry> testresultlist){
    QString str_csv;
    str_csv.clear();
    str_csv.append("产品SN");
    str_csv.append(",");
    str_csv.append("测试时间"),
    str_csv.append(",");
    str_csv.append("测试结果");
    str_csv.append(",");
    str_csv.append("硬件版本号");
    str_csv.append(",");
    str_csv.append("软件版本号");
    str_csv.append(",");
    str_csv.append("未踩踏距离");
    str_csv.append(",");
    str_csv.append("踩踏距离");
    str_csv.append(",");
    str_csv.append("工作9V电流");
    str_csv.append(",");
    str_csv.append("工作13V电流");
    str_csv.append(",");
    str_csv.append("工作16V电流");
    str_csv.append(",");
    str_csv.append("待机9V电流");
    str_csv.append(",");
    str_csv.append("待机13V电流");
    str_csv.append(",");
    str_csv.append("待机16V电流");
    str_csv.append(",");
    str_csv.append("静态9V电流");
    str_csv.append(",");
    str_csv.append("静态13V电流");
    str_csv.append(",");
    str_csv.append("静态16V电流");
    str_csv.append(",");
    str_csv.append("投影直径");
    str_csv.append(",");
    str_csv.append("投影照度");
    str_csv.append(",");
    str_csv.append("投影畸变\n");
    writeCSVFile(path,str_csv.toUtf8());
    for(TestResultEntry entry:testresultlist) {
        QString str_csv;
        str_csv.clear();
        str_csv.append(entry.SN);
        str_csv.append(",");
        str_csv.append(entry.dateTime);
        str_csv.append(",");
        str_csv.append(entry.result);
        str_csv.append(",");
        str_csv.append(entry.hw);
        str_csv.append(",");
        str_csv.append(entry.sw);
        str_csv.append(",");
        str_csv.append(QString::number(entry.nottofdistance,10)).append("cm");
        str_csv.append(",");
        str_csv.append(QString::number(entry.tofdistance,10)).append("cm");
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_9V,10)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_13V,10)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_15V,10)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_9V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_13V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_15V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_9V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_13V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_15V,'f',3)).append("mA");
        str_csv.append(",");
        str_csv.append(entry.diamater).append("mm");
        str_csv.append(",");
        str_csv.append(entry.brightness).append("lux");
        str_csv.append(",");
        str_csv.append(entry.distortion).append("mm");
        str_csv.append("\n");
        writeCSVFile(path,str_csv.toUtf8());
    }
}
