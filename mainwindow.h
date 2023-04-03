#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datatypedef.h"
#include "ctrl_serialport.h"
#include "ccd_serialport.h"
#include "pcl_serialport.h"
#include "ims_serialport.h"
#include<QTime>
#include <QMutex>
#include <QMutexLocker>
#include <QList>
#include <QLabel>
#include <QAbstractNativeEventFilter>
#include "switchcontrol.h"
#include <QLineEdit>
#include<QCalendarWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QString name, QString pwd,QWidget *parent = nullptr);
    ~MainWindow();
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    // uint16_t normal_voltage_value;
  //  uint16_t sleep_current_value;
 //   uint16_t sleep_voltage_value;
 //   uint16_t idle_voltage_value;
  //  uint16_t temp_voltage_value;
     int mNormal_current_value_9V = 0;
     int mNormal_current_value_13V = 0;
     int mNormal_current_value_15V = 0;
     float mIdle_current_value_9V = 0.0;
     float mIdle_current_value_13V = 0.0;
     float mIdle_current_value_15V = 0.0;
     float mSleep_current_value_9V = 0.0;
     float mSleep_current_value_13V = 0.0;
     float mSleep_current_value_15V = 0.0;
     QString mHw = "0";
     QString mSw = "0";
     QString mProduct_Num;
     int mNotTofDistance = 0;
     int mTofDistance = 0;
   // QString product_num_value_1;
   // QString product_num_value_2;
  //  uint8_t logo_openned;
  //  uint8_t tof_openned;
   // uint8_t tof_open_value;
  //  uint8_t vol_high_normal_flag;
  //  uint8_t vol_low_normal_flag;
  //  uint8_t vol_clear_flag;
  //  uint16_t wakeup_current_value;
 //   uint16_t wakeup_voltage_value;
 //   uint8_t wakeup_flag;
    void serial_cmd_pack(uint8_t cmd,uint8_t *data);
    void testStart();
    void doEOLTest();
    void doBoardTest();
    void doFunctionTest();
    void sendCcdCommand();
    void parseCcdData(QString buffer);
    void parsePclData(QString buffer);

    caseCheckedState mVolNormalChecked= NOT_CHECKED;
    caseCheckedState mVol9NormalChecked = NOT_CHECKED;
    caseCheckedState mVol13NormalChecked= NOT_CHECKED;
    caseCheckedState mVol15NormalChecked= NOT_CHECKED;
    caseCheckedState mVolSleepChecked= NOT_CHECKED;
    caseCheckedState mVol9SleepChecked= NOT_CHECKED;
    caseCheckedState mVol13SleepChecked= NOT_CHECKED;
    caseCheckedState mVol15SleepChecked= NOT_CHECKED;
    caseCheckedState mVolIdleChecked= NOT_CHECKED;
    caseCheckedState mVol9IdleChecked= NOT_CHECKED;
    caseCheckedState mVol13IdleChecked= NOT_CHECKED;
    caseCheckedState mVol15IdleChecked= NOT_CHECKED;
    caseCheckedState mTempVolChecked= NOT_CHECKED;
    caseCheckedState mHwChecked= NOT_CHECKED;
    caseCheckedState mSwChecked= NOT_CHECKED;
    caseCheckedState mProductNumFirstChecked= NOT_CHECKED;
    caseCheckedState mProductNumSecondChecked= NOT_CHECKED;
    caseCheckedState mTofOpennedChecked= NOT_CHECKED;
    caseCheckedState mTofReadValueChecked= NOT_CHECKED;
    caseCheckedState mPclChecked= NOT_CHECKED;
    caseCheckedState mLogoChecked= NOT_CHECKED;
    caseCheckedState mCcdChecked= NOT_CHECKED;
    caseCheckedState mVolHighChecked= NOT_CHECKED;
    caseCheckedState mVolLowChecked= NOT_CHECKED;
    caseCheckedState mVolClearChecked= NOT_CHECKED;
    caseCheckedState mWakeupChecked= NOT_CHECKED;

signals:
   void startTest(bool start,int case_item,int testType, QString testProject);
   void startTofTest(int flag);
   void writePort_sig(int portId,QByteArray buff);
   void writeIMS_sig(int requestCode,QString barcode,int result);
   void closePort_sig(int portId);
   void send_plc_command(int set);
   void readPlcValue(int flag);
   void runTestCase(int cmd,int case_item);

private slots:
   void on_pushButtonCtrl_clicked();
   void on_pushButtonCcd_clicked();
   void on_pushButtonPcl_clicked();
   void on_pushButtonims_clicked();
   void on_pushButtonBoardTest_clicked();
   void on_pushButtonFuncTest_clicked();
   void on_pushButtonEolTest_clicked();
   void on_pushButtonStartTest_clicked();
   void on_pushButtonSave_clicked();
   void on_pushButtonSaveSetting_clicked();
   void on_pushButtonChooseAll_clicked();
   void on_pushButtonClearAll_clicked();
   void on_pushButtonTestResult_clicked();
   void on_pushButtonImportXml_clicked();
   void on_tabWidget_currentChanged();
   void on_pushButtonEndTest_clicked();
   void on_pushButton_search_clicked();
   void onToggled(bool bChecked);

