#include "ccd_serialport.h"
#include "datatypedef.h"
Ccd_SerialPort::Ccd_SerialPort(int portId, QString port,QObject *parent): QObject(parent)
{
    my_thread = new QThread();
    m_portId = portId;
    show_fun_id();
    show_slots_id();
    m_SerialPort = new QSerialPort();
    init_port(port);
    cun=0;
    this->moveToThread(my_thread);
    m_SerialPort->moveToThread(my_thread);
    my_thread->start();//
    qDebug()<<portId <<"in main thread";
    mStartTest = false;
}

Ccd_SerialPort::~Ccd_SerialPort()
{
    cun=0;
}
void Ccd_SerialPort::show_fun_id()
{
    qDebug()<<"fun_id is:"<<QThread::currentThreadId();
}
void Ccd_SerialPort::show_slots_id()
{
    qDebug()<<"ccd slots_id is:"<<QThread::currentThreadId();
    //show_fun_id();
    if(!dataReaded) {
        qDebug()<<"ccd slots_id is:"<<dataReaded;
        emit exception_data(COM1,"没有收到CCD串口消息");
        if(m_readTimer != NULL) {
            m_readTimer->stop();
        }

    }
}

void Ccd_SerialPort::init_port(QString port)
{
    m_SerialPort->setPortName(port);
    m_SerialPort->setBaudRate(19200);//波特率
    m_SerialPort->setDataBits(QSerialPort::Data8);//数据位
    m_SerialPort->setParity(QSerialPort::NoParity);//奇偶校验
    m_SerialPort->setStopBits(QSerialPort::OneStop);//停止位
     m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);//控制位

    if(!m_SerialPort->open(QIODevice::ReadWrite))//读写方式打开
    {
         qDebug()<<"open port "<<m_portId <<" failed!"<<endl;
         mPortOpened = false;
    }
    else
    {
        qDebug()<<"Port "<<m_portId<<" have been opened.";
        mPortOpened = true;
    }
    m_readTimer = new QTimer(this);
   // m_readTimer->start(100); /*开启定时器，并且每100ms后询问一次串口。定时的时间一到，马上产生timeout（）信号，继续执行自定义槽函数readMyCom() */
    //connect(m_readTimer,SIGNAL(timeout()),this,SLOT(read_data()));
    connect(m_SerialPort,SIGNAL(readyRead()),this,SLOT(read_data()));//有数据就读
    connect(m_readTimer,SIGNAL(timeout()),this,SLOT(show_slots_id()));
    //connect(this,SIGNAL(thread_sig()),this,SLOT(show_slots_id()));
}

bool Ccd_SerialPort::checkPortOpen(){
    return mPortOpened;
}
QSerialPort * Ccd_SerialPort:: getSerialPort() {
    return m_SerialPort;
}

void Ccd_SerialPort::closePort(int portId)
{
    if(portId==m_portId)
    {
        qDebug() << "Ccd_SerialPort closePort:" << portId;
        m_readTimer->stop();
        delete m_readTimer;
        m_readTimer = NULL;
        m_SerialPort->close();
        m_SerialPort->deleteLater();
        m_SerialPort = NULL;

        my_thread->exit();
        QThread::msleep(500);
        my_thread->deleteLater();
        my_thread = NULL;
        mPortOpened = false;
        dataReaded = false;
    }
}

