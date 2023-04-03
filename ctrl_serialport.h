#ifndef CTRL_SERIALPORT_H
#define CTRL_SERIALPORT_H

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

class Ctrl_SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit Ctrl_SerialPort(int portId, QString port,QObject *parent = nullptr);
    virtual ~Ctrl_SerialPort();
    void show_fun_id();
    void init_port(QString port);//初始化串口
    QSerialPort * getSerialPort();
    bool checkPortOpen();

signals:
    void run_case_sig(bool start, int cmd,int case_item);
    void receive_data(int portId,QByteArray buff);//向主线程发送接收到的串口数据
   // void test_start(int started);
   // void test_vol_normal(int volTestType,int current,int voltage );
   // void test_vol_sleep(int volTestType,int current,int voltage );
   // void test_vol_idle(int volTestType,int current,int voltage );
   // void test_temp(int voltage);
  //  void test_hw(QString hw);
   // void test_sw(QString sw);
    //void test_productNum(QString prodNum);
    //void test_vol_high(int flag);
   // void test_vol_low(int flag);
   // void test_vol_clear(int flag);
    //void test_logo_opened(int opened);
   // void test_tof_opened(int opened,int distance);
 //   void test_tof_read_value(int distance, int tofFlag);
   // void test_wakeup(int current_value,int voltage_value,int wakeup_flag);
    void show_serial_data(int cmd, int volTestType,int current_value,int voltage_value,int flag,int tofFunc,int distance,QString info);
    void exception_data(int port,QString info);
public slots:
    void write_data(int portId,QByteArray buff);//写串口数据 /*QByteArray*/
    void closePort(int portId);
    void startCtrlPortTest(bool start,int case_item,int testType, QString testProject);
    void tof_test_start(int flag);
    void read_plc_value(int flag);
    void start_run_test_case(int cmd,int case_num);

private slots:
    void show_slots_id();
    void read_data();//读取串口数据
    void runTestCase(bool start, int cmd,int case_item );

private:
    void handleReadData(QByteArray buff);
    void parseReadData(QString buff);
    void parseReadData_temp(QString buff);
    void serial_cmd_pack(uint8_t cmd,uint8_t *data);
    void testStart();
    void testVol9Normal();
    void testVol13Normal();
    void testVol15Normal();
    void testVol9Sleep();
    void testVol13Sleep();
    void testVol15Sleep();
    void testVol9Idle();
    void testVol13Idle();
    void testVol15Idle();
    void testTemp();
    void testHardwareVer();
    void testSoftwareVer();
    void testProductNumFirst();
    void testProductNumSecond();
    void testLogo();
    void testTofOpenned();
    void testTofReadValue();
    void testVolHigh();
    void testVolLow();
    void testVolClear();
    void testWakeup();
    void retrySendCommand();
    void initTestCaseData(QString projectName);
private:
    QThread *my_thread = NULL;
    QSerialPort *m_SerialPort = NULL;//实例化一个指向串口的指针，用于访问串口
    int m_portId;
    QTimer *m_readTimer = NULL;
    QTimer *m_sendTimer = NULL;
    bool mPortOpened = false;
    uint8_t mTestCmd = AR_KEY_NO_CMD;
    uint8_t mTestType = EOL_TEST;
    QString mTestProject = "C03";
    int mVolTestType = V9_NORMAL;
    int mTofFunc = TOF_OPEN;
    int mProductNum = 0;
    QByteArray mByteArray = 0;
   // uint16_t normal_current_value;
   // uint16_t normal_voltage_value;
  //  uint16_t sleep_current_value;
  //  uint16_t sleep_voltage_value;
  //  uint16_t idle_current_value;
   // uint16_t idle_voltage_value;
   // uint16_t temp_voltage_value;
   // QString hw;
  //  QString sw;
    QString product_num_value_1 = 0;
    QString product_num_value_2 = 0;
  //  uint8_t logo_openned;
  //  uint8_t tof_openned;
  //  uint8_t tof_open_value;
  //  uint8_t vol_high_normal_flag;
   // uint8_t vol_low_normal_flag;
  //  uint8_t vol_clear_flag;
  //  uint16_t wakeup_current_value;
  //  uint16_t wakeup_voltage_value;
  //  uint8_t wakeup_flag;
    int cun;
    int mTofDataReadTime = 0;
    caseCheckedState mVolClearChecked= NOT_CHECKED;
    caseCheckedState mTofChecked= NOT_CHECKED;
    bool dataReaded = false;
    bool mCommandException = false;
    QMutex mutex;
    //int readTimeout = 60*1000;
    int readTimeout = 20*1000;
    bool mStartTest = false;
    bool mOnlyClearData = false;

};

#endif // CTRL_SERIALPORT_H