private slots:
   void plc_set_finished(int flag);
   void handle_ctrl_serial_data(int cmd, int volTestType,int current_value,int voltage_value,int flag,int TofFunc,int distance,QString info);
   void receive_exception_data(int port,QString info);
   void testStatusTrack();
   bool checkCcd(int flag,QString diameter, QString brightness, QString distortion);
   void notifySendPlcCommand();
   //multi thread transfer data
   void showSerialPortData(int portId,QByteArray buff);//用于显示数据
   void checkStartButtonState();
   void start_button_state(bool flag);
   void recUsrname(QString name);
   void checkImsData(int requestCode,QString data);
   void showTime();
   void setDate();
private:
    Ui::MainWindow *ui;
    QString mUser_Name;
    QString mUser_Pwd;
    long sendNum=0, recvNum=0, tSend=0, tRecv=0;// 发送/接收数量，历史发送/接收数量，Byte
    long sendRate=0, recvRate=0;// 发送/接收速率，Byte/s
    long recvFrameNum=0, recvFrameRate=0, recvErrorNum=0, tFrame=0;
    QLabel *lblSendNum, *lblRecvNum, *lblSendRate, *lblRecvRate, *lblRecvFrameNum, *lblFrameRate;
    // 发送速率、接收速率统计-定时器
    QTimer *timRate;
    QTimer *mTimeSendPlcR2 = NULL;
    QTimer *mTimeStartTest = NULL;
    int mTimeStartTimeout = 200;
    int mTimeSendR2Timeout = 1000;
    uint16_t TreadFunc_Time = 0;
    uint8_t  TreadFunc_Time_Flag = 0;
    //Use for Multi-port conected
    Ctrl_SerialPort *Port_ctrl = NULL;
    Ccd_SerialPort *Port_ccd = NULL;
    Pcl_SerialPort *Port_pcl =NULL;
    Ims_SerialPort *Port_ims =NULL;
    QStringList getPortNameList();
    QStringList m_portNameList;
    QTimer *m_showTimer = NULL;
    QSerialPort *currentSerialPort;
    QTime mTestTime; //记录总时间
    QMutex mutex;
    QString mDisplayLogData = 0;
    QString mFileName;
    int mPassNum = 0 ;
    int mFailedNum = 0;
    int TestTimeout = 45*1000;
   // int TestTimeout = 2*1000;
    QList<TestResultEntry> mTestResultList;
    // Api according to EOL protocol file
    uint8_t mTestCmd = AR_KEY_NO_CMD;
    int mTestType = EOL_TEST;
    int mTestProject = C03;
    QByteArray mRecBuffer;
    QStringList mProjectList;
    QString mStr_dir;
    QString mTestResultFileName;
    QString mTestResultFileName_csv;
    QString mTotalCountFileName;
    bool mTestRunning = false;
    QString mParamSetFilePath = NULL;
    QString mSN = "0";
    QString mDiameter = "0";
    QString mBrightness="0";
    QString mDistortion = "0";
    int mCurrentTestCase = -1;
    bool mToggleSwitchChecked = false;
    SwitchControl * MRSwitchControl = NULL;
    TestResultEntry mEntry;
    QString mTestResultFile;
    QLabel *label;
    QLineEdit *lineEdit;
    QCalendarWidget *calendarWidget;
private:
    void startEOLTest();
    bool checkTestStarted(int started);
    bool checkVolNormal(int volTestType,int current_value,int voltage_value);
    bool checkVolSleep(int volTestType,int current_value,int voltage_value);
    bool checkVolIdle(int volTestType,int current_value,int voltage_value);
    bool checkTempVoltage(int voltage_value);
    bool checkHwInfo(QString hw);
    bool checkSwInfo(QString sw);
    bool checkProductNum(QString prodNum);
    bool checkLogoOpened(int logo_opened);
    bool checkTofOpened(int tof_opened,int distance);
    bool checkTofReadValue(int distance, int tofFlag);
    bool checkVolHigh(int normal_flag);
    bool checkVolLow(int normal_flag);
    bool checkVolClear(int clear_flag);
    bool checkWakeup(int current_value,int voltage_value,int wakeup_flag);
    void dataRateCalculate(void);
    void setNumOnLabel(QLabel *lbl, QString strS, long num);
    //used for save logs
    void writeTXT(const char *str_data);
    void openTxt();
    QString generateSN();
    void importXmlData(QString filepath);
    void setAllSettingStatus(bool status);
    void testCaseItemChoose(bool enabled);
    void testResultSet();
    void sendTestResultToIMS();
    void InitSetting();
    void InitTestCase();
    void writeTestResult();
    void createSaveResultFolder();
    QString generateRandData();
    void startIMSProcess(int requestCode,QString barcode,int result);
	void showToggleSwitch();
    void writeDataToJson(int num);
    void modifyDataToJson(int num);
    void readDataFromJson();
    QString intToFloat(int value);
    QList<QStringList> ReadFromCSV(QString PathName);
    void writeCSVFile(const char *str_data);
    void showCalendarWidget();
    bool checkSNValid(QString sn);
};
#endif // MAINWINDOW_H
