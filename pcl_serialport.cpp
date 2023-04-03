#include "pcl_serialport.h"
Pcl_SerialPort::Pcl_SerialPort(int portId, QString port,QObject *parent): QObject(parent)
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
    dataReaded = false;
}

Pcl_SerialPort::~Pcl_SerialPort()
{
    cun=0;
    delete m_SerialPort;
    delete my_thread;
}
void Pcl_SerialPort::show_fun_id()
{
    qDebug()<<"fun_id is:"<<QThread::currentThreadId();
}
void Pcl_SerialPort::show_slots_id()
{
    qDebug()<<"plc slots_id is dataReaded:"<<QThread::currentThreadId()<<dataReaded << "==mPlcCommand:" <<mPlcCommand << "==mReadRegister:" <<PLC_READ_NULL;
   // show_fun_id();
    if(mPlcCommand == PLC_CMD_NULL && mReadRegister == PLC_READ_NULL) {
        return;
    }
    if(!dataReaded) {
        qDebug()<<"plc slots_id is:"<<dataReaded;
        emit exception_data(COM2, "没有收到PLC串口消息:"+ mPlcCommand+mReadRegister);
        if(m_readTimer != NULL && m_readTimer->isActive()) {
            qDebug()<<"m_readTimer != NULL";
            m_readTimer->stop();
        }
        if(m_SerialPort != NULL & mPlcCommand != PLC_CMD_NULL) {
            if(mPlcCommand == PLC_R3_SET) {
                sendR3Reset();
            }

        }
    }
}

void Pcl_SerialPort::init_port(QString port)
{
    if(m_SerialPort == NULL) return;
    m_SerialPort->setPortName(port);
    m_SerialPort->setBaudRate(115200);//波特率
    m_SerialPort->setDataBits(QSerialPort::Data8);//数据位
    m_SerialPort->setParity(QSerialPort::OddParity);//奇偶校验
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
   connect(m_readTimer,SIGNAL(timeout()),this,SLOT(show_slots_id()));
   connect(m_SerialPort,SIGNAL(readyRead()),this,SLOT(read_data()));//有数据就读
   //connect(this,SIGNAL(thread_sig()),this,SLOT(show_slots_id()));
}

bool Pcl_SerialPort::checkPortOpen(){
    return mPortOpened;
}

QSerialPort * Pcl_SerialPort:: getSerialPort() {
    return m_SerialPort;
}