void Ccd_SerialPort::read_data()
{
    QByteArray buff ;

    qint64 buffsize = m_SerialPort->size();//获取缓冲区数据的大小

   // qDebug()<<"portId: "<<m_portId<<" buffsize: "<<buffsize;
    cun++;
    if(buffsize > 0)
    {
        dataReaded = true;
        m_readTimer->stop();
        buff = m_SerialPort->readAll();
        emit receive_string_data(m_portId,buff);
        if(buffsize == 26) {
        //QString str_temp = buff.toHex().toUpper();
       // QString str_show;
        //for(int i = 0; i<str_temp.length (); i+=2)
       // {
        //    str_show += str_temp.mid (i,2);
        //    str_show += " ";
       // }
       // qDebug() <<"receive ccd" << str_show;
       // buff.clear();
       // QStringList list = str_show.split(" ");
       // QString string_flag = list.at(1);
        //bool ok;
       // int flag= string_flag.toInt(&ok, 16);
       // qDebug() <<"receive ccd flag:" << flag;
        //emit receive_data(flag);
        parseReadData(buff);
    }else if(buffsize < 26) {
        if(mCommandException) {
            mByteArray.append(buff);
            if(mByteArray.size() == 26 ) {
                mCommandException = false;
                parseReadData(mByteArray);
                mByteArray.clear();
            }
        }
        /*QString str_temp = buff.toHex().toUpper();
        QString str_show;
        for(int i = 0; i<str_temp.length (); i+=2)
        {
            str_show += str_temp.mid (i,2);
            str_show += " ";
        }
        qDebug()<<"==str_show" << str_show <<"===buffsize: "<<buffsize;
        QStringList list = str_show.split(" ");
        bool ok;
        //Frame head
        QString st_frame_head= list.at(0);
        uint8_t int_frame_head= st_frame_head.toInt(&ok, 16);*/
        if(buff.startsWith("%")) {
        //if(int_frame_head == enFrame_head::AR_KEY_RECEIVE_HEAD) {
            mByteArray.append(buff);
            mCommandException = true;
        }
     }
   }
}

