#include "ctrl_serialport.h"
#include "datatypedef.h"
#include <QStringList>
#include<QMessageBox>
#include <synchapi.h>
static int readtime = 0;
Ctrl_SerialPort::Ctrl_SerialPort(int portId, QString port,QObject *parent): QObject(parent)
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
    mTofDataReadTime = 0;
    dataReaded = false;
    m_sendTimer = NULL;
    mStartTest = false;
    readtime = 0;
    mVolClearChecked= NOT_CHECKED;
}

Ctrl_SerialPort::~Ctrl_SerialPort()
{
    cun=0;
    mPortOpened = false;
    if(m_SerialPort != NULL) {
        m_SerialPort->close();
        delete m_SerialPort;
    }
}
void Ctrl_SerialPort::show_fun_id()
{
    qDebug()<<"fun_id is:"<<QThread::currentThreadId();
}
void Ctrl_SerialPort::show_slots_id()
{
    qDebug()<<"ctrl slots_id is:"<<QThread::currentThreadId();
    //show_fun_id();
   // qDebug()<<"slots_id is time:"<<readtime;
    readtime ++;
    if(!dataReaded) {
        qDebug()<<"slots_id is:"<<dataReaded;
        emit exception_data(COM0,"没有收到控制串口信息");
        //QMutexLocker locker(&mutex);
        mStartTest = false;
        if(m_readTimer != NULL) {
            m_readTimer->stop();
        }
    }
}

void Ctrl_SerialPort::init_port(QString port)
{
    m_SerialPort->setPortName(port);
    m_SerialPort->setBaudRate(115200);//波特率
    m_SerialPort->setDataBits(QSerialPort::Data8);//数据位
    m_SerialPort->setParity(QSerialPort::NoParity);//奇偶校验
    m_SerialPort->setStopBits(QSerialPort::OneStop);//停止位
    m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);//控制位

    if(!m_SerialPort->open(QIODevice::ReadWrite))//读写方式打开
    {
         qDebug()<<"====open port "<<m_portId <<" failed!"<<endl;
         mPortOpened = false;
    }
    else
    {
        qDebug()<<"====Port "<<m_portId<<" have been opened.";
        mPortOpened = true;
    }
    m_readTimer = new QTimer(this);
    //m_readTimer->start(5000); /*开启定时器，并且每100ms后询问一次串口。定时的时间一到，马上产生timeout（）信号，继续执行自定义槽函数readMyCom() */
  //  m_sendTimer = new QTimer(this);
    //connect(m_sendTimer,SIGNAL(timeout()),this,SLOT(runtestcase()));
    connect(m_readTimer,SIGNAL(timeout()),this,SLOT(show_slots_id()));
    connect(m_SerialPort,SIGNAL(readyRead()),this,SLOT(read_data()));//有数据就读
    connect(this,SIGNAL(run_case_sig(bool,int,int)),this,SLOT(runTestCase(bool,int,int)));
}

bool Ctrl_SerialPort::checkPortOpen() {
    return mPortOpened;
}
QSerialPort * Ctrl_SerialPort:: getSerialPort() {
    return m_SerialPort;
}

