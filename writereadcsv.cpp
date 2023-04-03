#include "writereadcsv.h"
#include <QFile>
writereadcsv::writereadcsv()
{
}

writereadcsv::~writereadcsv() {

}
/*void writereadcsv::writeCSVFile(QString path,const char *str_data){
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

QList<QStringList> writereadcsv::ReadFromCSV(QString PathName){
    QList<QStringList> list;
    return list;
}

void writereadcsv::writeCSVFileSpec(QString path,QList<TestResultEntry> testresultlist){
    QString str_csv;
    str_csv.clear();
    str_csv.append("产品SN");
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
    str_csv.append("待机15V电流");
    str_csv.append(",");
    str_csv.append("静态9V电流");
    str_csv.append(",");
    str_csv.append("静态13V电流");
    str_csv.append(",");
    str_csv.append("静态15V电流");
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
        str_csv.append(entry.result);
        str_csv.append(",");
        str_csv.append(entry.hw);
        str_csv.append(",");
        str_csv.append(entry.sw);
        str_csv.append(",");
        str_csv.append(QString::number(entry.nottofdistance,10));
        str_csv.append(",");
        str_csv.append(QString::number(entry.tofdistance,10));
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_9V,10));
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_13V,10));
        str_csv.append(",");
        str_csv.append(QString::number(entry.normal_current_value_15V,10));
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_9V,'f',3));
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_13V,'f',3));
        str_csv.append(",");
        str_csv.append(QString::number(entry.idle_current_value_15V,'f',3));
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_9V,'f',3));
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_13V,'f',3));
        str_csv.append(",");
        str_csv.append(QString::number(entry.sleep_current_value_15V,'f',3));
        str_csv.append(",");
        str_csv.append(entry.diamater);
        str_csv.append(",");
        str_csv.append(entry.brightness);
        str_csv.append(",");
        str_csv.append(entry.distortion);
        str_csv.append("\n");
        writeCSVFile(path,str_csv.toUtf8());
    }
}*/