void Ccd_SerialPort::parseReadData(QByteArray buff){
    //00 01 00 00
    //01 00 00 0A 02 1B  10.539
    //01 00 47 EB 02 08  18411.520
    //01 00 00 03 01 F8   3.504
    //02 63
    QString str_temp = buff.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
    qDebug() <<"receive ccd" << str_show;
    emit receive_string_data(m_portId,buff);
    buff.clear();
    //QString str_show = "25 00 01 00 00 02 00 00 0A 02 1B 02 00 47 EB 02 08 02 00 00 03 01 F8 02 63 23";
    QStringList list = str_show.split(" ");
    QString string_flag = list.at(2);
    bool ok;
    int flag= string_flag.toInt(&ok, 16);
    qDebug() <<"receive ccd flag:" << flag;
    uint32_t int_diameter_integer= BYTE_TO_UINT32(00,(list.at(6).toInt(&ok, 16)), (list.at(7).toInt(&ok, 16)), ((list.at(8).toInt(&ok, 16))));
    uint16_t int_diameter_decimal = BYTE_TO_UINT16((list.at(9).toInt(&ok, 16)), (list.at(10).toInt(&ok, 16)));
    qDebug() <<"receive ccd flag:" << int_diameter_integer << "." << int_diameter_decimal;

    QString diameter = "";
    switch(list.at(5).toInt(&ok, 16)) {
    case 01:
    {
        if(int_diameter_decimal >0 && int_diameter_decimal <10) {
            diameter.append(QString::number(int_diameter_integer)).append(".00").append(QString::number(int_diameter_decimal));
        }else if(int_diameter_decimal >=10 && int_diameter_decimal < 100) {
            diameter.append(QString::number(int_diameter_integer)).append(".0").append(QString::number(int_diameter_decimal));
        }else {
            diameter.append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
        }

    }
        break;
    case 02:
    {
        if(int_diameter_decimal >0 && int_diameter_decimal <10) {
            diameter.append("-").append(QString::number(int_diameter_integer)).append(".00").append(QString::number(int_diameter_decimal));
        }else if(int_diameter_decimal >=10 && int_diameter_decimal < 100) {
            diameter.append("-").append(QString::number(int_diameter_integer)).append(".0").append(QString::number(int_diameter_decimal));
        }else{
            diameter.append("-").append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
        }

    }
        break;
    case 03:
    {
        if(int_diameter_decimal >0 && int_diameter_decimal <10) {
            diameter.append(QString::number(int_diameter_integer)).append(".00").append(QString::number(int_diameter_decimal));
        }else if(int_diameter_decimal >=10 && int_diameter_decimal < 100) {
            diameter.append(QString::number(int_diameter_integer)).append(".0").append(QString::number(int_diameter_decimal));
        }else {
            diameter.append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
        }

    }
        break;
    }
    qDebug()<<diameter;

    uint32_t int_brightness_integer= BYTE_TO_UINT32(00,(list.at(12).toInt(&ok, 16)), (list.at(13).toInt(&ok, 16)), ((list.at(14).toInt(&ok, 16))));
    uint16_t int_brightness_decimal = BYTE_TO_UINT16((list.at(15).toInt(&ok, 16)), (list.at(16).toInt(&ok, 16)));
    qDebug() <<"receive ccd flag:" << int_brightness_integer << "." << int_brightness_decimal;

    QString brightness = "";
    switch(list.at(11).toInt(&ok, 16)) {
    case 01:
    {
        if(int_brightness_decimal >0 && int_brightness_decimal < 10) {
            brightness.append(QString::number(int_brightness_integer)).append(".00").append(QString::number(int_brightness_decimal));
        }else if(int_brightness_decimal >= 10 && int_brightness_decimal < 100) {
            brightness.append(QString::number(int_brightness_integer)).append(".0").append(QString::number(int_brightness_decimal));
        }else {
            brightness.append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
        }

    }
        break;
    case 02:
    {
        if(int_brightness_decimal >0 && int_brightness_decimal < 10) {
            brightness.append("-").append(QString::number(int_brightness_integer)).append(".00").append(QString::number(int_brightness_decimal));
        }else if(int_brightness_decimal >= 10 && int_brightness_decimal < 100) {
            brightness.append("-").append(QString::number(int_brightness_integer)).append(".0").append(QString::number(int_brightness_decimal));
        }else {
            brightness.append("-").append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
        }
    }
        break;
    case 03:
    {
        if(int_brightness_decimal >0 && int_brightness_decimal < 10) {
            brightness.append(QString::number(int_brightness_integer)).append(".00").append(QString::number(int_brightness_decimal));
        }else if(int_brightness_decimal >= 10 && int_brightness_decimal < 100) {
            brightness.append(QString::number(int_brightness_integer)).append(".0").append(QString::number(int_brightness_decimal));
        }else {
            brightness.append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
        }

    }
        break;
    }
    qDebug()<<brightness;
    uint32_t int_distortion_integer= BYTE_TO_UINT32(00,(list.at(18).toInt(&ok, 16)), (list.at(19).toInt(&ok, 16)), ((list.at(20).toInt(&ok, 16))));
    uint16_t int_distortion_decimal = BYTE_TO_UINT16((list.at(21).toInt(&ok, 16)), (list.at(22).toInt(&ok, 16)));
    qDebug() <<"receive ccd flag:" << int_distortion_integer << "." << int_distortion_decimal;

    QString distortion = "";
    switch(list.at(17).toInt(&ok, 16)) {
    case 01:
    {
        if(int_distortion_decimal >0 && int_distortion_decimal < 10) {
            distortion.append(QString::number(int_distortion_integer)).append(".00").append(QString::number(int_distortion_decimal));
        }else if(int_distortion_decimal >= 10 && int_distortion_decimal <100) {
            distortion.append(QString::number(int_distortion_integer)).append(".0").append(QString::number(int_distortion_decimal));
        }else {
            distortion.append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
        }
    }
        break;
    case 02:
    {
        if(int_distortion_decimal >0 && int_distortion_decimal < 10) {
            distortion.append("-").append(QString::number(int_distortion_integer)).append(".00").append(QString::number(int_distortion_decimal));
        }else if(int_distortion_decimal >= 10 && int_distortion_decimal <100) {
            distortion.append("-").append(QString::number(int_distortion_integer)).append(".0").append(QString::number(int_distortion_decimal));
        }else {
            distortion.append("-").append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
        }
    }
        break;
    case 03:
    {
        if(int_distortion_decimal >0 && int_distortion_decimal < 10) {
            distortion.append(QString::number(int_distortion_integer)).append(".00").append(QString::number(int_distortion_decimal));
        }else if(int_distortion_decimal >= 10 && int_distortion_decimal <100) {
            distortion.append(QString::number(int_distortion_integer)).append(".0").append(QString::number(int_distortion_decimal));
        }else {
            distortion.append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
        }

    }
        break;
    }
    qDebug()<<distortion;
    emit receive_data(flag,diameter,brightness,distortion);


}
 void Ccd_SerialPort::write_data(int portId,QByteArray buff)
 {
     initTestCaseData();
     if(portId==m_portId)
     {
         m_readTimer->start(readTimeout);
         qDebug()<<"write ccd data: "<< buff;
         m_SerialPort->write(buff,4);
         mCcdCommand = buff;
     }
    // emit thread_sig();
 }

 void Ccd_SerialPort::initTestCaseData(){
     dataReaded = false;
     mCommandException = false;
     mByteArray.clear();
 }
