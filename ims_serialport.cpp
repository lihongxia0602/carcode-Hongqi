#include "ims_serialport.h"
#include "datatypedef.h"
Ims_SerialPort::Ims_SerialPort(int portId, QString port,QObject *parent): QObject(parent)
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

Ims_SerialPort::~Ims_SerialPort()
{
    cun=0;
}
void Ims_SerialPort::show_fun_id()
{
    qDebug()<<"fun_id is:"<<QThread::currentThreadId();
}
void Ims_SerialPort::show_slots_id()
{
    qDebug()<<"Ims slots_id is:"<<QThread::currentThreadId();
    //show_fun_id();
    //static int time = 0;
    if(!dataReaded) {
       // if(time < 2) {
        //    time ++;
       // } else {
            if(m_readTimer != NULL) {
                m_readTimer->stop();
            }
            if(mRequestState == IMS_REQUEST_BARCODE) {
                QString info = "没有收到IMS请求条码消息";
                //emit send_exception_data(COM3,info);
                emit send_exception_data(COM3,info.toUtf8());
            }else if(mRequestState == IMS_SEND_TEST_RESULT) {
                QString info = "没有收到IMS测试结果反馈消息";
                emit send_exception_data(COM3,"没有收到IMS测试结果反馈消息");
            }

           // time =0;
       // }
    }
}

void Ims_SerialPort::init_port(QString port)
{
    m_SerialPort->setPortName(port);
    m_SerialPort->setBaudRate(115200);//波特率
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

bool Ims_SerialPort::checkPortOpen(){
    return mPortOpened;
}
QSerialPort * Ims_SerialPort:: getSerialPort() {
    return m_SerialPort;
}

void Ims_SerialPort::closePort(int portId)
{
    if(portId==m_portId)
    {
        qDebug() << "IMS_SerialPort closePort:" << portId;
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

void Ims_SerialPort::read_data()
{
    QByteArray buff ;

    qint64 buffsize = m_SerialPort->size();//获取缓冲区数据的大小

   // qDebug()<<"portId: "<<m_portId<<" buffsize: "<<buffsize;
    cun++;
    if(buffsize > 0) {
        dataReaded = true;
        m_readTimer->stop();
        buff = m_SerialPort->readAll();
        parseReceiveData(buff);
    }
}

void Ims_SerialPort::requestBarCode(){
    initTestCaseData();
    mRequestState = IMS_REQUEST_BARCODE;
    m_readTimer->start(readTimeout);
    QByteArray sendData = "REQBARCODE";
    qDebug()<<"requestBarCode:" << sendData << QThread::currentThreadId();;
    emit receive_string_data(COM3,sendData);
    m_SerialPort->write(sendData);
}

void Ims_SerialPort::sendTestResult(QString barcode,int result){
    initTestCaseData();
    mRequestState = IMS_SEND_TEST_RESULT;
    m_readTimer->start(readTimeout);
    QByteArray sendData = 0;
    sendData.clear();
    sendData.append("RET").append(barcode).append(",");
    if(result == 1) {
        sendData.append("OK");
    }else {
        sendData.append("NG");
    }
    qDebug()<<"sendTestResult:" << sendData;
    emit receive_string_data(COM3,sendData);
    m_SerialPort->write(sendData);
}

void Ims_SerialPort::parseReceiveData(QByteArray buff){
    qDebug()<<"parseReceiveData buff.length():" << buff.length();
   // 1、EOL设备请求条码信号固定为： REQBARCODE
   // 2、IMS响应条码请求时回传 “ACK” +条码：ACK123456789
   // 3、EOL设备测试结果需“RET”+条码+测试结果，中间以英文逗号“，”隔开。测试结果以OK/NG表示：RET123456789,NG/OK
  //  4、IMS响应EOL设备测试结果固定为: RECOK
    switch (mRequestState) {
    case IMS_REQUEST_BARCODE:{
        if(buff.length() > 29) return;
        QString barcode = "";
        if(buff.length() == 29) {
            if(buff.startsWith("ACK")) {
                barcode = buff.mid(3,28);
                qDebug()<<"parseReceiveData barcode:" << barcode;
                emit receive_data(IMS_REQUEST_BARCODE,barcode);
                emit receive_string_data(COM3,barcode.toUtf8());
            }else {
                emit receive_data(IMS_REQUEST_BARCODE,"");
            }
        }else if(buff.length() < 29){
            if(mCommandException) {
                mByteArray.append(buff);
                if(mByteArray.size() >= 29) {
                    mCommandException = false;
                    if(mByteArray.startsWith("ACK")) {
                        barcode = mByteArray.mid(3,28);
                        qDebug()<<"receive_data barcode:" << barcode;
                        emit receive_data(IMS_REQUEST_BARCODE,barcode);
                        emit receive_string_data(COM3,barcode.toUtf8());
                    } else {
                        emit receive_data(IMS_REQUEST_BARCODE,"");
                    }

                    mByteArray.clear();
                }
            }
            if(buff.startsWith("A")) {
                mByteArray.append(buff);
                mCommandException = true;
            }
        }
    }

        break;
    case IMS_SEND_TEST_RESULT:{
        if(buff.length() > 5) return;
        if(buff.size()== 5) {
            if(buff== "RECOK") {
                emit receive_data(IMS_SEND_TEST_RESULT,buff);
                emit receive_string_data(COM3,buff);
            }else {
                emit receive_data(IMS_SEND_TEST_RESULT,"");
            }
        }else if(buff.size() < 5) {
            if(mCommandException) {
                mByteArray.append(buff);
                if(mByteArray.size() >= 5) {
                    mCommandException = false;
                    if(mByteArray == "RECOK") {
                        emit receive_data(IMS_SEND_TEST_RESULT,mByteArray);
                        emit receive_string_data(COM3,mByteArray);
                    }else {
                        emit receive_data(IMS_SEND_TEST_RESULT,"");
                    }

                    mByteArray.clear();
                }
            }
            if(buff.startsWith("R")) {
                mByteArray.append(buff);
                mCommandException = true;
            }
        }
    }
        break;
    default:
        break;
    }
}

 void Ims_SerialPort::write_data(int requestCode,QString barcode,int result)
 {
     initTestCaseData();
     if(requestCode == IMS_REQUEST_BARCODE) {
         requestBarCode();
     } else if(requestCode == IMS_SEND_TEST_RESULT) {
         sendTestResult(barcode,result);
     } else {
         if(m_readTimer != NULL && m_readTimer->isActive()) {
             m_readTimer->stop();
         }
     }
 }

 void Ims_SerialPort::initTestCaseData(){
     dataReaded = false;
     mRequestState = IMS_DEFAULT;
     mCommandException = false;
     mByteArray.clear();
 }
