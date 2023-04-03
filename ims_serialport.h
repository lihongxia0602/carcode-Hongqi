#ifndef IMS_SERIALPORT_H
#define IMS_SERIALPORT_H

#include <QObject>

#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QString>
#include <QThread>
#include <QByteArray>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include "datatypedef.h"
class Ims_SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit Ims_SerialPort(int portId, QString port,QObject *parent = nullptr);
    virtual ~Ims_SerialPort();
    void show_fun_id();
    void init_port(QString port);//初始化串口
    QSerialPort * getSerialPort();
    bool checkPortOpen();
    void initTestCaseData();
    void requestBarCode();
    void sendTestResult(QString barcode,int resultCode);   //1:testPass 0:testFailed
    void parseReceiveData(QByteArray buff);

signals:
    void thread_sig();
    void receive_data(int requestCode,QString data);//向主线程发送接收到的串口数据
    void receive_string_data(int portId,QByteArray buff);
    void send_exception_data(int port,QString info);

public slots:
    void write_data(int requestCode,QString barcode,int result);//写串口数据 /*QByteArray*/
    void closePort(int portId);

private slots:
    void show_slots_id();
    void read_data();//读取串口数据

private:
    QThread *my_thread = NULL;
    QSerialPort *m_SerialPort = NULL;//实例化一个指向串口的指针，用于访问串口
    int m_portId;
    QTimer *m_readTimer = NULL;
    bool mPortOpened = false;
    bool dataReaded = false;
    QMutex mutex;
    int readTimeout = 4000;
    bool mStartTest = false;
    int cun;
    QByteArray mCcdCommand;
    int mRequestState = IMS_DEFAULT;
    bool mCommandException = false;
    QByteArray mByteArray = 0;

};

#endif // IMS_SERIALPORT_H
