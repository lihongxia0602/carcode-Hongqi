#ifndef CCD_SERIALPORT_H
#define CCD_SERIALPORT_H

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
class Ccd_SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit Ccd_SerialPort(int portId, QString port,QObject *parent = nullptr);
    virtual ~Ccd_SerialPort();
    void show_fun_id();
    void init_port(QString port);//初始化串口
    QSerialPort * getSerialPort();
    bool checkPortOpen();
    void initTestCaseData();
    void parseReadData(QByteArray buff);

signals:
    void thread_sig();
    void receive_data(int flag,QString diameter, QString brightness, QString distortion);//向主线程发送接收到的串口数据
    void receive_string_data(int portId,QByteArray buff);
    void exception_data(int port,QString info);

public slots:
    void write_data(int portId,QByteArray buff);//写串口数据 /*QByteArray*/
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
    int readTimeout = 6000;
    bool mStartTest = false;
    int cun;
    QByteArray mCcdCommand;
    bool mCommandException = false;
    QByteArray mByteArray = 0;

};

#endif // CCD_SERIALPORT_H