void Ctrl_SerialPort::closePort(int portId)
{
    if(portId==m_portId)
    {
        qDebug() << "Ctrl_SerialPort closePort:" << portId;
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

void Ctrl_SerialPort::read_data()
{

    // qDebug()<< "read_data"<<QThread::currentThreadId();
   // dataReaded = true;
    QByteArray buff = 0;

    qint64 buffsize = m_SerialPort->size();//获取缓冲区数据的大小
    //qDebug()<<"===buffsize: "<<buffsize;
    cun++;
     if(buffsize > 0)
     {
        dataReaded = true;
        if(m_readTimer != NULL) {
            m_readTimer->stop();
        }

        buff = m_SerialPort->readAll();
        if(buffsize < 8) {
            if(mCommandException) {
                mByteArray.append(buff);
                if(mByteArray.size() == 8) {
                    mCommandException = false;
                    handleReadData(mByteArray);
                    mByteArray.clear();
                }
            }
            QString str_temp = buff.toHex().toUpper();
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
            uint8_t int_frame_head= st_frame_head.toInt(&ok, 16);
            if(int_frame_head == enFrame_head::AR_KEY_RECEIVE_HEAD) {
                mByteArray.append(buff);
                mCommandException = true;
            }
        } else if (buffsize == 8) {
            handleReadData(buff);
        }
    }
}

void Ctrl_SerialPort::handleReadData(QByteArray buff) {
    //qDebug()<<"===buff: "<<buff << "--buffsize: "<<buff.size();
    emit receive_data(m_portId,buff);
    qDebug() << "buff:" << buff;
    QString str_temp = buff.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
    QTime time = QTime::currentTime();
    QString time_strting = time.toString ("hh:mm:ss:msms");
    qDebug()<<time_strting<< "=====readData" << str_show << "--str_show size:" << str_show.size();
    buff.clear();
    parseReadData(str_show);
}

void Ctrl_SerialPort::parseReadData_temp(QString buff){
    QStringList list = buff.split(" ");
    bool ok;
    //Frame head
    QString st_frame_head= list.at(0);
    uint8_t int_frame_head= st_frame_head.toInt(&ok, 16);
    if(int_frame_head != enFrame_head::AR_KEY_RECEIVE_HEAD) {
        //retrySendCommand();
        return;
    }

    //Test_cmd
    QString st_test_cmd= list.at(1);
    uint8_t int_test_cmd= st_test_cmd.toInt(&ok, 16);
    QString str_data_0;
    uint8_t int_data_0;
    QString str_data_1;
    uint8_t int_data_1;
    QString str_data_2;
    uint8_t int_data_2;
    QString str_data_3;
    uint8_t int_data_3;
    QString str_data_4;
    uint8_t int_data_4;
    str_data_0= list.at(2);
    int_data_0= str_data_0.toInt(&ok, 16);
    str_data_1= list.at(3);
    int_data_1= str_data_1.toInt(&ok, 16);
    str_data_2= list.at(4);
    int_data_2= str_data_2.toInt(&ok, 16);
    str_data_3= list.at(5);
    int_data_3= str_data_3.toInt(&ok, 16);
    str_data_4= list.at(6);
    int_data_4= str_data_4.toInt(&ok, 16);
    switch(int_test_cmd){
    case enTest_cmd::AR_KEY_TEST_START:
    {
       // emit test_start(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,int_data_0,0,0,0);
        if( int_data_0 ==1) {
           // QThread::msleep(100);
            testVol9Normal();
        // runTestCase();
        } else {
            mTestCmd = AR_KEY_NO_CMD;
            return;
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_NORMAL:
    {
        qDebug()<<AR_KEY_TEST_VOL_NORMAL;
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
       // emit test_vol_normal(mVolTestType,current_value,voltage_value);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,voltage_value,0,0,0,0);
        //mVol9NormalChecked = CHECKED_PASS;
        if(mVolTestType == V9_NORMAL) {
            QThread::msleep(100);
            testVol13Normal();
        } else if(mVolTestType == V13_NORMAL) {
            QThread::msleep(100);
            testVol15Normal();
        } else if(mVolTestType == V15_NORMAL) {
          //  testVol9Sleep();
            QThread::msleep(100);
            testVol9Idle();
        }
        /*if(int_data_4 == V9_NORMAL) {
             QThread::msleep(100);
             testVol13Normal();
         } else if(int_data_4 == V13_NORMAL) {
             QThread::msleep(100);
             testVol15Normal();
         } else if(int_data_4 == V15_NORMAL) {
           //  testVol9Sleep();
             QThread::msleep(100);
             testVol9Idle();
         }*/
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_SLEEP:
    {
        qDebug() << "===parseSerialData AR_KEY_TEST_VOL_SLEEP";
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
        //emit test_vol_sleep(mVolTestType,current_value,voltage_value);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,voltage_value,0,0,0,0);
        if(mVolTestType == V9_SLEEP) {
            QThread::msleep(100);
            testVol13Sleep();
        } else if(mVolTestType == V13_SLEEP) {
            QThread::msleep(100);
            testVol15Sleep();
        } else if(mVolTestType == V15_SLEEP) {
            QThread::msleep(100);
            testVol9Idle();
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_IDLE:
    {
        qDebug() << "AR_KEY_TEST_VOL_IDLE";
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
        //emit test_vol_idle(mVolTestType,current_value,voltage_value);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,voltage_value,0,0,0,0);
        if(mVolTestType == V9_IDLE) {
            QThread::msleep(100);
            testVol13Idle();
        } else if(mVolTestType == V13_IDLE) {
            QThread::msleep(100);
            testVol15Idle();
        } else if(mVolTestType == V15_IDLE) {
            QThread::msleep(100);
            testHardwareVer();
        }
        /*if(int_data_4 == V9_IDLE) {
            QThread::msleep(100);
            testVol13Idle();
        } else if(int_data_4 == V13_IDLE) {
            QThread::msleep(100);
            testVol15Idle();
        } else if(int_data_4 == V15_IDLE) {
            QThread::msleep(100);
            testHardwareVer();
        }*/
    }break;
    case enTest_cmd::AR_KEY_TEST_TEMP:
    {
        int voltage_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        emit show_serial_data(int_test_cmd,0,0,voltage_value,0,0,0,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_HARDWARE_VER:
    {
        /*
         * 版本号：ASCII编码，4个字节，形式为：VX.X.XX，4个字节依次对应4个X
         * */
        qDebug() << "AR_KEY_TEST_HARDWARE_VER";
        int low_four, high_four;
        high_four = (int_data_0 & 0xf0) >> 4;
        low_four = int_data_0 & 0x0f;
        qDebug() << "hw high_four:" << high_four << "low_four:" <<low_four;
        QString v = "V";
        QString hw_string = v.append(QString::number(high_four)).append(".").append(QString::number(low_four)).append(".").append(str_data_1);
        qDebug() << "hw:" << hw_string;
        //emit test_hw(hw_string);
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,hw_string);
        QThread::msleep(100);
        testSoftwareVer();
    }break;
    case enTest_cmd::AR_KEY_TEST_SOFTWARE_VER:
    {
        qDebug() << "parseSerialData AR_KEY_TEST_SOFTWARE_VER";
        int low_four, high_four;
        high_four = (int_data_0 & 0xf0) >> 4;
        low_four = int_data_0 & 0x0f;
        qDebug() << "sw high_four:" << high_four << "low_four:" <<low_four;
        QString v = "V";
        QString sw_string = v.append(QString::number(high_four)).append(".").append(QString::number(low_four)).append(".").append(str_data_1);
        qDebug() << "sw_string:" << sw_string;
        //emit test_sw(sw_string);
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,sw_string);
        QThread::msleep(100);
        testProductNumFirst();
    }break;
    case enTest_cmd::AR_KEY_TEST_PRODUCT_NUB:
    {
        //convert ASCII
       QString productNum = QString(int_data_0).append(QString(int_data_1)).append(QString(int_data_2)).append(QString(int_data_3)).append(QString(int_data_4));
       static int readTime = 0;
       readTime ++;
       if(readTime == 1) {
           product_num_value_1 = productNum;
           QThread::msleep(100);
           testProductNumSecond();
           qDebug() << "product_num_value_1:" << product_num_value_1;
           return;
       }
       QString TotalNum = 0;
       if(readTime ==2) {
           product_num_value_2 = productNum;
           qDebug() << "product_num_value_2:" << product_num_value_2;
           readTime = 0;
           TotalNum = product_num_value_1.append(product_num_value_2);
           qDebug() << "TotalNum:" << TotalNum;
       }
       //emit test_productNum(TotalNum);
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,TotalNum);
       QThread::msleep(100);
       testVolHigh();
    }break;

    case enTest_cmd::AR_KEY_TEST_VOL_HIGH:
    {
        qDebug() << "AR_KEY_TEST_VOL_HIGH";
        int vol_high_normal_flag= int_data_0;
       // emit test_vol_high(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,vol_high_normal_flag,0,0,0);
        QThread::msleep(100);
        testVolLow();
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_LOW:
    {
        qDebug() << "AR_KEY_TEST_VOL_LOW";
        int vol_low_normal_flag =int_data_0;
        //emit test_vol_low(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,vol_low_normal_flag,0,0,0);
        QThread::msleep(100);
        testVolClear();
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_CLEAR:
    {
        qDebug() << "AR_KEY_TEST_VOL_CLEAR";
        int vol_clear_flag= int_data_0;
        //emit test_vol_clear(int_data_0);
        if(mVolClearChecked == NOT_CHECKED) {
            mVolClearChecked= CHECKED;
            emit show_serial_data(int_test_cmd,0,0,0,vol_clear_flag,0,0,0);
            QThread::msleep(100);
            testLogo();
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_WAKEUP:
    {
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value= BYTE_TO_UINT16(int_data_2, int_data_3);
        int flag = int_data_4;
        //emit test_wakeup(current_value,voltage_value,flag);
        emit show_serial_data(int_test_cmd,0,current_value,voltage_value,flag,0,0,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_LOGO:
    {
        qDebug() << "+++++AR_KEY_TEST_LOGO";
        //int logo_opened = BYTE_TO_UINT16(int_data_0, int_data_1);
       // emit test_logo_opened(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,int_data_0,0,0,0);
        QThread::msleep(100);
        testTofOpenned();
    }break;
    case enTest_cmd::AR_KEY_TEST_TOF:
    {
        qDebug() << "AR_KEY_TEST_TOF" <<mTofFunc;
        //int tof_opened = BYTE_TO_UINT16(int_data_0, int_data_1);
        int tof_opened = int_data_0;
        int tof_distance = BYTE_TO_UINT16(int_data_1, int_data_2);
        if(mTofFunc == TOF_OPEN) {
            if(int_data_0 == 1) {
               // testTofReadValue();
              // QThread::msleep(100);
            }
            //emit test_tof_opened(int_data_0,int_data_1);
            emit show_serial_data(int_test_cmd,0,0,0,tof_opened,mTofFunc,tof_distance,0);
        }else if(mTofFunc == TOF_READ_VALUE) {
            qDebug() << "mTofDataReadTime" <<mTofDataReadTime;
            mTofDataReadTime ++;
            if(int_data_0 == 1) {
                emit show_serial_data(int_test_cmd,0,0,0,int_data_0,mTofFunc,tof_distance,0);
                QThread::msleep(100);
                testVolClear();
                return;
            } else {
                if(mTofDataReadTime == 3) {
                    emit show_serial_data(int_test_cmd,0,0,0,int_data_0,mTofFunc,tof_distance,0);
                    QThread::msleep(100);
                    testVolClear();
                    return;
                } else if(mTofDataReadTime < 3) {
                    QThread::msleep(100);
                   testTofReadValue();
                } else{
                    mTofDataReadTime = 0;
                    return;
                }
            }

        }

    }break;
    }
}

void Ctrl_SerialPort::parseReadData(QString buff){
    QStringList list = buff.split(" ");
    bool ok;
    //Frame head
    QString st_frame_head= list.at(0);
    uint8_t int_frame_head= st_frame_head.toInt(&ok, 16);
    qDebug() <<"st_frame_head1111:" << int_frame_head;
    if(int_frame_head != enFrame_head::AR_KEY_RECEIVE_HEAD) {
        //retrySendCommand();
        return;
    }

    //Test_cmd
    QString st_test_cmd= list.at(1);
    uint8_t int_test_cmd= st_test_cmd.toInt(&ok, 16);
    QString str_data_0;
    uint8_t int_data_0;
    QString str_data_1;
    uint8_t int_data_1;
    QString str_data_2;
    uint8_t int_data_2;
    QString str_data_3;
    uint8_t int_data_3;
    QString str_data_4;
    uint8_t int_data_4;
    str_data_0= list.at(2);
    int_data_0= str_data_0.toInt(&ok, 16);
    str_data_1= list.at(3);
    int_data_1= str_data_1.toInt(&ok, 16);
    str_data_2= list.at(4);
    int_data_2= str_data_2.toInt(&ok, 16);
    str_data_3= list.at(5);
    int_data_3= str_data_3.toInt(&ok, 16);
    str_data_4= list.at(6);
    int_data_4= str_data_4.toInt(&ok, 16);
    switch(int_test_cmd){
    case enTest_cmd::AR_KEY_TEST_START:
    {
         qDebug() << "AR_KEY_TEST_START:" << AR_KEY_TEST_START;
        if(mOnlyClearData) return;
        emit show_serial_data(int_test_cmd,0,0,0,int_data_0,0,0,0);
        if( int_data_0 ==1) {
          //  QThread::msleep(100);
         //   emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V9_NORMAL);
        } else {
            mTestCmd = AR_KEY_NO_CMD;
            return;
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_NORMAL:
    {
        qDebug()<<AR_KEY_TEST_VOL_NORMAL;
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
       // uint32_t current_value = BYTE_TO_UINT32(int_data_0,int_data_1,int_data_2,int_data_3);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,0,0,0,0,0);
        /*if(int_data_4 == V9) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V13_NORMAL);
         } else if(int_data_4 == V13) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V15_NORMAL);
         } else if(int_data_4 == V15) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V9_IDLE);
         }*/
         QThread::msleep(100);
        if(mVolTestType == V9_NORMAL) {
           // testVol13Normal();
            emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V13_NORMAL);
        } else if(mVolTestType == V13_NORMAL) {
            //testVol15Normal();
            emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V15_NORMAL);
        } else if(mVolTestType == V15_NORMAL) {
          //  testVol9Sleep();
            //testVol9Idle();
          //  emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V9_IDLE);
            //emit run_case_sig(true,AR_KEY_TEST_VOL_HIGH,0);
            emit run_case_sig(true,AR_KEY_TEST_HARDWARE_VER,0);

        }
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_SLEEP:
    {
        qDebug() << "===parseSerialData AR_KEY_TEST_VOL_SLEEP";
        uint32_t current_value = BYTE_TO_UINT32(int_data_0,int_data_1,int_data_2,int_data_3);
       // int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
      //  int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,0,0,0,0,0);
        QThread::msleep(100);
        if(mVolTestType == V9_SLEEP) {
            //testVol13Sleep();
            emit run_case_sig(true,AR_KEY_TEST_VOL_SLEEP,V13_SLEEP);
        } else if(mVolTestType == V13_SLEEP) {
            //testVol15Sleep();
            emit run_case_sig(true,AR_KEY_TEST_VOL_SLEEP,V15_SLEEP);
        } else if(mVolTestType == V15_SLEEP) {
           // emit run_case_sig(true,AR_KEY_TEST_VOL_HIGH,0);
            emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V9_IDLE);
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_IDLE:
    {
        qDebug() << "AR_KEY_TEST_VOL_IDLE";
       // int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
      //  int voltage_value = BYTE_TO_UINT16(int_data_2, int_data_3);
       uint32_t current_value = BYTE_TO_UINT32(int_data_0,int_data_1,int_data_2,int_data_3);
        emit show_serial_data(int_test_cmd,mVolTestType,current_value,0,0,0,0,0);
        /*if(int_data_4 == V9_IDLE) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V13_IDLE);
        } else if(int_data_4 == V13_IDLE) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V15_IDLE);
        } else if(int_data_4 == V15_IDLE) {
            QThread::msleep(100);
             emit run_case_sig(true,AR_KEY_TEST_HARDWARE_VER,0);
        }*/
         QThread::msleep(100);
        if(mVolTestType == V9_IDLE) {
            //testVol13Idle();
            emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V13_IDLE);
        } else if(mVolTestType == V13_IDLE) {
            //testVol15Idle();
            emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V15_IDLE);
        } else if(mVolTestType == V15_IDLE) {
            //testHardwareVer();
            //testVol9Sleep();
           // emit run_case_sig(true,AR_KEY_TEST_VOL_SLEEP,V9_SLEEP);
            emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V9_NORMAL);
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_TEMP:
    {
        int voltage_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        QThread::msleep(100);
        emit show_serial_data(int_test_cmd,0,0,voltage_value,0,0,0,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_HARDWARE_VER:
    {
        /*
         * 版本号：ASCII编码，4个字节，形式为：VX.X.XX，4个字节依次对应4个X
         * */
        qDebug() << "AR_KEY_TEST_HARDWARE_VER "<< int_data_0 <<str_data_0;
        int low_four, high_four;
        high_four = (int_data_0 & 0xf0) >> 4;
        low_four = int_data_0 & 0x0f;
        qDebug() << "hw high_four:" << high_four << "low_four:" <<low_four;
        QString v;
        char st_0= int_data_0;
        QString str_ascii_0 = QString(st_0);
        char st_1= int_data_1;
        QString str_ascii_1 = QString(st_1);
        char st_2= int_data_2;
        QString str_ascii_2 = QString(st_2);
        QString hw_string = v.append(str_ascii_0).append(str_ascii_1).append(str_ascii_2);
        qDebug() << "hw:" << hw_string;
        //emit test_hw(hw_string);
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,hw_string);
        QThread::msleep(100);
        emit run_case_sig(true,AR_KEY_TEST_SOFTWARE_VER,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_SOFTWARE_VER:
    {
        qDebug() << "parseSerialData AR_KEY_TEST_SOFTWARE_VER";
        int low_four, high_four;
        high_four = (int_data_0 & 0xf0) >> 4;
        low_four = int_data_0 & 0x0f;
        qDebug() << "sw high_four:" << high_four << "low_four:" <<low_four;
        QString v = "S";
        char st_0= int_data_0;
        QString str_ascii_0 = QString(st_0);
        char st_1= int_data_1;
        QString str_ascii_1 = QString(st_1);
        char st_2= int_data_2;
        QString str_ascii_2 = QString(st_2);
        char st_3= int_data_3;
        QString str_ascii_3 = QString(st_3);
        char st_4= int_data_4;
        QString str_ascii_4 = QString(st_4);
        QString sw_string = v.append(str_ascii_0).append(str_ascii_1).append(str_ascii_2).append(str_ascii_3).append(str_ascii_4);
        qDebug() << "sw_string:" << sw_string;
        //emit test_sw(sw_string);
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,sw_string);
        //QThread::msleep(100);
        //emit run_case_sig(true,AR_KEY_TEST_PRODUCT_NUB,1);
        emit run_case_sig(true,AR_KEY_TEST_VOL_HIGH,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_PRODUCT_NUB:
    {
        //convert ASCII
       QString productNum = QString(int_data_0).append(QString(int_data_1)).append(QString(int_data_2)).append(QString(int_data_3)).append(QString(int_data_4));
       static int readTime = 0;
       readTime ++;
       if(readTime == 1) {
           product_num_value_1 = productNum;
           emit run_case_sig(true,AR_KEY_TEST_PRODUCT_NUB,2);
           qDebug() << "product_num_value_1:" << product_num_value_1;
           return;
       }
       QString TotalNum = 0;
       if(readTime ==2) {
           product_num_value_2 = productNum;
           qDebug() << "product_num_value_2:" << product_num_value_2;
           readTime = 0;
           TotalNum = product_num_value_1.append(product_num_value_2);
           qDebug() << "TotalNum:" << TotalNum;
       }
        emit show_serial_data(int_test_cmd,0,0,0,0,0,0,TotalNum);
        QThread::msleep(100);
        emit run_case_sig(true,AR_KEY_TEST_VOL_HIGH,0);
    }break;

    case enTest_cmd::AR_KEY_TEST_VOL_HIGH:
    {
        qDebug() << "AR_KEY_TEST_VOL_HIGH";
        int vol_high_normal_flag= int_data_0;
       // emit test_vol_high(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,vol_high_normal_flag,0,0,0);
        QThread::msleep(100);
        emit run_case_sig(true,AR_KEY_TEST_VOL_LOW,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_LOW:
    {
        qDebug() << "AR_KEY_TEST_VOL_LOW";
        int vol_low_normal_flag =int_data_0;
        //emit test_vol_low(int_data_0);
        emit show_serial_data(int_test_cmd,0,0,0,vol_low_normal_flag,0,0,0);
        QThread::msleep(100);
        emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_VOL_CLEAR:
    {
        qDebug() << "AR_KEY_TEST_VOL_CLEAR" << mVolClearChecked;
        if(mOnlyClearData) return;
        int vol_clear_flag= int_data_0;
        //emit test_vol_clear(int_data_0);
        if(mVolClearChecked== CHECKED) {
           // emit show_serial_data(int_test_cmd,0,0,0,vol_clear_flag,0,0,0);
            return;
        }
        if(mVolClearChecked == NOT_CHECKED) {
            mVolClearChecked= CHECKED;
            emit show_serial_data(int_test_cmd,0,0,0,vol_clear_flag,0,0,0);
            QThread::msleep(100);
            emit run_case_sig(true,AR_KEY_TEST_LOGO,0);
        }
    }break;
    case enTest_cmd::AR_KEY_TEST_WAKEUP:
    {
        int current_value = BYTE_TO_UINT16(int_data_0, int_data_1);
        int voltage_value= BYTE_TO_UINT16(int_data_2, int_data_3);
        int flag = int_data_4;
        //emit test_wakeup(current_value,voltage_value,flag);
        emit show_serial_data(int_test_cmd,0,current_value,voltage_value,flag,0,0,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_LOGO:
    {
        qDebug() << "===============AR_KEY_TEST_LOGO";
        //int logo_opened = BYTE_TO_UINT16(int_data_0, int_data_1);
        emit show_serial_data(int_test_cmd,0,0,0,int_data_0,0,0,0);
    }break;
    case enTest_cmd::AR_KEY_TEST_TOF:
    {
        qDebug() << "======AR_KEY_TEST_TOF func:" <<mTofFunc;
        //int tof_opened = BYTE_TO_UINT16(int_data_0, int_data_1);
        int tof_opened = int_data_0;
        int tof_distance = BYTE_TO_UINT16(int_data_1, int_data_2);
        if(mTofFunc == TOF_OPEN) {
            if(int_data_0 == 1) {
              // QThread::msleep(100);
            }
            emit show_serial_data(int_test_cmd,0,0,0,tof_opened,mTofFunc,tof_distance,0);
        }else if(mTofFunc == TOF_READ_VALUE) {
            if(mTofChecked== CHECKED) {
                return;
            }
            if(mTofChecked == NOT_CHECKED) {
                mTofChecked= CHECKED;
                QTime time = QTime::currentTime();
                QString time_strting = time.toString ("hh:mm:ss:msms");
                qDebug()<<time_strting<< "mTofDataReadTime" <<mTofDataReadTime;
                mTofDataReadTime ++;
                emit show_serial_data(int_test_cmd,0,0,0,int_data_0,mTofFunc,tof_distance,0);
                emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
            }
            /*if(int_data_0 == 1) {
                emit show_serial_data(int_test_cmd,0,0,0,int_data_0,mTofFunc,tof_distance,0);
               // QThread::msleep(100);
                emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
                return;
            } else {
                if(mTofDataReadTime == 3) {
                    emit show_serial_data(int_test_cmd,0,0,0,int_data_0,mTofFunc,tof_distance,0);
                   // QThread::msleep(100);
                    emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
                    return;
                } else if(mTofDataReadTime < 3) {
                    QThread::msleep(100);
                    emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_READ_VALUE);
                } else{
                    mTofDataReadTime = 0;
                    return;
                }
            }*/

        }
    }break;
    }
}

void Ctrl_SerialPort::runTestCase(bool start, int cmd,int case_item ){
   // qDebug() << "===runTestCase" << QThread::currentThreadId();
    //QMutexLocker locker(&mutex);
    if(!start && cmd == AR_KEY_NO_CMD) {
        qDebug() << "===runTestCase return 1111";
        return;
    }
    if(!mStartTest) {
        qDebug() << "===runTestCase return 2222";
        return;
    }
    qDebug() << "===runTestCase start:" << start << "cmd:"<<cmd;
    if(start) {
        switch(cmd) {
        case AR_KEY_TEST_START: {
            testStart();
        }break;
        case AR_KEY_TEST_VOL_NORMAL:{
            if(case_item == V9_NORMAL) {
                //QThread::msleep(100);
                testVol9Normal();
            }else if(case_item == V13_NORMAL){
                //QThread::msleep(100);
                testVol13Normal();
            }else if(case_item == V15_NORMAL) {
               // QThread::msleep(100);
                testVol15Normal();
            }
        }break;
        case AR_KEY_TEST_VOL_SLEEP:{
            if(case_item == V9_SLEEP) {
                testVol9Sleep();
            }else if(case_item == V13_SLEEP){
                testVol13Sleep();
            }else if(case_item == V15_SLEEP) {
                testVol15Sleep();
            }
        }break;
        case AR_KEY_TEST_VOL_IDLE:{
            if(case_item == V9_IDLE) {
               // QThread::msleep(100);
                testVol9Idle();
            }else if(case_item == V13_IDLE){
               // QThread::msleep(100);
                testVol13Idle();
            }else if(case_item == V15_IDLE) {
               // QThread::msleep(100);
                testVol15Idle();
            }
        }break;
        case AR_KEY_TEST_TEMP:{
        }break;
        case AR_KEY_TEST_HARDWARE_VER:{
            //QThread::msleep(100);
            testHardwareVer();
        }break;
        case AR_KEY_TEST_SOFTWARE_VER:{
            //QThread::msleep(100);
            testSoftwareVer();
        }break;
        case AR_KEY_TEST_PRODUCT_NUB:{
            if(case_item == 1) {
               // QThread::msleep(100);
                testProductNumFirst();
            } else if(case_item == 2) {
               // QThread::msleep(100);
                testProductNumSecond();
            }
        }break;
        case AR_KEY_TEST_VOL_HIGH:{
           // QThread::msleep(100);
             testVolHigh();
        }break;
        case AR_KEY_TEST_VOL_LOW:{
           // QThread::msleep(100);
            testVolLow();
        }break;
        case AR_KEY_TEST_VOL_CLEAR :{
          //  QThread::msleep(100);
            testVolClear();
        }break;
        case AR_KEY_TEST_WAKEUP :{

        }break;
        case AR_KEY_TEST_LOGO:{
           // QThread::msleep(100);
            testLogo();
        }break;
        case AR_KEY_TEST_TOF:{
            if(case_item == TOF_OPEN){
               // QThread::msleep(100);
               qDebug() << "========runTestCase testTofOpenned";
                testTofOpenned();
            } else if(case_item == TOF_READ_VALUE) {
             //   QThread::msleep(100);
                testTofReadValue();
            }
        }break;
        }
    }
}
 void Ctrl_SerialPort::write_data(int portId,QByteArray buff)
 {
     //if(portId==m_portId)
     //{
         qDebug()<<m_portId<<" write_id is:"<<buff << buff.size();
         m_SerialPort->write(buff,8);
         m_readTimer->start(readTimeout);
    // }
    // emit thread_sig();
 }

 void Ctrl_SerialPort::startCtrlPortTest(bool start, int case_item,int testType, QString testProject) {
     initTestCaseData(testProject);
     qDebug() << "startCtrlPortTest" << start;
   //  mTestProject = testProject;
     mTestType = testType;
    // QMutexLocker locker(&mutex);
     mStartTest = start;
     //todo
     if(start) {
         m_readTimer->start(readTimeout);
         emit run_case_sig(start,AR_KEY_TEST_START,0);
     }else {
         m_readTimer->stop();
         emit run_case_sig(start,AR_KEY_NO_CMD,0);
     }
    // emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_OPEN);

}

void Ctrl_SerialPort::tof_test_start(int flag){
    qDebug()<<"========on_tof_test_start";
    emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_OPEN);
}

void Ctrl_SerialPort::read_plc_value(int flag){
    emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_READ_VALUE);
    //int time = 3;
   //while(time >0) {
       // qDebug()<<"on_read_plc_value" << time;
       // time --;
       // testTofReadValue();
       // Sleep(1000);
    //}
}

void Ctrl_SerialPort::start_run_test_case(int cmd,int case_num){
    switch (cmd) {
    case AR_KEY_TEST_VOL_NORMAL:
        emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,case_num);
        break;
    case AR_KEY_TEST_VOL_SLEEP:
        emit run_case_sig(true,AR_KEY_TEST_VOL_SLEEP,case_num);
        break;
    case AR_KEY_TEST_TOF: {
        if(case_num == TOF_READ_VALUE) {
            emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_READ_VALUE);
        } else {
            emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_OPEN);
        }
    }break;
    case AR_KEY_TEST_LOGO:{
        emit run_case_sig(true,AR_KEY_TEST_LOGO,0);
    }break;
    case AR_KEY_TEST_VOL_CLEAR:{
        if(case_num == 0) {
            emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
        }else {
            mOnlyClearData = true;
            testVolClear();
        }
    }break;
    case AR_KEY_TEST_HARDWARE_VER:{
        emit run_case_sig(true,AR_KEY_TEST_HARDWARE_VER,0);
    }break;
    case AR_KEY_TEST_START: {
        if(case_num == 1) {
            mOnlyClearData = true;
            testStart();
        }else {

        }
    }
    }
}
void Ctrl_SerialPort::testStart(){
    mTestCmd = enTest_cmd::AR_KEY_TEST_START;
    uint8_t cmd_data[5] = {0};
    if(mTestProject.compare("C206") == 0) {
        cmd_data[0] = 0x01;
    }else if(mTestProject.compare("C03") == 0) {
        cmd_data[0] = 0x02;
    }else if(mTestProject.compare("N61") == 0) {
        cmd_data[0] = 0x03;
    }else if(mTestProject.compare("N60") == 0) {
        cmd_data[0] = 0x04;
    }else if(mTestProject.compare("C46") == 0) {
        cmd_data[0] = 0x05;
    }
    serial_cmd_pack(mTestCmd,cmd_data);
}

 //工作电压测试
 //0X01-9V
 void Ctrl_SerialPort::testVol9Normal(){
     qDebug()<<"testVol9Normal ";
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x01;
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_NORMAL;
     mVolTestType = V9_NORMAL;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 //0X00-13V
 void Ctrl_SerialPort::testVol13Normal(){
     qDebug()<<"testVol13Normal ";
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_NORMAL;
     mVolTestType = V13_NORMAL;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //0X02-15V
 void Ctrl_SerialPort::testVol15Normal(){
     qDebug()<<"testVol13Normal ";
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x02;
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_NORMAL;
     mVolTestType = V15_NORMAL;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 //睡眠电压测试
 //0X01-9V
 void Ctrl_SerialPort::testVol9Sleep(){
     qDebug()<<"testVol9Sleep ";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_SLEEP;
     mVolTestType = V9_SLEEP;
     cmd_data[0] = 0x01;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //0X00-13V
 void Ctrl_SerialPort::testVol13Sleep(){
     qDebug()<<"testVol13Sleep";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_SLEEP;
     mVolTestType = V13_SLEEP;
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //0X02-15V
 void Ctrl_SerialPort::testVol15Sleep(){
     qDebug()<<"testVol15Sleep";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_SLEEP;
     mVolTestType = V15_SLEEP;
     cmd_data[0] = 0x02;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //空闲电压测试
 void Ctrl_SerialPort::testVol9Idle(){
     qDebug()<<"testVol9Idle ";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_IDLE;
     mVolTestType = V9_IDLE;
     //0X01-9V
     cmd_data[0] = 0x01;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 void Ctrl_SerialPort::testVol13Idle(){
     qDebug()<<"testVol13Idle ";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_IDLE;
     mVolTestType = V13_IDLE;
     //0X00-13V
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 void Ctrl_SerialPort::testVol15Idle(){
     qDebug()<<"testVol15Idle ";
     uint8_t cmd_data[5] = {0};
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_IDLE;
     mVolTestType = V15_IDLE;
     //0X02-15V
     cmd_data[0] = 0x02;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //温度电压测试：
 void Ctrl_SerialPort::testTemp(){
     qDebug()<<"testTemp: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_TEMP;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //硬件版本号
 void Ctrl_SerialPort::testHardwareVer(){
     qDebug()<<"testHardwareVer: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_HARDWARE_VER;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //软件版本号
 void Ctrl_SerialPort::testSoftwareVer(){
     qDebug()<<"testSoftwareVer: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_SOFTWARE_VER;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //零件号
 void Ctrl_SerialPort::testProductNumFirst(){
     qDebug()<<"testProductNumFirst ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_PRODUCT_NUB;
     uint8_t cmd_data[5] = {0};
     mProductNum = 0;
     //0x00-read first 5bytes
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 void Ctrl_SerialPort::testProductNumSecond(){
     qDebug()<<"testProductNumSecond ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_PRODUCT_NUB;
     uint8_t cmd_data[5] = {0};
     mProductNum = 1;
    // 0x01-read second 5bytes
     cmd_data[0] = 0x01;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 //投影功能测试
 void Ctrl_SerialPort::testLogo(){
     qDebug()<<"testLogo: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_LOGO;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //TOF功能测试
 void Ctrl_SerialPort::testTofOpenned(){
     qDebug()<<"=============testTofOpenned ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_TOF;
     uint8_t cmd_data[5] = {0};
     mTofFunc = TOF_OPEN;
    // 0-open TOF
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 void Ctrl_SerialPort::testTofReadValue(){
     QTime time = QTime::currentTime();
     QString time_strting = time.toString ("hh:mm:ss:msms");
     qDebug()<<time_strting<<"testTofReadValue ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_TOF;
     uint8_t cmd_data[5] = {0};
     mTofFunc = TOF_READ_VALUE;
     //1-read TOF val
     cmd_data[0] = 0x01;
     serial_cmd_pack(mTestCmd,cmd_data);
     //emit receive_data(m_portId,"%01#WCSR0002121\r");
 }
 //过压故障测试
 void Ctrl_SerialPort::testVolHigh(){
     qDebug()<<"testVolHigh ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_HIGH;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //欠压故障测试
 void Ctrl_SerialPort::testVolLow(){
     qDebug()<<"testVolLow: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_LOW;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //故障清除测试
 void Ctrl_SerialPort::testVolClear(){
     qDebug()<<"testVolClear: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_VOL_CLEAR;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }
 //休眠唤醒测试
 void Ctrl_SerialPort::testWakeup(){
     qDebug()<<"testWakeup: ";
     mTestCmd = enTest_cmd::AR_KEY_TEST_WAKEUP;
     uint8_t cmd_data[5] = {0};
     cmd_data[0] = 0x00;
     serial_cmd_pack(mTestCmd,cmd_data);
 }

 void Ctrl_SerialPort::serial_cmd_pack(uint8_t cmd,uint8_t *data)
 {
     QByteArray sendData = 0;
     sendData.resize(8);
     sendData[0] = enFrame_head::AR_KEY_SEND_HEAD;
     sendData[1] = cmd;
     sendData[2] = *data;
     sendData[3] = *(data+1);
     sendData[4] = *(data+2);
     sendData[5] = *(data+3);
     sendData[6] = *(data+4);
     sendData[7] = (sendData[0]+sendData[1]+sendData[2]+sendData[3]+sendData[4]+sendData[5]+sendData[6]+sendData[7])&0xff;
    // qDebug() << "=====write sendData:" << sendData;
     emit receive_data(m_portId,sendData);
     QString str_temp = sendData.toHex().toUpper();
     QString str_show;
     for(int i = 0; i<str_temp.length (); i+=2)
     {
         str_show += str_temp.mid (i,2);
         str_show += " ";
     }
    // qDebug() << "str_show:" << str_show;
     QTime time = QTime::currentTime();
     QString time_strting = time.toString ("hh:mm:ss:msms");
     qDebug()<<time_strting<< "write sendData:" << str_show;
   //  qDebug()<< "write"<<QThread::currentThreadId();
     m_SerialPort->write(sendData,8);
 }

 void Ctrl_SerialPort::retrySendCommand() {
     if(!mStartTest) {
         qDebug() << "===retrySendCommand return 2222";
         return;
     }
     qDebug() << "===retrySendCommand mTestCmd"<<mTestCmd;
     switch(mTestCmd) {
     case AR_KEY_TEST_START: {
         //testStart();
         emit run_case_sig(true,AR_KEY_TEST_START,0);
     }break;
     case AR_KEY_TEST_VOL_NORMAL:{
         if(mVolTestType == V9_NORMAL) {
             emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V9_NORMAL);
            // testVol9Normal();
         }else if(mVolTestType == V13_NORMAL) {
             emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V13_NORMAL);
             //testVol13Normal();
         }else if(mVolTestType == V15_NORMAL) {
             emit run_case_sig(true,AR_KEY_TEST_VOL_NORMAL,V15_NORMAL);
            // testVol15Normal();
         }

     }break;
     case AR_KEY_TEST_VOL_IDLE:{
         if(mVolTestType == V9_IDLE) {
             //testVol9Idle();
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V9_IDLE);
         }else if(mVolTestType == V13_IDLE) {
            // testVol13Idle();
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V13_IDLE);
         }else if(mVolTestType == V15_IDLE) {
            // testVol15Idle();
             emit run_case_sig(true,AR_KEY_TEST_VOL_IDLE,V15_IDLE);
         }
     }break;
     case AR_KEY_TEST_HARDWARE_VER:{
         //testHardwareVer();
         emit run_case_sig(true,AR_KEY_TEST_HARDWARE_VER,0);
     }break;
     case AR_KEY_TEST_SOFTWARE_VER:{
         //testSoftwareVer();
         emit run_case_sig(true,AR_KEY_TEST_SOFTWARE_VER,0);
     }break;
     case AR_KEY_TEST_PRODUCT_NUB:{
         if(mProductNum == 0) {
            // testProductNumFirst();
             emit run_case_sig(true,AR_KEY_TEST_PRODUCT_NUB,1);
         }else {
            // testProductNumSecond();
             emit run_case_sig(true,AR_KEY_TEST_PRODUCT_NUB,2);
         }

     }break;
     case AR_KEY_TEST_LOGO:{
         //testLogo();
         emit run_case_sig(true,AR_KEY_TEST_LOGO,0);
     }break;
     case AR_KEY_TEST_TOF:{
          if(mTofFunc == TOF_OPEN){
              emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_OPEN);
              //testTofOpenned();
          }else {
              emit run_case_sig(true,AR_KEY_TEST_TOF,TOF_READ_VALUE);
             // testTofReadValue();
          }
     }break;
     case AR_KEY_TEST_VOL_HIGH:{
          emit run_case_sig(true,AR_KEY_TEST_VOL_HIGH,0);
          //testVolHigh();
     }break;
     case AR_KEY_TEST_VOL_LOW:{
         //testVolLow();
         emit run_case_sig(true,AR_KEY_TEST_VOL_LOW,0);
     }break;
     case AR_KEY_TEST_VOL_CLEAR:{
         //testVolClear();
         emit run_case_sig(true,AR_KEY_TEST_VOL_CLEAR,0);
     }break;
     }
 }

 void Ctrl_SerialPort::initTestCaseData(QString projectName) {
     dataReaded = false;
     mOnlyClearData = false;
     mVolClearChecked = NOT_CHECKED;
     mTofDataReadTime = 0;
     mByteArray.clear();
     //product_num_value_1 = 0;
     //product_num_value_2 = 0;
     mTestCmd = AR_KEY_NO_CMD;
     mTestType = EOL_TEST;
     mTestProject = projectName;
     mVolTestType = V9_NORMAL;
     mTofFunc = TOF_OPEN;
     mProductNum = 0;
     mTofChecked= NOT_CHECKED;
     mCommandException = false;
 }