void Pcl_SerialPort::closePort(int portId)
{
    if(portId==m_portId)
    {
        qDebug()<<"Pcl_SerialPort closePort"<<m_portId;
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

void Pcl_SerialPort::read_data()
{
    QByteArray buff ;

    qint64 buffsize = m_SerialPort->size();//获取缓冲区数据的大小

   // qDebug()<<"read_data buffsize:" << buffsize;
    cun++;
    if(buffsize>0)
    {
        dataReaded = true;
        m_readTimer->stop();
        buff = m_SerialPort->readAll();
      //  emit receive_data(m_portId,buff);

        if(buff.contains("!")) {
            return;
        }
      //  qDebug()<<"buff: "<<buff <<"plc command:" << mPlcCommand <<"mReadRegister:" << mReadRegister;
        if(mReadRegister != PLC_READ_NULL ) {
            bool ret = false;
            bool needReadRegister = false;
            if(buffsize < 13) {
               // emit receive_data(m_portId,buff);
                if(mCommandException) {
                    mByteArray.append(buff);
                    if(mByteArray.size() >= 12) {
                        mCommandException = false;
                       // parseReceiveData(mByteArray);
                       // mByteArray.clear();
                        needReadRegister = true;
                    }
                }
                if(buff.startsWith("%")) {
                    mByteArray.append(buff);
                    mCommandException = true;
                }
            } else if (buffsize == 13) {
                //parseReceiveData(buff);
                mByteArray = buff;
                needReadRegister = true;
            }
            switch(mReadRegister) {
            case PLC_READ_R100_REGISTER: {
                if(needReadRegister) {
                    ret = parseReceiveData(mByteArray);
                    needReadRegister = false;
                }

              //  if(ret) {
                   // emit receive_data(m_portId,mByteArray);
               // }
                mByteArray.clear();
            }break;
            default:
                break;

            }
            emit send_start_button_state(ret);
            return;
        }
        emit receive_data(m_portId,buff);
        if((buff.compare("%01$WC14") == 0) | buff.contains("4")) {
        //    emit receive_data(m_portId,buff);
        //if(buff.startsWith("%")) {
        //if(buff.compare("%01") == 0) {
            switch(mPlcCommand) {
            case PLC_R1_SET:
            case PLC_R3_SET:
            {
                qDebug()<<"mPlcCommand == PLC_R3_SET";
                //readRegisterData();
                sendR11Set();
            }break;
            case PLC_R11_SET: {
                emit switch_to_ctrl(PLC_NOTIFY_START_TEST_CASE);
            } break;
            case PLC_R2_SET: {
                // readRegisterData();
                 //emit switch_to_ctrl(PLC_NOTIFY_R2_FINISH);
            } break;
            case PLC_R1_RESET:
            case PLC_R3_RESET:
            {
                sendR11Reset();
            } break;
            case PLC_R11_RESET:{
                sendR2Reset();
            } break;
            case PLC_R2_RESET:{
                emit switch_to_ctrl(PLC_NOTIFY_SET_TEST_RESULT);
            } break;
            default:
                break;
            }
            return;
        }

    }
}

bool Pcl_SerialPort::parseReceiveData(QByteArray buff) {
   // QByteArray buff = "%01$RD110013\r";
    if(!buff.startsWith("%01$RD")) {
        return false;
    }
    char * a = buff.data();
  //  qDebug() << a;
    QByteArray byteArray = 0;
    byteArray.append(*(a+6)).append(*(a+7));
    QString string = byteArray;
  //  qDebug() << string;

    bool ok;
    int dec=string.toInt(&ok,16);
   // qDebug() << dec;
    QString result=QString("%1").arg(dec, 8, 2, QChar('0'));
  //  qDebug() << result;
    QString R107 = result.at(0);
    QString R106 = result.at(1);
    QString R105 = result.at(2);
    QString R104 = result.at(3);
    QString R103 = result.at(4);
    QString R102 = result.at(5);
    QString R101 = result.at(6);
    QString R100 = result.at(7);
  //  qDebug() << "R107:" << R107 << "R106:" << R106 << "R105:" << R105 << "R104:" << R104 << "R103:" << R103 << "R102:" << R102 << "R101:" << R101 << "R100:" << R100 ;
    if(mReadRegister == PLC_READ_R100_REGISTER) {
        //qDebug()<<"PLC_R1_SET_REGISTER";
        if(QString::compare(R100, "1") == 0) {
            qDebug()<<"PLC_R1_SET_REGISTER true";
            return true;
        } else {
           //qDebug()<<"PLC_R1_SET_REGISTER false";
            return false;
        }
    }
    return false;
}
 void Pcl_SerialPort::write_data(int portId,QByteArray buff)
 {
     if(portId==m_portId)
     {
         m_SerialPort->write(buff,8);
     }
   //  emit thread_sig();
 }

 void Pcl_SerialPort::write_plc_data(int set){
   //  qDebug()<<"write_plc_data:" << set;
    // initTestCaseData();
   //  m_readTimer->start(readTimeout);
     switch(set) {
     case PLC_R1_SET:
         mStart = true;
         sendR1Set();
         break;
     case PLC_R2_SET:
         mStart = true;
         sendR2Set();
         break;
     case PLC_R3_SET:
         mStart = true;
         sendR3Set();
         break;
     case PLC_R1_RESET:
         mStart = true;
         sendR1Reset();
         break;
     case PLC_R2_RESET:
         mStart = true;
         sendR2Reset();
         break;
     case PLC_R3_RESET:
         mStart = true;
         sendR3Reset();
         break;
     case PLC_READ_R100_REGISTER:
     case PLC_READ_R101_REGISTER:
     case PLC_READ_R102_REGISTER:
     case PLC_READ_R103_REGISTER:
     case PLC_READ_R104_REGISTER:
     case PLC_READ_R105_REGISTER:
     case PLC_READ_R106_REGISTER:
     case PLC_READ_R107_REGISTER:
         //mStart = true;
         readRegisterData(set);
         break;
     case PLC_CMD_STOP:
         mStart = false;
         break;
     default:
         break;
     }

 }

 void Pcl_SerialPort::sendR1Set(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R1_SET;
     //QByteArray sendData = "%01#WCSR0001122\r";
     QByteArray sendData = "%01#WCSR0003120\r";
     qDebug()<<"sendR1Set write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR2Set(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R2_SET;
     QByteArray sendData = "%01#WCSR0002121\r";
     qDebug()<<"sendR2Set write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR3Set(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R3_SET;
     QByteArray sendData = "%01#WCSR0003120\r";
     qDebug()<<"sendR3Set write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR11Set(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R11_SET;
     QByteArray sendData = "%01#WCSR0011123\r";
     qDebug()<<"sendR11Set write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR12Set(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R12_SET;
     QByteArray sendData = "%01#WCSR0012120\r";
     qDebug()<<"sendR12Set write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR1Reset(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R1_RESET;
     //QByteArray sendData = "%01#WCSR0001023\r";
     QByteArray sendData = "%01#WCSR0003021\r";
     qDebug()<<"sendR1Reset write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR2Reset(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R2_RESET;
     QByteArray sendData = "%01#WCSR0002020\r";
     qDebug()<<"sendR2Reset write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR3Reset(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R3_RESET;
     QByteArray sendData = "%01#WCSR0003021\r";
     qDebug()<<"sendR3Reset write_plc_data" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR11Reset(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R11_RESET;
     QByteArray sendData = "%01#WCSR0011022\r";
     qDebug()<<"sendR11Reset write_plc_data:" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::sendR12Reset(){
     if(!mStart) return;
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mPlcCommand = PLC_R12_RESET;
     QByteArray sendData = "%01#WCSR0012021\r";
     qDebug()<<"sendR12Reset write_plc_data:" << sendData;
     emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
 }

 void Pcl_SerialPort::readRegisterData(int command) {
     initTestCaseData();
     m_readTimer->start(readTimeout);
     mReadRegister = command;
    // qDebug()<<"===============mReadRegister:" << mReadRegister;
     QByteArray sendData = "%01#RDD001000010055\r";
    // qDebug()<<"===============readRegisterData:" << sendData;
    // emit receive_data(m_portId,sendData);
     m_SerialPort->write(sendData);
    // qDebug()<<"===============readRegisterData write finish" ;

 }

 void Pcl_SerialPort::initTestCaseData(){
     mCommandException = false;
     dataReaded = false;
     mPlcCommand = PLC_CMD_NULL;
     mReadRegister = PLC_READ_NULL;
     mByteArray.clear();
 }
