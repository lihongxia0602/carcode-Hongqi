#ifndef PCL_SERIALPORT_H
#define PCL_SERIALPORT_H

#include <QObject>

#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QString>
#include <QThread>
#include <QByteArray>
#include <QTimer>
#include "datatypedef.h"
#include <QMutex>
#include <QMutexLocker>

class Pcl_SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit Pcl_SerialPort(int portId, QString port,QObject *parent = nullptr);
    virtual ~Pcl_SerialPort();
    void show_fun_id();
    void init_port(QString port);//初始化串口
    QSerialPort * getSerialPort();
    bool checkPortOpen();
signals:
    void thread_sig();
    void receive_data(int portId,QByteArray buff);//向主线程发送接收到的串口数据
    void switch_to_ctrl(int flag);
    void exception_data(int port,QString info);
    void send_start_button_state(bool flag);
public slots:
    void write_data(int portId,QByteArray buff);//写串口数据 /*QByteArray*/
    void closePort(int portId);
    void write_plc_data(int set);

private slots:
    void show_slots_id();
    void read_data();//读取串口数据

private:
    void sendR1Set();
    void sendR2Set();
    void sendR3Set();
    void sendR11Set();
    void sendR12Set();
    void sendR1Reset();
    void sendR2Reset();
    void sendR3Reset();
    void sendR11Reset();
    void sendR12Reset();
    void readRegisterData(int command);
    void initTestCaseData();
    bool parseReceiveData(QByteArray buff);
private:
    QThread *my_thread = NULL;
    QSerialPort *m_SerialPort = NULL;//实例化一个指向串口的指针，用于访问串口
    int m_portId;
    QTimer *m_readTimer = NULL;
    bool mPortOpened = false;
    int cun;
    int mPlcCommand = PLC_CMD_NULL;
    int mReadRegister = PLC_READ_NULL;
    bool dataReaded = false;
    bool mCommandException = false;
    bool mStart = false;
    QMutex mutex;
    //int readTimeout = 2000;
    int readTimeout = 5000;
    QByteArray mByteArray = 0;

};

#endif // PCL_SERIALPORT_H
