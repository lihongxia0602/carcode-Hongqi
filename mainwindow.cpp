#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include "readwritexml.h"
#include <string.h>
#include <windows.h>
#include "toast.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "readwritecsv.h"
static int mTotalTestItems = 0;
static int mTotalPassItems = 0;
static int mTotalTestNum = 0;
MainWindow::MainWindow(QString name, QString pwd,QWidget *parent)
    :mUser_Name(name),mUser_Pwd(pwd), QMainWindow(parent), ui(new Ui::MainWindow)
{
    qDebug() << mUser_Name << mUser_Pwd ;
    mPassNum = 0;
    mFailedNum = 0;
    ui->setupUi(this);
    //setWindowTitle("ASU ARLEOL测试工具2.0");
    ui->tabWidget->setTabText(0, "测试");
    ui->tabWidget->setTabText(1, "设置");
    ui->tabWidget->setCurrentIndex(1);
    InitSetting();
    ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
    ui->label_5->setStyleSheet("background-color: rgb(0, 217, 0)");
    ui->tabWidget->usesScrollButtons();
    //ui->pushButtonEndTest->setVisible(false);
    QStatusBar *sBar = statusBar();
    // 状态栏的收、发计数标签
    lblSendNum = new QLabel(this);
    lblRecvNum = new QLabel(this);
    lblSendRate = new QLabel(this);
    lblRecvRate = new QLabel(this);
    // 设置标签最小大小
    lblSendNum->setMinimumSize(100, 20);
    lblRecvNum->setMinimumSize(100, 20);
    lblSendRate->setMinimumSize(100, 20);
    lblRecvRate->setMinimumSize(100, 20);
    //sBar->addPermanentWidget(lblSendNum);
    //sBar->addPermanentWidget(lblSendRate);
   // sBar->addPermanentWidget(lblRecvNum);
   // sBar->addPermanentWidget(lblRecvRate);
    lblRecvFrameNum = new QLabel(this);
    lblFrameRate = new QLabel(this);
    lblRecvFrameNum->setMinimumSize(100, 20);
    lblFrameRate->setMinimumSize(80, 20);
    setNumOnLabel(lblRecvFrameNum, "FNum: ", recvFrameNum);
    setNumOnLabel(lblFrameRate, "FPS: ", recvFrameRate);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblRecvFrameNum);
    sBar->addPermanentWidget(lblFrameRate);
    timRate = new QTimer;
    //timRate->start(1000);
    connect(timRate, &QTimer::timeout, this, [=](){
        dataRateCalculate();
    });
    m_portNameList = getPortNameList();

    ui->comboBoxCtrl->addItems(m_portNameList);
    ui->comboBoxCcd->addItems(m_portNameList);
    ui->comboBoxPcl->addItems(m_portNameList);
    ui->myComboBoxIMS->addItems(m_portNameList);
    for(int i = 0; i< m_portNameList.size();++i)
    {
        QString tmp = m_portNameList.at(i);
        if(tmp.contains("Prolific USB")) {
            ui->comboBoxCcd->setCurrentIndex(i);
        }
    }
    //if(ui->comboBoxPcl->count()>=3)
   // {
       // ui->comboBoxCtrl->setCurrentIndex(0);
       // ui->comboBoxCcd->setCurrentIndex(1);
      //  ui->comboBoxPcl->setCurrentIndex(2);
    //}
    m_showTimer = new QTimer(this);
    connect(m_showTimer, SIGNAL(timeout()), this, SLOT(testStatusTrack()));
   // m_showTimer->start(10);/*开启定时器，并且每10ms后询问一次。
                            //定时的时间一到，马上产生timeout（）信号，继续执行自定义槽函数*/
    mDisplayLogData.clear();
    mTestResultList.clear();
    ui->PlainTextEdit_show->clear();
    ui->lineEdit_total_test->clear();
    ui->lineEdit_pass_num->clear();
    ui->lineEdit_total_test->setText("0");
    ui->lineEdit_pass_num->setText("0");
    if(mUser_Name.compare("administrator")== 0) {
         ui->pushButtonSaveSetting->setHidden(false);
         ui->pushButtonSaveSetting->setEnabled(false);
    } else {
         ui->pushButtonSaveSetting->setHidden(true);
    }

    //ui->pushButtonSave->setHidden(true);
    //ui->pushButton9VolSleep->setHidden(true);
    ui->comboBoxProject->clear();
    //ui->comboBoxProject->addItem("C206");
  //  ui->comboBoxProject->addItem("C03");
   // ui->comboBoxProject->setCurrentText("G59");
    mTimeSendPlcR2 = new QTimer(this);
    connect(mTimeSendPlcR2, SIGNAL(timeout()), this, SLOT(notifySendPlcCommand()));
   // createSaveResultFolder();
    mTestRunning = false;
    mSN.clear();
    if(ui->tabWidget->currentIndex() == 1) {
         showToggleSwitch();
    }

    if(ui->tabWidget->currentIndex() == 2) {
        qDebug() << "ui->tabWidget->currentIndex():" << ui->tabWidget->currentIndex();
        showCalendarWidget();
    }
    readDataFromJson();
    ui->lineEditSN->clear();
   // ui->lineEditSN->setMaxLength(20);
    ui->lineEditSN->setEnabled(false);

}

bool MainWindow::checkSNValid(QString sn) {
    bool ret = false;
    if(ui->comboBoxProject->currentText() == "N60") {
        if(sn.startsWith("10501007") && sn.length() == 20) {
            ret = true;
        }
    }else if(ui->comboBoxProject->currentText() == "N61") {
        if(sn.startsWith("10501008") && sn.length() == 20) {
            ret = true;
        }
    }else if(ui->comboBoxProject->currentText() == "C46") {
        if(sn.startsWith("10501006") && sn.length() == 20) {
            ret = true;
        }
    }
    return ret;
}

void MainWindow::showCalendarWidget(){
    //this->resize(600,500);
    //label = new QLabel(this);
   // label->setText("选择日期");
        //标签以及编辑框位置
   // label->setGeometry(QRect(100,70,100,25));
    //lineEdit = new QLineEdit(this);
   // lineEdit->setGeometry(QRect(180,70,150,25));
        //cursorPositionChanged触发的事件
    ui->label_4->setHidden(true);
    ui->lineEditCalendar->setHidden(true);
    connect(ui->lineEditCalendar,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(showTime()));
        //实例时间控件
    calendarWidget = new QCalendarWidget(this);
        //日历组件位置
   // calendarWidget->setGeometry(1310,110,450,280);
    QHBoxLayout* hLayout = new QHBoxLayout(ui->widget_calendar);
   // hLayout->addStretch();
   hLayout->addWidget(calendarWidget);
        //隐藏日历组件
    calendarWidget->setHidden(true);
        //时间控件点击事件
    connect(calendarWidget,SIGNAL(clicked(QDate)),this,SLOT(setDate()));
}

//槽函数
void MainWindow::showTime()
{
     calendarWidget->setHidden(false);
}

void MainWindow::setDate()
{
     //接收选择时间
     QDate date = calendarWidget->selectedDate();
     //时间格式化
     QString str = date.toString("yyyy-MM-dd");
     ui->lineEditCalendar->setText(str);
     //日历组件隐藏
     calendarWidget->setHidden(true);
}

void MainWindow::showToggleSwitch() {
    MRSwitchControl = new SwitchControl(this);
    QPoint posT = ui->pushButtonims->pos();
    qDebug() << posT.x() << posT.y();
    //MRSwitchControl -> move(1430,160);
    MRSwitchControl->resize(100,30);
    //设置状态、样式
    MRSwitchControl -> setToggle(false);
    ui->myComboBoxIMS->setEnabled(false);
    ui->pushButtonims->setEnabled(false);
    QHBoxLayout* hLayout = new QHBoxLayout(ui->widget_ims);
    hLayout->addStretch();
    hLayout->addWidget(MRSwitchControl);
   // QVBoxLayout* mainLayout = new QVBoxLayout(ui->widget);
    //mainLayout->addStretch();
   // mainLayout->addLayout(hLayout);
    //ui->widget_ims->setEnabled(false);
    //可设置槽函数控制按钮的开关状态
    connect(MRSwitchControl,SIGNAL(toggled(bool)),this,SLOT(onToggled(bool)));

    // 连接信号槽
    //connect(pSwitchControl, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
}

void MainWindow::onToggled(bool bChecked) {
    qDebug() << "bChecked"<< bChecked;
    mToggleSwitchChecked = bChecked;
    if(bChecked) {
        ui->myComboBoxIMS->setEnabled(true);
        ui->pushButtonims->setEnabled(true);
    }else{
        ui->myComboBoxIMS->setEnabled(false);
        ui->pushButtonims->setEnabled(false);
    }
}

void MainWindow::recUsrname(QString name)
{
  //  QString str = QString("欢迎你! %1").arg(name);
   // qDebug() << name ;
  //  ui->label->setText(str);
}

bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        QDateTime current_date_time = QDateTime::currentDateTime();
        MSG *pMsg = reinterpret_cast<MSG*>(message);
       // qDebug() << "++++++++++pMsg->message:"<< pMsg->message << WM_POWERBROADCAST;
        if (pMsg->message == WM_POWERBROADCAST)    //屏幕唤醒
        {
            qDebug() << "++++++++++pMsg->wParam:"<< pMsg->wParam;
            if (pMsg->wParam == PBT_APMSUSPEND) {
                qDebug() << "++++++++++shui mian";
                //处理睡眠后操作
            }
            else if (pMsg->wParam == PBT_APMRESUMEAUTOMATIC)
            {
                qDebug() << "++++++++++++liang ping";
                //处理亮屏后操作
            }

            QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm::ss.zzz");
           // qDebug() << "current_date=" << current_date;
        }
    }
    return false;
}
QStringList MainWindow::getPortNameList(){
    QStringList serialPortName;
    //查找可用的串口
    foreach(const QSerialPortInfo &Info,QSerialPortInfo::availablePorts())//读取串口信息
    {
        QString serialPortInfo = Info.portName() + ": " + Info.description();
        //serialPortName << Info.portName();
        serialPortName << serialPortInfo;
      //  qDebug()<<"portname: "<<Info.description();//调试时可以看的串口信息
    }
    return serialPortName;
}
//onClicked function
void MainWindow::on_pushButtonCtrl_clicked(){
    qDebug() << "on_pushButtonCtrl_clicked";
    QString spTxt = ui->comboBoxCtrl->currentText();
    spTxt = spTxt.section(':', 0, 0);
    if(ui->pushButtonCtrl->text()=="打开串口")
    {
         Port_ctrl = new Ctrl_SerialPort(COM0,spTxt);

         bool portOpened = Port_ctrl->checkPortOpen();
         if(!portOpened) {
             QMessageBox::warning(this,tr("错误"),tr("串口打开失败，请重连"));
             mDisplayLogData.append("串口打开失败，请重连").append("\n");
             return;
         }
        //接收从子线程传输数据的信号
        connect(Port_ctrl,SIGNAL(receive_data(int,QByteArray)),this,SLOT(showSerialPortData(int,QByteArray)));//,Qt::QueuedConnection
        //connect(this,SIGNAL(writePort_sig(int,QByteArray)),Port_ctrl,SLOT(write_data(int,QByteArray)));
        connect(this,SIGNAL(closePort_sig(int)),Port_ctrl,SLOT(closePort(int)));
        connect(this,SIGNAL(startTest(bool,int, int,QString)),Port_ctrl,SLOT(startCtrlPortTest(bool,int,int, QString)));
        connect(this,SIGNAL(startTofTest(int)),Port_ctrl,SLOT(tof_test_start(int)));
        connect(this,SIGNAL(readPlcValue(int)),Port_ctrl,SLOT(read_plc_value(int)));
        connect(this,SIGNAL(runTestCase(int,int)),Port_ctrl,SLOT(start_run_test_case(int,int)));
        connect(Port_ctrl,SIGNAL(show_serial_data(int, int,int,int,int,int,int,QString)),this,SLOT(handle_ctrl_serial_data(int, int,int,int,int,int,int,QString)));
        connect(Port_ctrl,SIGNAL(exception_data(int,QString)),this,SLOT(receive_exception_data(int,QString)));
        //关闭设置菜单使能
        ui->comboBoxCtrl->setEnabled(false);
        ui->pushButtonCtrl->setText("关闭串口");
        // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）

    }
    else
    {
        mTestRunning = false;
        //关闭线程
        if(Port_ctrl!=nullptr)
        {
            //todo need confirm why force close if close thread
            emit closePort_sig(COM0);
        }
        //恢复设置菜单使能
        ui->comboBoxCtrl->setEnabled(true);
        ui->pushButtonCtrl->setText("打开串口");
    }
}
void MainWindow::on_pushButtonCcd_clicked(){
    qDebug() << "on_pushButtonCcd_clicked";
    QString spTxt = ui->comboBoxCcd->currentText();
    spTxt = spTxt.section(':', 0, 0);
    if(ui->pushButtonCcd->text()=="打开串口")
    {
         Port_ccd = new Ccd_SerialPort(COM1,spTxt);
         bool portOpened = Port_ccd->checkPortOpen();
         if(!portOpened) {
             QMessageBox::warning(this,tr("错误"),tr("串口打开失败，请重连"));
             mDisplayLogData.append("CCD串口打开失败，请重连").append("\n");
             return;
         }
        //接收从子线程传输数据的信号
        connect(Port_ccd,SIGNAL(receive_data(int,QString, QString, QString)),this,SLOT(checkCcd(int,QString, QString, QString)));//,Qt::QueuedConnection
        connect(Port_ccd,SIGNAL(receive_string_data(int,QByteArray)),this,SLOT(showSerialPortData(int,QByteArray)));
        connect(this,SIGNAL(writePort_sig(int,QByteArray)),Port_ccd,SLOT(write_data(int,QByteArray)));
        connect(Port_ccd,SIGNAL(exception_data(int,QString)),this,SLOT(receive_exception_data(int,QString)));
        connect(this,SIGNAL(closePort_sig(int)),Port_ccd,SLOT(closePort(int)));
        //关闭设置菜单使能
        ui->comboBoxCcd->setEnabled(false);
        ui->pushButtonCcd->setText("关闭串口");
        // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）

    }
    else
    {
        mTestRunning = false;
        //关闭线程
        if(Port_ccd!=nullptr)
        {
            //todo need confirm why force close if close thread
            emit closePort_sig(COM1);
        }
        //恢复设置菜单使能
        ui->comboBoxCcd->setEnabled(true);
        ui->pushButtonCcd->setText("打开串口");
    }
}
void MainWindow::on_pushButtonPcl_clicked(){
    qDebug() << "on_pushButtonPcl_clicked";
    QString spTxt = ui->comboBoxPcl->currentText();
    spTxt = spTxt.section(':', 0, 0);
    qDebug() << "on_pushButtonPcl_clicked" << spTxt;
    if(ui->pushButtonPcl->text()=="打开串口")
    {
         Port_pcl = new Pcl_SerialPort(COM2,spTxt);
         bool portOpened = Port_pcl->checkPortOpen();
         if(!portOpened) {
             QMessageBox::warning(this,tr("错误"),tr("串口打开失败，请重连"));
             mDisplayLogData.append("plc串口打开失败，请重连").append("\n");
             return;
         }
        //接收从子线程传输数据的信号
        if(Port_pcl != nullptr) {
            connect(Port_pcl,SIGNAL(receive_data(int,QByteArray)),this,SLOT(showSerialPortData(int,QByteArray)));//,Qt::QueuedConnection
       // connect(this,SIGNAL(writePort_sig(int,QByteArray)),Port_pcl,SLOT(write_data(int,QByteArray)));
            connect(Port_pcl,SIGNAL(exception_data(int,QString)),this,SLOT(receive_exception_data(int,QString)));
            connect(this,SIGNAL(send_plc_command(int)),Port_pcl,SLOT(write_plc_data(int)));
            connect(Port_pcl,SIGNAL(switch_to_ctrl(int)),this,SLOT(plc_set_finished(int)));
            connect(Port_pcl,SIGNAL(send_start_button_state(bool)),this,SLOT(start_button_state(bool)));
            connect(this,SIGNAL(closePort_sig(int)),Port_pcl,SLOT(closePort(int)));
        }
        mTimeStartTest = new QTimer(this);
        connect(mTimeStartTest, SIGNAL(timeout()), this, SLOT(checkStartButtonState()));
        //关闭设置菜单使能
        ui->comboBoxPcl->setEnabled(false);
        ui->pushButtonPcl->setText("关闭串口");
        // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）

    }
    else
    {
        mTestRunning = false;
        //恢复设置菜单使能
        ui->comboBoxPcl->setEnabled(true);
        ui->pushButtonPcl->setText("打开串口");
        //关闭线程
        if(Port_pcl!=nullptr)
        {
            //todo need confirm why force close if close thread
            emit closePort_sig(COM2);
        }
    }
}

void MainWindow::on_pushButtonims_clicked(){
    qDebug() << "on_pushButtonims_clicked";
    QString spTxt = ui->myComboBoxIMS->currentText();
    spTxt = spTxt.section(':', 0, 0);
    qDebug() << "on_pushButtonims_clicked" << spTxt;
    if(ui->pushButtonims->text()=="打开串口")
    {
         Port_ims = new Ims_SerialPort(COM3,spTxt);
         bool portOpened = Port_ims->checkPortOpen();
         if(!portOpened) {
             QMessageBox::warning(this,tr("错误"),tr("串口打开失败，请重连"));
             mDisplayLogData.append("IMS串口打开失败，请重连").append("\n");
             return;
         }
        //接收从子线程传输数据的信号
        if(Port_ims != nullptr) {
            connect(this,SIGNAL(writeIMS_sig(int,QString,int)),Port_ims,SLOT(write_data(int,QString,int)));
            connect(Port_ims,SIGNAL(receive_data(int,QString)),this,SLOT(checkImsData(int,QString)));//,Qt::QueuedConnection
            connect(Port_ims,SIGNAL(receive_string_data(int,QByteArray)),this,SLOT(showSerialPortData(int,QByteArray)));
            connect(Port_ims,SIGNAL(send_exception_data(int,QString)),this,SLOT(receive_exception_data(int,QString)));
            connect(this,SIGNAL(closePort_sig(int)),Port_ims,SLOT(closePort(int)));
        }
        //关闭设置菜单使能
        ui->myComboBoxIMS->setEnabled(false);
        ui->pushButtonims->setText("关闭串口");
        // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）

    }
    else
    {
        mTestRunning = false;
        if(ui->pushButtonStartTest->text() == "SN读取中") {
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setEnabled(true);
        }
        //关闭线程
        if(Port_ims != nullptr)
        {
            //todo need confirm why force close if close thread
            emit closePort_sig(COM3);
        }
        //恢复设置菜单使能
        ui->myComboBoxIMS->setEnabled(true);
        ui->pushButtonims->setText("打开串口");
    }
}

void MainWindow::on_pushButtonBoardTest_clicked(){
    mTestType = BOARD_TEST;
    //todo will use after start test
#if 0
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    str.append(">>>>>>>Board Test Start!!!>>>>>>>");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
#endif
    ui->pushButtonBoardTest->setStyleSheet("background-color: rgb(0, 217, 0)");
}
void MainWindow::on_pushButtonFuncTest_clicked(){
    mTestType = FUNC_TEST;
    //todo will use after start test
#if 0
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    str.append(">>>>>>>Func Test Start!!!>>>>>>>");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
#endif
    ui->pushButtonFuncTest->setStyleSheet("background-color: rgb(0, 217, 0)");
}
void MainWindow::on_pushButtonEolTest_clicked(){
    mTestType = EOL_TEST;
    qDebug() << "on_pushButtonEolTest_clicked";
    //sendCcdCommand();
    //emit send_plc_command(1);
   // emit startTofTest(1);
    //todo will use after start test
    //emit startTofTest(1);
    //emit startTest(true,AR_KEY_TEST_START,mTestType,ui->comboBoxProject->currentIndex());
#if 0
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    str.append(">>>>>>>EOL Test Start!!!>>>>>>>");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
#endif
    ui->pushButtonEolTest->setStyleSheet("background-color: rgb(0, 217, 0)");
}

void MainWindow::on_pushButtonStartTest_clicked() {
    qDebug() << "on_pushButtonStartTest_clicked" <<ui->pushButtonSaveSetting->isEnabled();
    qDebug() << "=====on_pushButtonStartTest_clicked" <<mToggleSwitchChecked;
    /*if(ui->lineEditSN->text()=="") {
        QMessageBox::warning(this,tr("提示"),tr("请扫描产品SN码"));
        return;
    }
    if(!checkSNValid(ui->lineEditSN->text())) {
        QMessageBox::warning(this,tr("提示"),tr("请扫描正确的产品SN码"));
        return;
    }*/
    //lhxtemp
    //createSaveResultFolder();
    //startEOLTest();
    //mTotalTestNum++;
    //writeDataToJson(mTotalTestNum);
   // qDebug() << "mTotalTestNum:" << mTotalTestNum;
    if(mTotalTestNum >= 1000) {
        mTotalTestNum = 0;
        writeDataToJson(mTotalTestNum);
        QString det= "请更换插头";
        det = tr("<font size='10' color='white'>") + det;
        det += tr("</font>");
        //QMessageBox::warning(this,tr("提示"),det);
        QMessageBox msgbox(QMessageBox::Warning,"提示",det,QMessageBox::Ok);//将QMessageBox实例化出来
    //下面补上Qss样式表的设置写法即可
        /*msgbox.setStyleSheet("QMessageBox{background-color: rgba(0, 0, 0,50);\
                 border:1px solid #CCFFF6;\
                border-radius:3px;\
                }");*/
        msgbox.setStyleSheet("QMessageBox{background-color: rgba(0, 0, 0,50);\
                             border:1px solid #CCFFF6;\
                            border-radius:3px;\
                            }QLabel{"
                      "min-width:400px;"
                      "min-height:250px; "
                      "font-size:25px;"
                      "}");
        msgbox.exec();
       /* QMessageBox box;
        //设置文本框的大小
        box.setStyleSheet("QLabel{"
                              "min-width:150px;"
                              "min-height:60px; "
                              "font-size:16px;"
                              "}");


        box.setText(QString::fromLocal8Bit("请更换插头"));
        box.setWindowTitle(QString::fromLocal8Bit("警告"));
        box.setIcon(QMessageBox::Icon::Warning );
        box.setButtonText(QMessageBox::Ok , QString::fromLocal8Bit("确定"));
        box.exec();*/
        return;
    }
    if(mSN.compare("") != 0) {
        mSN.clear();
    }
    /*if(m_showTimer != NULL && !m_showTimer->isActive()) {
        m_showTimer->start(TestTimeout);
    }*/

    if(mToggleSwitchChecked) {
        if(ui->widget_ims->isEnabled()) {
            ui->widget_ims->setEnabled(false);
        }
        startIMSProcess(IMS_REQUEST_BARCODE,"", 0);
    } else {
        createSaveResultFolder();
        startEOLTest();
    }

}

void MainWindow::on_pushButtonEndTest_clicked() {
    qDebug() << "on_pushButtonEndTest_clicked";
    if(mTestRunning) {
        mTestRunning = false;
    }

    if(ui->pushButtonStartTest->text() == "SN读取中") {
        //qDebug() << "on_pushButtonEndTest_clicked1111";
        if(Port_ims != NULL) {
          //  qDebug() << "on_pushButtonEndTest_clicked22222";
            emit writeIMS_sig(IMS_DEFAULT,"", 0);
        }
        testStatusTrack();
    } else if(ui->pushButtonStartTest->text() == "测试中") {
        if(Port_ctrl != NULL) {
            //qDebug() << "on_pushButtonEndTest_clicked33333";
            emit startTest(false,AR_KEY_TEST_START,mTestType,ui->comboBoxProject->currentText());
        }
        //if(Port_pcl != NULL) {
          //  emit send_plc_command(PLC_CMD_STOP);
       // }
        testStatusTrack();
    }
}

void MainWindow::on_tabWidget_currentChanged(){
   // qDebug() << "on_tabWidget_currentChanged:" << ui->tabWidget->currentIndex();
    if(ui->tabWidget->currentIndex() == 0) {
        MRSwitchControl->setHidden(true);
        if(mTimeStartTest != NULL) {
            qDebug()<<"ui->tabWidget->currentIndex()==0";
            if(!mTimeStartTest->isActive() && !mTestRunning) {
                if(Port_pcl != NULL) {
                    //lihongxia
                    mTimeStartTest->start(mTimeStartTimeout);
                }

            }

        }

    } else if(ui->tabWidget->currentIndex() == 1) {
        if(MRSwitchControl != NULL) {
             MRSwitchControl->setHidden(false);
        }

       // if(!MRSwitchControl->isToggled()) {
          //  ui->pushButtonims->setEnabled(false);
       // }
        if(mTimeStartTest != NULL) {
            if(mTimeStartTest->isActive()) {
                mTimeStartTest->stop();
            }
        }
        if(mTestRunning) {
            if(mUser_Name.compare("administrator") == 0) {
                setAllSettingStatus(false);
            }
            ui->pushButtonImportXml->setEnabled(false);
            ui->pushButtonCtrl->setEnabled(false);
            ui->pushButtonCcd->setEnabled(false);
            ui->pushButtonPcl->setEnabled(false);
            ui->pushButtonims->setEnabled(false);
            MRSwitchControl->setEnabled(false);
        }else {
            if(mUser_Name.compare("administrator") == 0) {
                setAllSettingStatus(true);
            }
            ui->pushButtonImportXml->setEnabled(true);
            ui->pushButtonCtrl->setEnabled(true);
            ui->pushButtonCcd->setEnabled(true);
            ui->pushButtonPcl->setEnabled(true);

            if(MRSwitchControl != NULL) {
                MRSwitchControl->setEnabled(true);
                if(MRSwitchControl->isToggled()) {
                    ui->pushButtonims->setEnabled(true);
                }
            }
        }

    }else if(ui->tabWidget->currentIndex() == 2){
        showCalendarWidget();
    }
}

void MainWindow::startEOLTest() {
    qDebug() << "=========startEOLTest mTestRunning:" <<mTestRunning;
    if(Port_ctrl == NULL) {
        QMessageBox::warning(this,tr("提示"),tr("请打开控制串口"));
        if(mTimeStartTest != NULL) {
            if(mTimeStartTest->isActive()) {
                mTimeStartTest->stop();
            }
        }
        return;
    }
   // if(Port_ccd == NULL) {
     //   QMessageBox::warning(this,tr("提示"),tr("请打开CCD串口"));
     //   return;
    //}
   // if(Port_pcl == NULL) {
   //     QMessageBox::warning(this,tr("提示"),tr("请打开PLC串口"));
      //  return;
   // }
   // if(mToggleSwitchChecked) {
       // if(Port_ims == NULL) {
         //   QMessageBox::warning(this,tr("提示"),tr("请打开IMS串口"));
         //   return;
      //  }
   // }
    if(ui->comboBoxProject->currentIndex() == -1) {
        QMessageBox::warning(this,tr("提示"),tr("请导入默认参数设置测试项目"));
		if(mTimeStartTest != NULL) {
            if(mTimeStartTest->isActive()) {
                mTimeStartTest->stop();
            }
        }
        return;
    }

    if(m_showTimer != NULL && !m_showTimer->isActive()) {
        m_showTimer->start(TestTimeout);
    }

    if(mTestRunning) {
        return;
    }
    mTestRunning = true;
    ui->pushButtonSaveSetting->setEnabled(false);
    mTotalTestItems++;
    mTotalTestNum++;
    writeDataToJson(mTotalTestNum);
    qDebug() << "=========mTotalTestItems:" <<mTotalTestItems << "--mTotalTestNum:" <<mTotalTestNum;
    openTxt();
    /*if(ui->pushButtonSaveSetting->isEnabled() == true) {
        QMessageBox::warning(this,tr("错误"),tr("先设置再开始测试"));
        return;
    }*/
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    QString projectName = ui->comboBoxProject->currentText();
    str.append(projectName).append(" ").append("EOL测试开始");
    mDisplayLogData.append(str).append("\n");
    writeTXT(mDisplayLogData.toUtf8());
    mDisplayLogData.clear();
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    //m_showTimer->start(TestTimeout);
    InitTestCase();
    ui->lineEdit_total_test->setText(QString::number(mTotalTestItems));
    emit startTest(true,AR_KEY_TEST_START,mTestType,ui->comboBoxProject->currentText());
    ui->pushButtonStartTest->setText("测试中");
    ui->pushButtonStartTest->setEnabled(false);
}

void MainWindow::on_pushButtonSave_clicked() {
    //todo open save窗口
    qDebug() << "on_pushButtonSave_clicked";
    //mTotalTestItems = 0;
   // mTotalPassItems = 0;
    QDateTime ctime = QDateTime::currentDateTime();
    QString str_time = ctime.toString("yyyy-MM-dd-hh-mm-ss");
    QFileDialog fileDialog;
    QString fileName = fileDialog.getSaveFileName(this,tr("Open File"),'/'+str_time+QString("_test_result"),tr("Csv File(*.csv)"));
    if(fileName == "")
    {
        return;
    }
    QFile file(fileName);//可以自己选择路径来保存文件名
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      //  QMessageBox::warning(this,tr("错误"),tr("打开文件失败"));
        mDisplayLogData.append("打开文件失败").append("\n");
        return;
    }
    else
    {
        QTextStream textStream(&file);
      //  QMessageBox::warning(this,tr("提示"),tr("保存文件成功"));
        mDisplayLogData.append("保存文件成功").append("\n");
        file.close();
    }
   // ReadWriteXml *xml = new ReadWriteXml();
    //写数据到xml
   // xml->writeXml(fileName,mTotalTestItems,mTotalPassItems,mTestResultList);
  //  delete xml;
  //  xml = NULL;
    readwritecsv *csv = new readwritecsv();
    csv->writeCSVFileSpec(fileName,mTestResultList);
    delete csv;
    csv = NULL;
    ui->pushButtonCtrl->setEnabled(true);
    ui->pushButtonCcd->setEnabled(true);
    ui->pushButtonPcl->setEnabled(true);
}

void MainWindow::writeTXT(const char *str_data)
{
    if(mFileName == NULL) {
        return;
    }
    QFile files(mFileName);
    if(!files.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)){//杩藉姞鍐欏叆 娣诲姞缁撴潫绗r\n
       //  QMessageBox::warning(this,tr("错误"),tr("打开文件失败,数据保存失败"));
        return ;
    }else{
        files.write(str_data);
        files.flush();
    }
    files.close();
}

void MainWindow::openTxt()
{
    //保存为TXT文件
    QString fileName;

    /*QDateTime ctime = QDateTime::currentDateTime();
    QString str_time = ctime.toString("yyyy-MM-dd");
    QString str_dir;
    if(mTestType==FUNC_TEST) {
        str_dir = "C:/FuncTest/";
    } else if(mTestType == EOL_TEST) {
        str_dir = "C:/EOLTest/";
    } else if(mTestType == DV_TEST) {
        str_dir = "C:/DVTest/";
    }

    str_dir.append(str_time);
      QDir dir(str_dir);
      if(!dir.exists())
      {
          bool ok=dir.mkpath(str_dir);
          if(ok)
          {
             // QMessageBox::warning(this,tr("创建目录"),tr("创建成功"));
              mDisplayLogData.append("创建目录成功").append("\n");
          }
          else{
           //   QMessageBox::warning(this,tr("创建目录"),tr("创建失败"));
              mDisplayLogData.append("创建目录失败").append("\n");
          }
      }*/
    QDateTime curDateTime=QDateTime::currentDateTime();
    QString str_name = mStr_dir+"/Data_%1.txt";
    fileName = QString(str_name).arg(curDateTime.toString("MM-dd-hh-mm"));
    mFileName = fileName;
    QFile files(fileName);
    if(!files.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)){//追加写入 添加结束符\r\n
        // QMessageBox::warning(this,tr("错误"),tr("打开文件失败,数据保存失败"));
        mDisplayLogData.append("打开文件失败,数据保存失败").append("\n");
        return ;
    }else{
//        f.write();
    }
    files.close();
}

void MainWindow::on_pushButtonSaveSetting_clicked(){
     //测试结束这个恢复可用
    ReadWriteXml *xml = new ReadWriteXml();
    ParamSettingEntry entry;
    entry.project_name = ui->comboBoxProject->currentText();
    entry.hw = ui->lineEditHwSet->text();
    entry.sw = ui->lineEditSwSet->text();
  //  entry.produce_num = ui->lineEditProductNumSet->text();
    entry.nottofdistance = ui->lineEditNotTofDis->text();
    entry.tofdistance = ui->lineEditTofDis->text();
    entry.normal_value_9V_min = ui->lineEdit9VNorCurMin->text();
    entry.normal_value_9V_max = ui->lineEdit9VNorCurMax->text();
    entry.normal_value_15V_min = ui->lineEdit15VNorCurMin->text();
    entry.normal_value_15V_max = ui->lineEdit15VNorCurMax->text();
    entry.normal_value_13V_min = ui->lineEdit13VNorCurMin->text();
    entry.normal_value_13V_max = ui->lineEdit13VNorCurMax->text();
    entry.sleep_value_9V_min = ui->lineEdit9VSleepCurMin->text();
    entry.sleep_value_9V_max = ui->lineEdit9VSleepCurMax->text();
    entry.sleep_value_15V_min = ui->lineEdit15VSleepCurMin->text();
    entry.sleep_value_15V_max = ui->lineEdit15VSleepCurMax->text();
    entry.sleep_value_13V_min = ui->lineEdit13VSleepCurMin->text();
    entry.sleep_value_13V_max = ui->lineEdit13VSleepCurMax->text();
    entry.idle_value_9V_min = ui->lineEdit9VIdleCurMin->text();
    entry.idle_value_9V_max = ui->lineEdit9VIdleCurMax->text();
    entry.idle_value_15V_min = ui->lineEdit15VIdleCurMin->text();
    entry.idle_value_15V_max = ui->lineEdit15VIdleCurMax->text();
    entry.idle_value_13V_min = ui->lineEdit13VIdleCurMin->text();
    entry.idle_value_13V_max = ui->lineEdit13VIdleCurMax->text();
    entry.vol_float = ui->lineEditVolFloat->text();
    xml->updateSettingXml(mParamSetFilePath, entry);
    delete xml;
    xml = NULL;
    Toast::instance().show(Toast::INFO, "保存成功！");
}

void MainWindow::InitSetting() {
   // ui->comboBoxProject->addItem("G59");
    //ui->comboBoxProject->setCurrentText("G59");
    ui->pushButtonEolTest->setEnabled(true);
    ui->pushButtonEolTest->setStyleSheet("background-color: rgb(0, 217, 0)");
    testCaseItemChoose(true);
    ui->pushButtonFuncTest->setHidden(true);
    ui->pushButtonBoardTest->setHidden(true);
    mProjectList.clear();
}

void MainWindow::InitTestCase() {
    mPassNum = 0;
    mFailedNum = 0;
   // mSN = "";
    mCurrentTestCase = -1;
    //mDiameter.clear();
    //mBrightness.clear();
   // mDistortion.clear();
    mDiameter = "0" ;
    mBrightness= "0" ;
    mDistortion= "0" ;
    mNormal_current_value_9V = 0;
    mNormal_current_value_13V = 0;
    mNormal_current_value_15V = 0;
    mIdle_current_value_9V = 0.0;
    mIdle_current_value_13V = 0.0;
    mIdle_current_value_15V = 0.0;
    mSleep_current_value_9V = 0.0;
    mSleep_current_value_13V = 0.0;
    mSleep_current_value_15V = 0.0;
   // mHw.clear();
  //  mSw.clear();
     mHw = "0";
     mSw = "0";
   // mProduct_Num.clear();
    mNotTofDistance = 0;
    mTofDistance = 0;
    ui->pushButtonHw->setText("等待测试");
    ui->pushButtonHw->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonSw->setText("等待测试");
    ui->pushButtonSw->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
   // ui->pushButtonProductNum->setText("等待测试");
   // ui->pushButtonProductNum->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonTof->setText("等待测试");
    ui->pushButtonTof->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonLogo->setText("等待测试");
    ui->pushButtonLogo->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
  //  ui->pushButtonTemp->setText("等待测试");
  //  ui->pushButtonTemp->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolLow->setText("等待测试");
    ui->pushButtonVolLow->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolHigh->setText("等待测试");
    ui->pushButtonVolHigh->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
  //  ui->pushButtonWakeup->setText("等待测试");
  //  ui->pushButtonWakeup->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton15VolNormal->setText("等待测试");
    ui->pushButton15VolNormal->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton15VolIdle->setText("等待测试");
    ui->pushButton15VolIdle->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton15VolSleep->setText("等待测试");
    ui->pushButton15VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    //ui->pushButton15VolSleep->setText("不测试");
   // ui->pushButton15VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton13VolIdle->setText("等待测试");
    ui->pushButton13VolIdle->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
  //  ui->pushButton13VolSleep->setText("不测试");
  //  ui->pushButton13VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton13VolNormal->setText("等待测试");
    ui->pushButton13VolNormal->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton13VolSleep->setText("等待测试");
    ui->pushButton13VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton9VolIdle->setText("等待测试");
    ui->pushButton9VolIdle->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
  //  ui->pushButton9VolSleep->setText("不测试");
  //  ui->pushButton9VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton9VolNormal->setText("等待测试");
    ui->pushButton9VolNormal->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton9VolSleep->setText("等待测试");
    ui->pushButton9VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolNormal->setText("等待测试");
    ui->pushButtonVolNormal->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
  //  ui->pushButton9VolSleep->setText("不测试");
  //  ui->pushButton9VolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton9VolIdle->setText("等待测试");
  //  ui->pushButtonVolSleep->setText("不测试");
  //  ui->pushButtonVolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolIdle->setText("等待测试");
    ui->pushButtonVolIdle->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolSleep->setText("等待测试");
    ui->pushButtonVolSleep->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButton9VolIdle->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonVolClear->setText("等待测试");
    ui->pushButtonVolClear->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->pushButtonTestResult->setText("等待测试结果");
    ui->pushButtonTestResult->setStyleSheet("background-color: rgba(214, 214, 214, 255)");
    ui->lineEditHwRet->setText("0");
   // ui->lineEditSN->setText("0");
    ui->lineEditSwRet->setText("0");
    //ui->lineEditPNRet->setText("0");
    ui->TofDisRet->setText("0");
    ui->TofNotDisRet->setText("0");
    ui->lineEdit9VAIdleRet->setText("0");
    ui->lineEdit13VAIdleRet->setText("0");
    ui->lineEdit15VAIdleRet->setText("0");
    ui->lineEdit9VANormalRet->setText("0");
    ui->lineEdit13VANormalRet->setText("0");
    ui->lineEdit15VANormalRet->setText("0");
    ui->lineEdit9VASleepRet->setText("0");
    ui->lineEdit13VASleepRet->setText("0");
    ui->lineEdit15VASleepRet->setText("0");
    ui->TofResponseTime->setText("0");
    ui->diameterRet->setText("0");
    ui->distortionRet->setText("0");
    ui->BrightnessRet->setText("0");
    mVolNormalChecked= NOT_CHECKED;
    mVol9NormalChecked = NOT_CHECKED;
    mVol13NormalChecked= NOT_CHECKED;
    mVol15NormalChecked= NOT_CHECKED;
    mVolSleepChecked= NOT_CHECKED;
    mVol9SleepChecked= NOT_CHECKED;
    mVol13SleepChecked= NOT_CHECKED;
    mVol15SleepChecked= NOT_CHECKED;
    mVolIdleChecked= NOT_CHECKED;
    mVol9IdleChecked= NOT_CHECKED;
    mVol13IdleChecked= NOT_CHECKED;
    mVol15IdleChecked= NOT_CHECKED;
    mTempVolChecked= NOT_CHECKED;
    mHwChecked= NOT_CHECKED;
    mSwChecked= NOT_CHECKED;
    mProductNumFirstChecked= NOT_CHECKED;
    mProductNumSecondChecked= NOT_CHECKED;
    mTofOpennedChecked= NOT_CHECKED;
    mTofReadValueChecked= NOT_CHECKED;
    mPclChecked= NOT_CHECKED;
    mLogoChecked= NOT_CHECKED;
    mCcdChecked= NOT_CHECKED;
    mVolHighChecked= NOT_CHECKED;
    mVolLowChecked= NOT_CHECKED;
    mVolClearChecked= NOT_CHECKED;
    mWakeupChecked= NOT_CHECKED;
}

void MainWindow::testCaseItemChoose(bool enabled){
    ui->checkBoxVolNormalSet->setChecked(enabled);
    ui->checkBoxVolSleepSet->setChecked(enabled);
    ui->checkBoxVolIdleSet->setChecked(enabled);
    ui->checkBoxVolLowSet->setChecked(enabled);
    ui->checkBoxVolHighSet->setChecked(enabled);
    ui->checkBoxVolClearSet->setChecked(enabled);
    ui->checkBoxTofSet->setChecked(enabled);
    ui->checkBoxLogoSet->setChecked(enabled);
    //ui->checkBoxHwSet->setChecked(enabled);
    //ui->checkBoxSwSet->setChecked(enabled);
   // ui->checkBoxProductNumSet->setChecked(enabled);
}

void MainWindow::sendTestResultToIMS() {
    //if(!mTestRunning) return;
    //QString result = "不合格";
  //  if(mTestType == EOL_TEST && ui->comboBoxProject->currentText() == "G59" ) {
        qDebug() << "sendTestResultToIMS mSN:" <<mSN;
		if(mTestType == EOL_TEST) {
			if(ui->comboBoxProject->currentText() == "C206" ) {
		        if(mPassNum == TOTAL_TEST_CASE_EOL_C206) {
		           // ui->pushButtonTestResult->setText("合格");
		            //ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
		           // result = "合格";
		            //mTotalPassItems++;
		           // ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
		            startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
		        } else {
		           // ui->pushButtonTestResult->setText("不合格");
		         //   ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
		            //result = "不合格";
		            startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
		        }
			}else if(ui->comboBoxProject->currentText() == "C03") {
		        if(mPassNum == TOTAL_TEST_CASE_EOL_C03) {
		           // ui->pushButtonTestResult->setText("合格");
		            //ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
		           // result = "合格";
		            //mTotalPassItems++;
		           // ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
		            startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
		        } else {
		           // ui->pushButtonTestResult->setText("不合格");
		         //   ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
		            //result = "不合格";
		            startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
		        }
			}
		}
   // }
}

void MainWindow::testResultSet(){
    qDebug() << "testResultSet";
    //if(!mTestRunning) return;
    if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
        mTimeSendPlcR2->stop();
    }
    if(!ui->widget_ims->isEnabled()) {
        ui->widget_ims->setEnabled(true);
    }
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss.zzz"));
    str.prepend("[");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    ui->PlainTextEdit_show->appendPlainText("testResultSet");
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    mDisplayLogData.append(str).append("testResultSet").append("\n");
    writeTXT(mDisplayLogData.toUtf8());
    mDisplayLogData.clear();
    if(m_showTimer != NULL && m_showTimer ->isActive()) {
        m_showTimer->stop();
    }
    ui->pushButtonStartTest->setText("开始测试");
    ui->pushButtonStartTest->setEnabled(true);
    QString result = "不合格";
    if(mTestType == EOL_TEST) {
         if(ui->comboBoxProject->currentText().compare("C206" ) == 0) {
	        qDebug() << "testResultSet mPassNum:" <<mPassNum << "Failed:" << mFailedNum;
	        if(mPassNum == TOTAL_TEST_CASE_EOL_C206) {
	            ui->pushButtonTestResult->setText("合格");
	            ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
	            result = "合格";
	            mTotalPassItems++;
	            ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
	          //  startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
	        } else {
	            ui->pushButtonTestResult->setText("不合格");
	            ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
	            result = "不合格";
	           // startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
	        }
        }else if(ui->comboBoxProject->currentText().compare("C03") == 0) {
            qDebug() << "testResultSet mPassNum:" <<mPassNum << "Failed:" << mFailedNum;
            if(mPassNum == TOTAL_TEST_CASE_EOL_C03) {
                ui->pushButtonTestResult->setText("合格");
                ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
                result = "合格";
                mTotalPassItems++;
                ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
                //  startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
            } else {
                ui->pushButtonTestResult->setText("不合格");
                ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
                result = "不合格";
                // startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
            }
        }else if(ui->comboBoxProject->currentText().compare("N61") == 0) {
             qDebug() << "testResultSet mPassNum:" <<mPassNum << "Failed:" << mFailedNum;
             if(mPassNum == TOTAL_TEST_CASE_EOL_N61) {
                 ui->pushButtonTestResult->setText("合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
                 result = "合格";
                 mTotalPassItems++;
                 ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
                 //  startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
             } else {
                 ui->pushButtonTestResult->setText("不合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
                 result = "不合格";
                 // startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
             }
         }else if(ui->comboBoxProject->currentText().compare("N60") == 0) {
             qDebug() << "testResultSet mPassNum:" <<mPassNum << "Failed:" << mFailedNum;
             if(mPassNum == TOTAL_TEST_CASE_EOL_N60) {
                 ui->pushButtonTestResult->setText("合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
                 result = "合格";
                 mTotalPassItems++;
                 ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
                 //  startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
             } else {
                 ui->pushButtonTestResult->setText("不合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
                 result = "不合格";
                 // startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
             }
         }else if(ui->comboBoxProject->currentText().compare("C46") == 0) {
             qDebug() << "testResultSet mPassNum:" <<mPassNum << "Failed:" << mFailedNum;
             if(mPassNum == TOTAL_TEST_CASE_EOL_C46) {
                 ui->pushButtonTestResult->setText("合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(0, 217, 0)");
                 result = "合格";
                 mTotalPassItems++;
                 ui->lineEdit_pass_num->setText(QString::number(mTotalPassItems));
                 //  startIMSProcess(IMS_SEND_TEST_RESULT,mSN,1);
             } else {
                 ui->pushButtonTestResult->setText("不合格");
                 ui->pushButtonTestResult->setStyleSheet("background-color: rgb(255, 0, 0)");
                 result = "不合格";
                 // startIMSProcess(IMS_SEND_TEST_RESULT,mSN,0);
             }
         }
	}
    //QString sn = generateSN();
    QString sn = mSN;
    qDebug() <<"--------mSN:" <<sn;
    QDateTime curDateTime= QDateTime::currentDateTime();
    QString datetime = curDateTime.toString("yyyy-MM-dd hh:mm:ss");
    TestResultEntry entry;
    entry.id = mTotalTestItems;
    entry.totalTestItems = mTotalTestItems;
    entry.totalPassItems = mTotalPassItems;
    entry.dateTime = datetime;
    entry.SN = sn;
    entry.result =result;
    entry.normal_current_value_9V = mNormal_current_value_9V;
    entry.normal_current_value_13V = mNormal_current_value_13V;
    entry.normal_current_value_15V = mNormal_current_value_15V;
    entry.idle_current_value_9V = mIdle_current_value_9V;
    entry.idle_current_value_13V = mIdle_current_value_13V;
    entry.idle_current_value_15V = mIdle_current_value_15V;
    entry.sleep_current_value_9V = mSleep_current_value_9V;
    entry.sleep_current_value_13V = mSleep_current_value_13V;
    entry.sleep_current_value_15V = mSleep_current_value_15V;
    entry.hw = mHw;
    entry.sw = mSw;
  //  entry.produce_num = mProduct_Num;
    entry.nottofdistance = mNotTofDistance;
    entry.tofdistance = mTofDistance;
    entry.diamater = mDiameter;
    entry.brightness = mBrightness;
    entry.distortion = mDistortion;
    readwritecsv *csv = new readwritecsv();
    csv->ReadFromCSV(mTestResultFileName_csv);
    //ReadFromCSV(mTestResultFileName_csv);
    QString str_csv;
    str_csv.clear();
    str_csv.append(entry.SN);
    str_csv.append(",");
    str_csv.append(entry.dateTime);
    str_csv.append(",");
    str_csv.append(entry.result);
    str_csv.append(",");
    str_csv.append(entry.hw);
    str_csv.append(",");
    str_csv.append(entry.sw);
    str_csv.append(",");
    str_csv.append(QString::number(entry.nottofdistance,10)).append("cm");
    str_csv.append(",");
    str_csv.append(QString::number(entry.tofdistance,10)).append("cm");
    str_csv.append(",");
    str_csv.append(QString::number(entry.normal_current_value_9V,10)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.normal_current_value_13V,10)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.normal_current_value_15V,10)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.idle_current_value_9V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.idle_current_value_13V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.idle_current_value_15V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.sleep_current_value_9V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.sleep_current_value_13V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(QString::number(entry.sleep_current_value_15V,'f',3)).append("mA");
    str_csv.append(",");
    str_csv.append(entry.diamater).append("mm");
    str_csv.append(",");
    str_csv.append(entry.brightness).append("lux");
    str_csv.append(",");
    str_csv.append(entry.distortion).append("mm");
    str_csv.append("\n");
    csv->writeCSVFile(mTestResultFileName_csv,str_csv.toUtf8());
    delete csv;
    csv = NULL;
    mTestResultList.push_back(entry);
    ReadWriteXml *xml = new ReadWriteXml();
    xml->writeXml(mTestResultFileName,entry);
    delete xml;
    xml = NULL;
    if(Port_pcl != NULL) {
        //lihongxia
        mTimeStartTest->start(mTimeStartTimeout);
    }
    mTestRunning = false;
    ui->pushButtonSaveSetting->setEnabled(true);
    ui->lineEditSN->clear();
}

void MainWindow::writeTestResult(){
    ReadWriteXml *xml = new ReadWriteXml();
    //写数据到xml
    qDebug() << "mTotalTestItems:" << mTotalTestItems << "mTotalPassItems:"<< mTotalPassItems;
    xml->writeXml(mTestResultFileName,mTotalTestItems,mTotalPassItems,mTestResultList);
    delete xml;
    xml = NULL;
}

void MainWindow::createSaveResultFolder(){
    QDateTime ctime = QDateTime::currentDateTime();
    QString str_time = ctime.toString("yyyy-MM-dd");
   // QString str_dir;
    QString str_dir;
    if(mTestType==FUNC_TEST) {
        mStr_dir = QCoreApplication::applicationDirPath()+ "/FuncTest/";
    } else if(mTestType == EOL_TEST) {
        mStr_dir = QCoreApplication::applicationDirPath()+ "/EOLTest/";
    } else if(mTestType == DV_TEST) {
        mStr_dir = QCoreApplication::applicationDirPath()+ "/DVTest/";
    }
    str_dir = mStr_dir;
    //mStr_dir.append(str_time);
    QString currentProName = ui->comboBoxProject->currentText();
    if(currentProName.compare("") == 0) {
        currentProName.append("C03");
        //return;
    }
    QString sn_name;
    if(mSN.compare("") != 0 && mSN.contains("/")) {
         QStringList list = mSN.split("/");
         sn_name.append(list.at(0)).append(list.at(1));
    }else {
         sn_name = "0000000000";
    }
    QString str = currentProName + "/" +str_time + "/" + sn_name;
    mStr_dir.append(str);
      QDir dir(mStr_dir);
      if(!dir.exists())
      {
          bool ok=dir.mkpath(mStr_dir);
          if(ok)
          {
             // QMessageBox::warning(this,tr("创建目录"),tr("创建成功"));
              mDisplayLogData.append("创建目录成功").append("\n");
          }
          else{
           //   QMessageBox::warning(this,tr("创建目录"),tr("创建失败"));
              mDisplayLogData.append("创建目录失败").append("\n");
          }
      } else {
         // qDebug() << "mStr_dir exist";
      }
      mTestResultFileName = str_dir + "/" + currentProName + "/" + str_time + "/" +QString("Test_result_") + str_time+ ".xml";
      mTestResultFileName_csv = str_dir + "/" + currentProName + "/" + currentProName + QString("_Test_result")+ ".csv";
      if(mTestResultFileName == "")
      {
          return;
      }
      QFile file(mTestResultFileName);//可以自己选择路径来保存文件名
      if(file.exists()) {
         // qDebug() << "mTestResultFileName exist";
          ReadWriteXml *xml = new ReadWriteXml();
          xml->readXml(mTestResultFileName);
          bool ok;
          mTotalTestItems = (xml->getTotalTestNum()).toInt(&ok,10);
          ui->lineEdit_total_test->setText(xml->getTotalTestNum());
          mTotalPassItems = (xml->getTotalPassNum()).toInt(&ok,10);
          ui->lineEdit_pass_num->setText(xml->getTotalPassNum());
      }
      if(!file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
      {
        //  QMessageBox::warning(this,tr("错误"),tr("打开文件失败"));
          mDisplayLogData.append("打开文件失败").append("\n");
          //return;
      }
      else
      {
          QTextStream textStream(&file);
        //  QMessageBox::warning(this,tr("提示"),tr("保存文件成功"));
          mDisplayLogData.append("保存文件成功").append("\n");
          file.close();
      }
      if(mTestResultFileName == "")
      {
          return;
      }
      QFile file_csv(mTestResultFileName_csv);//可以自己选择路径来保存文件名
      if(file_csv.exists()) {
         // qDebug() << "mTestResultFileName_csv exist";
      }

      if(!file_csv.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
      {
        //  QMessageBox::warning(this,tr("错误"),tr("csv打开文件失败"));
          mDisplayLogData.append("打开文件失败").append("\n");
          //return;
      }
      else
      {
          QTextStream textStream(&file_csv);
        //  QMessageBox::warning(this,tr("提示"),tr("csv保存文件成功"));
          mDisplayLogData.append("保存文件成功").append("\n");
          file_csv.close();
      }
      mTotalCountFileName =  str_dir + "/" + currentProName + "/" + str_time + "/" + QString("count_num_") + str_time + ".json";
      if(mTotalCountFileName == "")
      {
          return;
      }

      QFile file_1(mTotalCountFileName);//可以自己选择路径来保存文件名
      if(file_1.exists()) {
          //qDebug() << "1111mTotalCountFileName exist";
          readDataFromJson();
      }
      if(!file_1.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
      {
        //  QMessageBox::warning(this,tr("错误"),tr("json打开文件失败"));
          mDisplayLogData.append("打开文件失败").append("\n");
         // return;
      }
      else
      {
          QTextStream textStream(&file_1);
        //  QMessageBox::warning(this,tr("提示"),tr("json保存文件成功"));
          mDisplayLogData.append("保存文件成功").append("\n");
          file_1.close();
      }
}

void MainWindow::on_pushButton_search_clicked() {
    qDebug() << "=========on_pushButton_search_clicked" << mTestResultFileName_csv;
    //if(ui->lineEditCalendar->text() == " ") {
     //   QMessageBox::warning(this,tr("提示"),tr("请先选择日期和项目"));
     //   return;
   // }
    QString str_dir;
    if(mTestType==FUNC_TEST) {
        str_dir = QCoreApplication::applicationDirPath()+ "/FuncTest/";
    } else if(mTestType == EOL_TEST) {
        str_dir = QCoreApplication::applicationDirPath()+ "/EOLTest/";
    } else if(mTestType == DV_TEST) {
        str_dir = QCoreApplication::applicationDirPath()+ "/DVTest/";
    }
    QString currentProName = ui->comboBoxSearch->currentText();
    QString str_time = ui->lineEditCalendar->text();
    QString csv_file_path = str_dir + currentProName + "/" + currentProName+QString("_Test_result") + ".csv";
    qDebug() << "=========on_pushButton_search_clicked1111" << csv_file_path;
    QList<QStringList> list_ret;
    ui->textEdit_search->clear();
    QString search_result;
    QFile file(csv_file_path);
    if(!file.exists()) {
        search_result.append("查询文件不存在");
        ui->textEdit_search->append(search_result);
        ui->textEdit_search->moveCursor(QTextCursor::End);
        return;
    }
    if(!file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Open Excel file failed!";
        return;
    }
    QStringList list;
    list.clear();
    QTextStream in(&file);
    in.setCodec("utf-8");      //这行的目的是支持读取中文信息
    in.seek(0);
    qDebug() << "------in.atEnd():" << in.atEnd();
    //遍历行
    for(int i = 0; !in.atEnd(); i++)
    {
        //qDebug() << "i:" << i;
        QString fileLine = in.readLine();
        list = fileLine.split(",", QString::SkipEmptyParts);    //根据","开分隔开每行的列
        list_ret.append(list);

    }
    file.close();
    if(list_ret.size()==0) return;

    qDebug() << "list_ret.size():" << list_ret.size();
    QList<QStringList> search_list;
    search_list.clear();
    int num = 0;
    QString sn = ui->lineEdit_SN_Search->text();
    qDebug() << "sn:" << sn;
    QString sn_result;
    QString test_time;
    QString test_result;
    QString hw;
    QString sw ;
    QString nottofdistance;
    QString tofdistance;
    QString normal_current_value_9V;
    QString normal_current_value_13V;
    QString normal_current_value_15V;
    QString idle_current_value_9V;
    QString idle_current_value_13V;
    QString idle_current_value_15V;
    QString sleep_current_value_9V;
    QString sleep_current_value_13V;
    QString sleep_current_value_15V;
    QString diamater;
    QString brightness;
    QString distortion;
    for(int i =0;i< list_ret.size(); i++) {
       qDebug() <<"====="<< list_ret.at(i);
        QStringList list_i = list_ret.at(i);
        if(list_i.contains(sn)) {
            search_list.append(list_i);
            num++;
            //break;
        }
    }

    //qDebug() <<"=====search_list:"<< search_list;
    if(search_list.size() == 0) {
        search_result.append("无此测试结果");
        ui->textEdit_search->append(search_result);
        ui->textEdit_search->moveCursor(QTextCursor::End);
        return;
    }
    for (int i= 0; i< search_list.size();i++) {
        sn_result = search_list.at(i).at(0);
        test_time = search_list.at(i).at(1);
        test_result = search_list.at(i).at(2);
        hw = search_list.at(i).at(3);
        qDebug() << "=======hw:" << hw;
        sw = search_list.at(i).at(4);
        qDebug() << "============sw:" << sw;
        nottofdistance = search_list.at(i).at(5);
        tofdistance = search_list.at(i).at(6);
        normal_current_value_9V = search_list.at(i).at(7);
        normal_current_value_13V = search_list.at(i).at(8);
        normal_current_value_15V = search_list.at(i).at(9);
        idle_current_value_9V = search_list.at(i).at(10);
        idle_current_value_13V = search_list.at(i).at(11);
        idle_current_value_15V = search_list.at(i).at(12);
        sleep_current_value_9V = search_list.at(i).at(13);
        sleep_current_value_13V = search_list.at(i).at(14);
        sleep_current_value_15V = search_list.at(i).at(15);
        diamater = search_list.at(i).at(16);
        brightness = search_list.at(i).at(17);
        distortion = search_list.at(i).at(18);
        search_result.clear();
        search_result.append("=======第").append(QString::number(i+1,10)).append("次测试结果======").append("\n");
        search_result.append("\n");
        search_result.append("产品SN:").append(sn).append("      ");
        search_result.append("测试时间:").append(test_time).append("\n");
        search_result.append("\n");
        search_result.append("测试结果:").append(test_result).append("\n");
        search_result.append("\n");
        search_result.append("硬件版本号:").append(hw).append("\n");
        search_result.append("\n");
        search_result.append("软件版本号:").append(sw).append("\n");
        search_result.append("\n");
        search_result.append("未踩踏距离:").append(nottofdistance).append("\n");
        search_result.append("\n");
        search_result.append("踩踏距离:").append(tofdistance).append("\n");
        search_result.append("\n");
        search_result.append("工作9V电流:").append(normal_current_value_9V).append("\n");
        search_result.append("\n");
        search_result.append("工作13V电流:").append(normal_current_value_13V).append("\n");
        search_result.append("\n");
        search_result.append("工作16V电流:").append(normal_current_value_15V).append("\n");
        search_result.append("\n");
        search_result.append("待机9V电流:").append(idle_current_value_9V).append("\n");
        search_result.append("\n");
        search_result.append("待机13V电流:").append(idle_current_value_13V).append("\n");
        search_result.append("\n");
        search_result.append("待机16V电流:").append(idle_current_value_15V).append("\n");
        search_result.append("\n");
        search_result.append("静态9V电流:").append(sleep_current_value_9V).append("\n");
        search_result.append("\n");
        search_result.append("静态13V电流:").append(sleep_current_value_13V).append("\n");
        search_result.append("\n");
        search_result.append("静态16V电流:").append(sleep_current_value_15V).append("\n");
        search_result.append("\n");
        search_result.append("投影直径:").append(diamater).append("\n");
        search_result.append("\n");
        search_result.append("投影照度:").append(brightness).append("\n");
        search_result.append("\n");
        search_result.append("投影畸变:").append(distortion).append("\n");
        search_result.append("\n");
        ui->textEdit_search->append(search_result);
        ui->textEdit_search->moveCursor(QTextCursor::End);
    }
}



QList<QStringList> MainWindow::ReadFromCSV(QString PathName)
{
    QList<QStringList> list_ret;
    //QString fileName = QFileDialog::getOpenFileName(this, tr("Excel file"), qApp->applicationDirPath (),
          //                                             tr("Files (*.csv)"));
    QFile file(PathName);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Open Excel file failed!";
        return list_ret;
    }
    QStringList list;
    list.clear();
    QTextStream in(&file);
    in.setCodec("utf-8");      //这行的目的是支持读取中文信息
    in.seek(0);
    qDebug() << "------in.atEnd():" << in.atEnd();
    //遍历行
    for(int i = 0; !in.atEnd(); i++)
    {
        //qDebug() << "i:" << i;
        QString fileLine = in.readLine();
        list = fileLine.split(",", QString::SkipEmptyParts);    //根据","开分隔开每行的列
        list_ret.append(list);

    }
        //qDebug() << list_ret;
   // qDebug()<< list_ret.size();
    bool isContain = false;
    for(int j =0;j< list_ret.size(); j++) {
        qDebug() <<"====="<< list_ret.at(j);
        if(list_ret.at(j).contains("产品SN")) {
            qDebug("=============containsn");
            isContain = true;
            break;
        }
    }
    if(list_ret.size() == 0 || !isContain) {
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
        str_csv.append("工作16V电流");
        str_csv.append(",");
        str_csv.append("待机9V电流");
        str_csv.append(",");
        str_csv.append("待机13V电流");
        str_csv.append(",");
        str_csv.append("待机16V电流");
        str_csv.append(",");
        str_csv.append("静态9V电流");
        str_csv.append(",");
        str_csv.append("静态13V电流");
        str_csv.append(",");
        str_csv.append("静态16V电流");
        str_csv.append(",");
        str_csv.append("投影直径");
        str_csv.append(",");
        str_csv.append("投影照度");
        str_csv.append(",");
        str_csv.append("投影畸变\n");
        //writeCSVFile(str_csv.toUtf8());
        file.write(str_csv.toUtf8());
        file.flush();
        file.seek(0);
    }

    file.close();
    return list_ret;
}

void MainWindow::writeCSVFile(const char *str_data){
    QFile file(mTestResultFileName_csv);
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

void MainWindow::writeDataToJson(int num){
    // 定义 { } 对象
    QJsonObject countObj;
    // 插入元素，对应键值对
    countObj.insert("testNum", num);
    // 定义根节点	也即是最外层 { }
    QJsonObject rootObject;
    rootObject.insert("count", countObj);
    // 将json对象里的数据转换为字符串
    QJsonDocument doc;
    // 将object设置为本文档的主要对象
    doc.setObject(rootObject);
    // Json字符串保存到json文件里
    QFile file(mTotalCountFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qDebug() << "can't open error!";
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");		// 设置写入编码是UTF8
    // 写入文件
    stream << doc.toJson();
    file.close();
}

void  MainWindow::modifyDataToJson(int num) {
    QFile readFile(mTotalCountFileName);
    if (!readFile.open(QFile::ReadOnly | QFile::Truncate)) {
        qDebug() << "can't open error!";
        return;
    }

    // 读取文件的全部内容
    QTextStream readStream(&readFile);
    readStream.setCodec("UTF-8");		// 设置读取编码是UTF8
    QString str = readStream.readAll();
    readFile.close();

    /* 修改Json */

    // QJsonParseError类用于在JSON解析期间报告错误。
    QJsonParseError jsonError;
    // 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
    // 如果解析成功，返回QJsonDocument对象，否则返回null
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        qDebug() << "Json格式错误！" << jsonError.error;
        return;
    }
    // 获取根 { }
    QJsonObject rootObj = doc.object();
    // 修改 { } 中的值
    QJsonValue countValue = rootObj.value("count");
    if (countValue.type() == QJsonValue::Object) {
        QJsonObject countObject = countValue.toObject();
        countObject["testNum"] = num;
        rootObj["count"] = countObject;
   }
    // 将object设置为本文档的主要对象
    doc.setObject(rootObj);

    // 重写打开文件，覆盖原有文件，达到删除文件全部内容的效果
    QFile writeFile(mTotalCountFileName);
    if (!writeFile.open(QFile::WriteOnly | QFile::Truncate)) {
        qDebug() << "can't open error!";
        return;
    }

    // 将修改后的内容写入文件
    QTextStream wirteStream(&writeFile);
    wirteStream.setCodec("UTF-8");		// 设置读取编码是UTF8
    wirteStream << doc.toJson();		// 写入文件
    writeFile.close();					// 关闭文件
}

void MainWindow::readDataFromJson(){
    QFile file(mTotalCountFileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "can't open error!";
        return;
    }

    // 读取文件的全部内容
    QTextStream stream(&file);
    stream.setCodec("UTF-8");		// 设置读取编码是UTF8
    QString str = stream.readAll();

    file.close();
    /* 解析Json */
    // QJsonParseError类用于在JSON解析期间报告错误。
    QJsonParseError jsonError;
    // 将json解析为UTF-8编码的json文档，并从中创建一个QJsonDocument。
    // 如果解析成功，返回QJsonDocument对象，否则返回null
    QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
    // 判断是否解析失败
    if (jsonError.error != QJsonParseError::NoError && !doc.isNull()) {
        qDebug() << "Json格式错误！" << jsonError.error;
        return;
    }

    // 获取根 { }
    QJsonObject rootObj = doc.object();
    // 解析对象 { }
    QJsonValue countValue = rootObj.value("count");
    // 判断是否是object类型
    if (countValue.type() == QJsonValue::Object) {
        // 转换为QJsonObject类型
        QJsonObject countObj = countValue.toObject();
        QJsonValue testNum = countObj.value("testNum");
        qDebug() << "testNum = " << testNum.toInt();
        mTotalTestNum = testNum.toInt();
    }
}
void MainWindow::on_pushButtonChooseAll_clicked(){
    testCaseItemChoose(true);
    //ui->checkBoxTempSet->setChecked(true);
    //ui->checkBoxWakeupSet->setChecked(true);
   // ui->checkBoxVolSleepSet->setChecked(true);
}
void MainWindow::on_pushButtonClearAll_clicked(){
    testCaseItemChoose(false);
    //ui->checkBoxTempSet->setChecked(false);
    //ui->checkBoxWakeupSet->setChecked(false);
    //ui->checkBoxVolSleepSet->setChecked(false);
}

void MainWindow::on_pushButtonTestResult_clicked(){
   // sendCcdCommand();
    //00 01 00 00
    //01 00 00 0A 02 1B  10.539
    //01 00 47 EB 02 08  18411.520
    //01 00 00 03 01 F8   3.504
    //02 63
   /* QString str_temp = buff.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
    qDebug() <<"receive ccd" << str_show;
    buff.clear();*/
    /*QString str_show = "25 00 01 00 00 02 00 00 0A 02 1B 02 00 47 EB 02 08 02 00 00 03 01 F8 02 63 23";
    QStringList list = str_show.split(" ");
    QString string_flag = list.at(1);
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
        diameter.append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
    }
        break;
    case 02:
    {
        diameter.append("-").append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
    }
        break;
    case 03:
    {
        diameter.append(QString::number(int_diameter_integer)).append(".").append(QString::number(int_diameter_decimal));
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
        brightness.append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
    }
        break;
    case 02:
    {
        brightness.append("-").append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
    }
        break;
    case 03:
    {
        brightness.append(QString::number(int_brightness_integer)).append(".").append(QString::number(int_brightness_decimal));
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
        distortion.append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
    }
        break;
    case 02:
    {
        distortion.append("-").append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
    }
        break;
    case 03:
    {
        distortion.append(QString::number(int_distortion_integer)).append(".").append(QString::number(int_distortion_decimal));
    }
        break;
    }
    qDebug()<<distortion;*/
    //emit send_plc_command(PLC_R3_SET);
    //sendCcdCommand();
   // testStart();
   // emit send_plc_command(PLC_R3_SET);
    //emit writeIMS_sig(IMS_REQUEST_BARCODE,"", 0);
    //generateSN();
    // ui->TofResponseTime->setText(generateRandData());
    /* QByteArray buff = "%01$RD0E0013\r";
     char * a = buff.data();
     qDebug() << a;
     QByteArray byteArray = 0;
     byteArray.append(*(a+6)).append(*(a+7));
     QString string = byteArray;
     qDebug() << string;

     bool ok;
     int dec=string.toInt(&ok,16);
     qDebug() << dec;
     QString result=QString("%1").arg(dec, 8, 2, QChar('0'));
     qDebug() << result;
     QString R107 = result.at(0);
     QString R106 = result.at(1);
     QString R105 = result.at(2);
     QString R104 = result.at(3);
     QString R103 = result.at(4);
     QString R102 = result.at(5);
     QString R101 = result.at(6);
     QString R100 = result.at(7);
     qDebug() << "R107:" << R107 << "R106:" << R106 << "R105:" << R105 << "R104:" << R104 << "R103:" << R103 << "R102:" << R102 << "R101:" << R101 << "R100:" << R100 ;*/
   /* QByteArray buff = "ACK202103180001";
    qDebug() << "on_pushButtonTestResult_clicked" << buff.size();
    QString barcode = "";
    if(buff.size() == 15) {
        if(buff.startsWith("ACK")) {
            qDebug()<<"parseReceiveData barcode:" << buff[3];
            qDebug()<<"parseReceiveData barcode:" << buff[14];
            barcode = buff.mid(3,14);
            qDebug()<<"parseReceiveData barcode:" << barcode;
        }
    }else if(buff.length() < 15){

    }*/

    intToFloat(0);
}

void MainWindow::on_pushButtonImportXml_clicked(){
    ReadWriteXml *xml = new ReadWriteXml();
   // qDebug() << QDir::currentPath();
    QString dirpath = QFileDialog::getOpenFileName(this,tr("Open File"),"./");
    mParamSetFilePath = dirpath;
    xml->readXml(dirpath);
    //xml->readXml(QDir::currentPath() + "default_param.xml");
   // xml->readXml("C:/Users/13521/Desktop/lhx/QT_EOL_TEST");
    //QStringList projectlist = xml->getProjectList();
    //if(mProjectList.size() == 0) {
       // mProjectList.append(projectlist);
       // ui->comboBoxProject->addItems(mProjectList);
   // }
   // ui->comboBoxProject->addItems(xml->getProjectList());
    ui->lineEditHwSet->setText(xml->getHwVersion());
    ui->lineEditSwSet->setText(xml->getSwVersion());
   // ui->lineEditProductNumSet->setText(xml->getProductNum());
    ui->lineEditNotTofDis->setText(xml->getNotTofDistance());
    ui->lineEditTofDis->setText(xml->getTofDistance());
    QString projectName = xml->getProjectName();
    mProjectList.clear();
    ui->comboBoxProject->clear();
    if(mProjectList.size() == 0) {
        mProjectList.append(projectName);
        ui->comboBoxProject->addItems(mProjectList);
    }
    ui->comboBoxProject->setCurrentText(projectName);
    ui->lineEdit9VNorCurMin->setText(xml->get9VNormalMinCur());
    ui->lineEdit15VNorCurMin->setText(xml->get15VNormalMinCur());
    ui->lineEdit13VNorCurMin->setText(xml->get13VNormalMinCur());
    ui->lineEdit9VSleepCurMin->setText(xml->get9VSleepMinCur());
    ui->lineEdit15VSleepCurMin->setText(xml->get15VSleepMinCur());
    ui->lineEdit13VSleepCurMin->setText(xml->get13VSleepMinCur());
    ui->lineEdit9VIdleCurMin->setText(xml->get9VIdleMinCur());
    ui->lineEdit15VIdleCurMin->setText(xml->get15VIdleMinCur());
    ui->lineEdit13VIdleCurMin->setText(xml->get13VIdleMinCur());
    ui->lineEdit9VNorCurMax->setText(xml->get9VNormalMaxCur());
    ui->lineEdit15VNorCurMax->setText(xml->get15VNormalMaxCur());
    ui->lineEdit13VNorCurMax->setText(xml->get13VNormalMaxCur());
    ui->lineEdit9VSleepCurMax->setText(xml->get9VSleepMaxCur());
    ui->lineEdit15VSleepCurMax->setText(xml->get15VSleepMaxCur());
    ui->lineEdit13VSleepCurMax->setText(xml->get13VSleepMaxCur());
    ui->lineEdit9VIdleCurMax->setText(xml->get9VIdleMaxCur());
    ui->lineEdit15VIdleCurMax->setText(xml->get15VIdleMaxCur());
    ui->lineEdit13VIdleCurMax->setText(xml->get13VIdleMaxCur());
    ui->lineEditVolFloat->setText(xml->getFloatVol());
    delete xml;
    xml = NULL;
    if(mUser_Name.compare("administrator") != 0) {
        setAllSettingStatus(false);
    }else {
        ui->pushButtonSaveSetting->setEnabled(true);
    }
    if(mTotalTestNum != 0) {
        mTotalTestNum = 0;
    }
}

void MainWindow::setAllSettingStatus(bool status) {
    if(!status) {
        //ui->checkBoxHwSet->setEnabled(false);
        //ui->checkBoxSwSet->setEnabled(false);
        ui->checkBoxTofSet->setEnabled(false);
        ui->checkBoxLogoSet->setEnabled(false);
       // ui->checkBoxTempSet->setEnabled(false);
        ui->checkBoxVolLowSet->setEnabled(false);
        //ui->checkBoxWakeupSet->setEnabled(false);
        ui->checkBoxVolIdleSet->setEnabled(false);
        ui->checkBoxVolClearSet->setEnabled(false);
        ui->checkBoxVolSleepSet->setEnabled(false);
        ui->checkBoxVolNormalSet->setEnabled(false);
       // ui->checkBoxProductNumSet->setEnabled(false);
        ui->checkBoxVolHighSet->setEnabled(false);
        ui->lineEditHwSet->setEnabled(false);
        ui->lineEditSwSet->setEnabled(false);
       // ui->lineEditProductNumSet->setEnabled(false);
        ui->lineEditNotTofDis->setEnabled(false);
        ui->lineEditTofDis->setEnabled(false);
        ui->lineEdit9VNorCurMin->setEnabled(false);
        ui->lineEdit15VNorCurMin->setEnabled(false);
        ui->lineEdit13VNorCurMin->setEnabled(false);
        ui->lineEdit9VSleepCurMin->setEnabled(false);
        ui->lineEdit15VSleepCurMin->setEnabled(false);
        ui->lineEdit13VSleepCurMin->setEnabled(false);
        ui->lineEdit9VIdleCurMin->setEnabled(false);
        ui->lineEdit15VIdleCurMin->setEnabled(false);
        ui->lineEdit13VIdleCurMin->setEnabled(false);
        ui->lineEdit9VNorCurMax->setEnabled(false);
        ui->lineEdit15VNorCurMax->setEnabled(false);
        ui->lineEdit13VNorCurMax->setEnabled(false);
        ui->lineEdit9VSleepCurMax->setEnabled(false);
        ui->lineEdit15VSleepCurMax->setEnabled(false);
        ui->lineEdit13VSleepCurMax->setEnabled(false);
        ui->lineEdit9VIdleCurMax->setEnabled(false);
        ui->lineEdit15VIdleCurMax->setEnabled(false);
        ui->lineEdit13VIdleCurMax->setEnabled(false);
        ui->lineEditVolFloat->setEnabled(false);
    }else {
        //ui->checkBoxHwSet->setEnabled(true);
        //ui->checkBoxSwSet->setEnabled(true);
        ui->checkBoxTofSet->setEnabled(true);
        ui->checkBoxLogoSet->setEnabled(true);
        //ui->checkBoxTempSet->setEnabled(true);
        ui->checkBoxVolLowSet->setEnabled(true);
        //ui->checkBoxWakeupSet->setEnabled(true);
        ui->checkBoxVolIdleSet->setEnabled(true);
        ui->checkBoxVolClearSet->setEnabled(true);
        ui->checkBoxVolSleepSet->setEnabled(true);
        ui->checkBoxVolNormalSet->setEnabled(true);
        //ui->checkBoxProductNumSet->setEnabled(true);
        ui->checkBoxVolHighSet->setEnabled(true);
        ui->lineEditHwSet->setEnabled(true);
        ui->lineEditSwSet->setEnabled(true);
      //  ui->lineEditProductNumSet->setEnabled(true);
        ui->lineEditNotTofDis->setEnabled(true);
        ui->lineEditTofDis->setEnabled(true);
        ui->lineEdit9VNorCurMin->setEnabled(true);
        ui->lineEdit15VNorCurMin->setEnabled(true);
        ui->lineEdit13VNorCurMin->setEnabled(true);
        ui->lineEdit9VSleepCurMin->setEnabled(true);
        ui->lineEdit15VSleepCurMin->setEnabled(true);
        ui->lineEdit13VSleepCurMin->setEnabled(true);
        ui->lineEdit9VIdleCurMin->setEnabled(true);
        ui->lineEdit15VIdleCurMin->setEnabled(true);
        ui->lineEdit13VIdleCurMin->setEnabled(true);
        ui->lineEdit9VNorCurMax->setEnabled(true);
        ui->lineEdit15VNorCurMax->setEnabled(true);
        ui->lineEdit13VNorCurMax->setEnabled(true);
        ui->lineEdit9VSleepCurMax->setEnabled(true);
        ui->lineEdit15VSleepCurMax->setEnabled(true);
        ui->lineEdit13VSleepCurMax->setEnabled(true);
        ui->lineEdit9VIdleCurMax->setEnabled(true);
        ui->lineEdit15VIdleCurMax->setEnabled(true);
        ui->lineEdit13VIdleCurMax->setEnabled(true);
        ui->lineEditVolFloat->setEnabled(true);
    }

}
//multi thread transfer data
void MainWindow::showSerialPortData(int portId,QByteArray buff) {
    //qDebug() << "===showSerialPortData:" <<QThread::currentThreadId();
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    mRecBuffer.append(buff);
    QString str_temp = buff.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }

   // qDebug() <<str_show;

    switch(portId)
    {
    case COM0:
        ui->PlainTextEdit_show->appendPlainText("控制串口:"+ str_show);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(str).append("控制串口：").append(str_show).append("\n");
        //parseSerialData(str_show);
        if(buff.compare("%01#WCSR0002121\r") == 0) {
            qDebug() << "===showSerialPortData send_plc_command(PLC_R2_SET)";
            //emit send_plc_command(PLC_R2_SET);
        }
        break;
    case COM1:
        ui->PlainTextEdit_show->appendPlainText("CCD串口:"+str_show);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(str).append("CCD串口: ").append(str_show).append("\n");
        break;
    case COM2:
       /* if(buff.compare("%01#RDD001000010055\r") == 0){
            mDisplayLogData.append(str).append("PLC串口:"+buff).append("\n");
        }else {*/
            ui->PlainTextEdit_show->appendPlainText("PLC串口:"+buff);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
            mDisplayLogData.append(str).append("PLC串口:"+buff).append("\n");
       // }

        break;
    case COM3:
        ui->PlainTextEdit_show->appendPlainText("IMS串口:"+buff);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(str).append("IMS串口:"+buff).append("\n");
        break;
    default:
        break;
    }
    writeTXT(mDisplayLogData.toUtf8());
    mDisplayLogData.clear();
    buff.clear();
}

//create serialcmdpackage //todo serialport
void MainWindow::serial_cmd_pack(uint8_t cmd,uint8_t *data)
{
    //qDebug()<<"=======serial_cmd_pack: ";
    QByteArray sendData = 0;
   // sendData.resize(8);
    sendData[0] = enFrame_head::AR_KEY_SEND_HEAD;
    sendData[1] = cmd;
    sendData[2] = *data;
    sendData[3] = *(data+1);
    sendData[4] = *(data+2);
    sendData[5] = *(data+3);
    sendData[6] = *(data+4);
    // sendData[3] = 0x00;
    // sendData[4] = 0x00;
    // sendData[5] = 0x00;
    // sendData[6] = 0x00;
    qDebug() << "sendData[1]:" << sendData[1] << cmd;
    qDebug() << "sendData[2]:" << sendData[2] << *data;
    qDebug() << "sendData[3]:" << sendData[3];
    qDebug() << "sendData[4]:" << sendData[4];
    qDebug() << "sendData[5]:" << sendData[5];
    qDebug() << "sendData[6]:" << sendData[6];
    int sum = sendData[0]+sendData[1]+sendData[2]+sendData[3]+sendData[4]+sendData[5]+sendData[6]+sendData[7];
    qDebug() << "sum:" << sum;
    sendData[7] = (sendData[0]+sendData[1]+sendData[2]+sendData[3]+sendData[4]+sendData[5]+sendData[6]+sendData[7])&0xff;

    //qDebug() << "sendData:" << sendData;
    //mRecBuffer.append(sendData);
    QString str_temp = sendData.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
    qDebug() << "str_show:" << str_show;
    mDisplayLogData.append("控制串口：").append(str_show);
    writeTXT(mDisplayLogData.toUtf8());
    ui->PlainTextEdit_show->appendPlainText(str_show);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
   // uint16_t b = BYTE_TO_UINT8(0x12,0x31);
   // uint16_t vol_val =(uint16_t) 0x12<<8|0x31;
   // qDebug() << b << vol_val;
   // parseSerialData(str_show);
    if(Port_ctrl != NULL) {
        qDebug()<<"=======serial_cmd_pack: " <<sendData ;
       // Port_ctrl->getSerialPort()->write(sendData,8);
       emit writePort_sig(COM0,sendData);
       qDebug()<<"=======serial_cmd_pack end " ;
    }
}

void MainWindow::sendCcdCommand(){
    QByteArray sendData=0;
    sendData[0] = 0x00;
    sendData[1] = 0x01;
    sendData[2] = 0x00;
    sendData[3] = 0x00;
     //qDebug() << "ccd command:"<<sendData;
    QString str_temp = sendData.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
   // qDebug() << "ccd command 11:"<<str_show;
   // sendData.clear();
    QStringList list = str_show.split(" ");
    QString string_flag = list.at(1);
    bool ok;
    int flag= string_flag.toInt(&ok, 16);
    if(Port_ccd != NULL) {
        mTestTime = QTime::currentTime();
        QString str;
        str.prepend("]  ");
        str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
        str.prepend("[");
        str.append(str_show);
        ui->PlainTextEdit_show->appendPlainText(str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        emit writePort_sig(COM1,sendData);
    }
}

void MainWindow::parseCcdData(QString buffer) {
    qDebug()<<"parseCcdData: ";

}

void MainWindow::parsePclData(QString buffer) {
    qDebug()<<"parsePclData: ";
}
//功能测试开始 //todo 添加项目选择下拉列表
void MainWindow::testStart() {
    //qDebug()<<"===========testStart: ";
    mTestCmd = enTest_cmd::AR_KEY_TEST_START;
    uint8_t cmd_data[5] = {0};
    int currentProId = ui->comboBoxProject->currentIndex();
    qDebug()<<"testStart: " <<currentProId ;
    switch(currentProId) {
    case enTestProject::G59:
        cmd_data[0] = 0x00;
        break;
    case enTestProject::C206:
        cmd_data[0] = 0x01;
        break;
    case enTestProject::C03:
        cmd_data[0] = 0x02;
        break;
    case enTestProject::N61:
        cmd_data[0] = 0x03;
        break;
    case enTestProject::N60:
        cmd_data[0] = 0x04;
        break;
    case enTestProject::C46:
        cmd_data[0] = 0x05;
        break;
    default:
        break;
    }
    serial_cmd_pack(mTestCmd,cmd_data);
}

void MainWindow::doEOLTest(){
}
void MainWindow::doBoardTest(){

}
void MainWindow::doFunctionTest(){

}

bool MainWindow::checkTestStarted(int started){
    qDebug() << "checkTestStarted:";
    if(mTimeStartTest != NULL) {
        if(mTimeStartTest->isActive()) {
            mTimeStartTest->stop();
        }
    }
    if(started){
        ui->PlainTextEdit_show->appendPlainText("功能测试开始");
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        //lhxtemp
        if(Port_pcl != NULL) {
           emit send_plc_command(PLC_R3_SET);
        }
       //emit runTestCase(AR_KEY_TEST_VOL_SLEEP,V9_SLEEP);

    } else {
        ui->PlainTextEdit_show->appendPlainText("功能测试开始失败");
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mTestCmd = AR_KEY_NO_CMD;
        QMessageBox::warning(this,tr("开始功能测试失败，请重连！"),tr("请确认：1.DTU电源线连接是否正常？\r\n               2.CAN/LIN总线通讯是否正常？"));
        m_showTimer->stop();
        mTestTime = QTime::currentTime();
        QString str;
        str.prepend("]  ");
        str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
        str.prepend("[");
        ui->PlainTextEdit_show->appendPlainText(str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        if(ui->pushButtonStartTest->text() == "测试中") {
            emit startTest(false,0,mTestType,ui->comboBoxProject->currentText());
            ui->pushButtonStartTest->setEnabled(true);
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
        }
        mDisplayLogData.append("测试开始失败").append("\n");
        writeTXT(mDisplayLogData.toUtf8());
        mDisplayLogData.clear();
        if(ui->tabWidget->currentIndex() == 0) {
            if(Port_pcl != NULL) {
                //lihongxia
                mTimeStartTest->start(mTimeStartTimeout);
            }
        }
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
        mTestRunning = false;
        ui->pushButtonSaveSetting->setEnabled(true);

    }
    return true;
}

QString MainWindow::intToFloat(int value) {
   // QString strHex = ui.six_float1->text();
   // int c = strHex.toInt(nullptr, 16);
   // bool ok;
  //  QString data0_string = "3D";
  //  QString data1_string = "23";
  //  QString data2_string = "D7";
 //   QString data3_string = "0A";
 //   uint8_t int_data_0= data0_string.toInt(&ok, 16);
 //   uint8_t int_data_1= data1_string.toInt(&ok, 16);
  //  uint8_t int_data_2= data2_string.toInt(&ok, 16);
  //  uint8_t int_data_3= data3_string.toInt(&ok, 16);
  //  uint32_t current_value = BYTE_TO_UINT32(int_data_0,int_data_1,int_data_2,int_data_3);
    float value_float = *(float*)&value;
    //float value_float = *(float*)&current_value;
    QString radiation = QString::number(value_float,'f',3);
    qDebug() << "radiation:" << radiation;
    return radiation;
    //QString radiation = QString("%1").arg(d);
   // ui.six_float2->setText(radiation);

}

/* 标准：9V时工作电流小于1.3A大于1.1A, 即为PASS。
* 13V时工作电流小于0.85A大于0.6A, 即为PASS。
*15V时工作电流小于0.75A大于0.5A, 即为PASS。
*/
bool MainWindow::checkVolNormal(int volTestType,int current_value,int voltage_value){
    qDebug() << "checkVolNormal:"<< current_value <<"voltage_value:"<<voltage_value;
    if(!mTestRunning) return false;


    //返回电压上下浮动
    int float_value =( ui->lineEditVolFloat->text()).toInt();
    int v9_min_current_value = (ui->lineEdit9VNorCurMin->text()).toInt();
   // float v9_min_current_value = (ui->lineEdit9VNorCurMin->text()).toFloat();
    qDebug() << "v9_min_current_value:" <<v9_min_current_value;
    int v9_max_current_value = (ui->lineEdit9VNorCurMax->text()).toInt();
    int v13_max_current_value = (ui->lineEdit13VNorCurMax->text()).toInt();
    int v13_min_current_value = (ui->lineEdit13VNorCurMin->text()).toInt();
    int v15_max_current_value = (ui->lineEdit15VNorCurMax->text()).toInt();
    int v15_min_current_value = (ui->lineEdit15VNorCurMin->text()).toInt();

   //  QString current_value_string = intToFloat(current_value);
   //  float current_value_float = current_value_string.toFloat();
     qDebug() << "current_value_float:" <<current_value;
     int test_Voltage = V9;
    // 8, 10可以提前设置阀值
    /*if( voltage_value>(TEST_VOL_9-FLOAT_VOL) && voltage_value <(TEST_VOL_9+FLOAT_VOL)) {
        test_Voltage = V9;
        //todo Pass
    } else if(voltage_value>(TEST_VOL_13-FLOAT_VOL) && voltage_value <(TEST_VOL_13+FLOAT_VOL)){
        test_Voltage = V13;
    } else if(voltage_value>(TEST_VOL_15-FLOAT_VOL) && voltage_value <(TEST_VOL_15+FLOAT_VOL)){
        test_Voltage = V15;
    }*/
    if( volTestType == V9_NORMAL) {
        test_Voltage = V9;
        //todo Pass
    } else if(volTestType == V13_NORMAL){
        test_Voltage = V13;
    } else if(volTestType == V15_NORMAL){
        test_Voltage = V15;
    }
    switch(test_Voltage) {
    //unit is mA
    case V9:
        if(current_value > v9_min_current_value && current_value < v9_max_current_value) {
            qDebug() << "checkVolNormal V9 pass";
            mVol9NormalChecked = CHECKED_PASS;
            mPassNum = mPassNum +1;
            ui->pushButton9VolNormal->setText("合格");
            ui->pushButton9VolNormal->setStyleSheet("background-color: rgb(0, 217, 0)");
            //pass
        } else {
            qDebug() << "checkVolNormal V9 fail";
            mVol9NormalChecked = CHECKED_FAILED;
            mFailedNum = mFailedNum +1;
            ui->pushButton9VolNormal->setText("不合格");
            ui->pushButton9VolNormal->setStyleSheet("background-color: rgb(255, 0, 0)");
            //fail
        }
        ui->lineEdit9VANormalRet->setText(QString::number(current_value));
        mNormal_current_value_9V = current_value;
        break;
    case V13:
        if(current_value > v13_min_current_value && current_value < v13_max_current_value) {
            //pass
            mPassNum = mPassNum +1;
            mVol13NormalChecked = CHECKED_PASS;
            ui->pushButton13VolNormal->setText("合格");
            ui->pushButton13VolNormal->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            mFailedNum = mFailedNum +1;
             mVol13NormalChecked = CHECKED_FAILED;
            //fail
             ui->pushButton13VolNormal->setText("不合格");
             ui->pushButton13VolNormal->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit13VANormalRet->setText(QString::number(current_value));
        mNormal_current_value_13V = current_value;
        break;
    case V15:
        if(current_value > v15_min_current_value && current_value < v15_max_current_value) {
            //pass
            mPassNum = mPassNum +1;
            mVol15NormalChecked = CHECKED_PASS;
            ui->pushButton15VolNormal->setText("合格");
            ui->pushButton15VolNormal->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol15NormalChecked = CHECKED_FAILED;
           //fail
            ui->pushButton15VolNormal->setText("不合格");
            ui->pushButton15VolNormal->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit15VANormalRet->setText(QString::number(current_value));
        mNormal_current_value_15V = current_value;
        break;
    }
    if(mVol13NormalChecked == NOT_CHECKED || mVol9NormalChecked == NOT_CHECKED || mVol15NormalChecked == NOT_CHECKED) {
        return false;
    }
    if(mVol13NormalChecked == CHECKED_PASS && mVol9NormalChecked == CHECKED_PASS && mVol15NormalChecked == CHECKED_PASS) {
        mVolNormalChecked = CHECKED_PASS;
        ui->pushButtonVolNormal->setText("合格");
        ui->pushButtonVolNormal->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        mVolNormalChecked = CHECKED_FAILED;
        ui->pushButtonVolNormal->setText("不合格");
        ui->pushButtonVolNormal->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}

/*
标准：13V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。
标准：15V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。
标准：9V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。
*/
bool MainWindow::checkVolSleep(int volTestType,int current_value,int voltage_value){
    qDebug() << "checkVolSleep:"<< current_value <<"voltage_value:"<<voltage_value;
    if(!mTestRunning) return false;
    int test_Voltage = V9;
    //int float_value =( ui->lineEditVolFloat->text()).toInt();
   // int v9_min_current_value = (ui->lineEdit9VSleepCurMin->text()).toInt();
   // int v9_max_current_value = (ui->lineEdit9VSleepCurMax->text()).toInt();
    //int v13_max_current_value = (ui->lineEdit13VSleepCurMax->text()).toInt();
   // int v13_min_current_value = (ui->lineEdit13VSleepCurMin->text()).toInt();
  //  int v15_max_current_value = (ui->lineEdit15VSleepCurMax->text()).toInt();
   // int v15_min_current_value = (ui->lineEdit15VSleepCurMin->text()).toInt();
    float v9_min_current_value = (ui->lineEdit9VSleepCurMin->text()).toFloat();
    float v9_max_current_value = (ui->lineEdit9VSleepCurMax->text()).toFloat();
    float v13_max_current_value = (ui->lineEdit13VSleepCurMax->text()).toFloat();
    float v13_min_current_value = (ui->lineEdit13VSleepCurMin->text()).toFloat();
    float v15_max_current_value = (ui->lineEdit15VSleepCurMax->text()).toFloat();
    float v15_min_current_value = (ui->lineEdit15VSleepCurMin->text()).toFloat();
    QString current_value_string = intToFloat(current_value);
    float current_value_float = current_value_string.toFloat();
    // 8, 10可以提前设置阀值
    /*if( voltage_value>(TEST_VOL_9-FLOAT_VOL) && voltage_value <(TEST_VOL_9+FLOAT_VOL)) {
        test_Voltage = V9;
        //todo Pass
    } else if(voltage_value>(TEST_VOL_13-FLOAT_VOL) && voltage_value <(TEST_VOL_13+FLOAT_VOL)){
        test_Voltage = V13;
    } else if(voltage_value>(TEST_VOL_15-FLOAT_VOL) && voltage_value <(TEST_VOL_15+FLOAT_VOL)){
        test_Voltage = V15;
    }*/
    if( volTestType == V9_SLEEP) {
        test_Voltage = V9;
        //todo Pass
    } else if(volTestType == V13_SLEEP){
        test_Voltage = V13;
    } else if(volTestType == V15_SLEEP){
        test_Voltage = V15;
    }
    switch(test_Voltage) {
    //todo need confirm the value
    //unit is mA
    case V9:
        if(current_value_float > v9_min_current_value && current_value_float < v9_max_current_value) {
            qDebug() << "checkVolNormal V9 pass";
            mPassNum = mPassNum +1;
            mVol9SleepChecked = CHECKED_PASS;
            ui->pushButton9VolSleep->setText("合格");
            ui->pushButton9VolSleep->setStyleSheet("background-color: rgb(0, 217, 0)");
            //pass
        } else {
            qDebug() << "checkVolNormal V9 fail";
            //fail
            mFailedNum = mFailedNum +1;
            mVol9SleepChecked = CHECKED_FAILED;
            ui->pushButton9VolSleep->setText("不合格");
            ui->pushButton9VolSleep->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit9VASleepRet->setText(current_value_string);
        mSleep_current_value_9V = current_value_float;
        break;
    case V13:
        if(current_value_float > v13_min_current_value && current_value_float < v13_max_current_value) {
            //pass
            mPassNum = mPassNum +1;
            mVol13SleepChecked = CHECKED_PASS;
            ui->pushButton13VolSleep->setText("合格");
            ui->pushButton13VolSleep->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol13SleepChecked = CHECKED_FAILED;
            ui->pushButton13VolSleep->setText("不合格");
            ui->pushButton13VolSleep->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit13VASleepRet->setText(current_value_string);
        mSleep_current_value_13V = current_value_float;
        break;
    case V15:
        if(current_value_float > v15_min_current_value && current_value_float < v15_max_current_value) {
            //pass
            mPassNum = mPassNum +1;
            mVol15SleepChecked = CHECKED_PASS;
            ui->pushButton15VolSleep->setText("合格");
            ui->pushButton15VolSleep->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol15SleepChecked = CHECKED_FAILED;
            ui->pushButton15VolSleep->setText("不合格");
            ui->pushButton15VolSleep->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit15VASleepRet->setText(current_value_string);
        mSleep_current_value_15V = current_value_float;
        break;

    }
    qDebug() << mVol13SleepChecked << mVol9SleepChecked << mVol15SleepChecked;
    if(mVol13SleepChecked == NOT_CHECKED || mVol9SleepChecked == NOT_CHECKED || mVol15SleepChecked == NOT_CHECKED) {
        qDebug() << "1111"<< mVol13SleepChecked << mVol9SleepChecked << mVol15SleepChecked;
        return false;
    }
    qDebug() << "2222"<< mVol13SleepChecked << mVol9SleepChecked << mVol15SleepChecked;
    if(mVol13SleepChecked == CHECKED_PASS && mVol9SleepChecked == CHECKED_PASS && mVol15SleepChecked == CHECKED_PASS) {
        mVolSleepChecked = CHECKED_PASS;
        ui->pushButtonVolSleep->setText("合格");
        ui->pushButtonVolSleep->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        mVolNormalChecked = CHECKED_FAILED;
        ui->pushButtonVolSleep->setText("不合格");
        ui->pushButtonVolSleep->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}

/*
标准：13V时工作电流小于 60mA 大于20mA, 即为PASS。
标准：15V时工作电流小于 60mA 大于20mA, 即为PASS。
标准：9V时工作电流小于 70mA 大于20mA, 即为PASS。
*/
bool MainWindow::checkVolIdle(int volTestType,int current_value,int voltage_value){
    qDebug() << "checkVolIdle:"<< current_value <<"voltage_value:"<<voltage_value;
    if(!mTestRunning) return false;
    int test_Voltage = V9;
    int float_value =( ui->lineEditVolFloat->text()).toInt();
    float v9_min_current_value = (ui->lineEdit9VIdleCurMin->text()).toFloat();
    float v9_max_current_value = (ui->lineEdit9VIdleCurMax->text()).toFloat();
    float v13_max_current_value = (ui->lineEdit13VIdleCurMax->text()).toFloat();
    float v13_min_current_value = (ui->lineEdit13VIdleCurMin->text()).toFloat();
    float v15_max_current_value = (ui->lineEdit15VIdleCurMax->text()).toFloat();
    float v15_min_current_value = (ui->lineEdit15VIdleCurMin->text()).toFloat();
    QString current_value_string = intToFloat(current_value);
    float current_value_float = current_value_string.toFloat();
    // 8, 10可以提前设置阀值
    /*if( voltage_value>(TEST_VOL_9-FLOAT_VOL) && voltage_value <(TEST_VOL_9+FLOAT_VOL)) {
        test_Voltage = V9;
        //todo Pass
    } else if(voltage_value>(TEST_VOL_13-FLOAT_VOL) && voltage_value <(TEST_VOL_13+FLOAT_VOL)){
        test_Voltage = V13;
    } else if(voltage_value>(TEST_VOL_15-FLOAT_VOL) && voltage_value <(TEST_VOL_15+FLOAT_VOL)){
        test_Voltage = V15;
    }*/
    if( volTestType == V9_IDLE) {
        test_Voltage = V9;
        //todo Pass
    } else if(volTestType == V13_IDLE){
        test_Voltage = V13;
    } else if(volTestType == V15_IDLE){
        test_Voltage = V15;
    }
    switch(test_Voltage) {
    //todo need confirm the value
    //unit is mA
    case V9:
        if(current_value_float > v9_min_current_value && current_value_float < v9_max_current_value) {
            qDebug() << "checkVolNormal V9 pass";
            mPassNum = mPassNum +1;
            mVol9IdleChecked = CHECKED_PASS;
            ui->pushButton9VolIdle->setText("合格");
            ui->pushButton9VolIdle->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol9IdleChecked = CHECKED_FAILED;
            ui->pushButton9VolIdle->setText("不合格");
            ui->pushButton9VolIdle->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit9VAIdleRet->setText(current_value_string);
        mIdle_current_value_9V = current_value_float;
        break;
    case V13:
        if(current_value_float > v13_min_current_value && current_value_float < v13_max_current_value) {
            //pass
            mVol13IdleChecked = CHECKED_PASS;
            mPassNum = mPassNum +1;
            ui->pushButton13VolIdle->setText("合格");
            ui->pushButton13VolIdle->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol13IdleChecked = CHECKED_FAILED;
            ui->pushButton13VolIdle->setText("不合格");
            ui->pushButton13VolIdle->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit13VAIdleRet->setText(current_value_string);
        mIdle_current_value_13V = current_value_float;
        break;
    case V15:
        if(current_value_float > v15_min_current_value && current_value_float < v15_max_current_value) {
            mVol15IdleChecked = CHECKED_PASS;
            mPassNum = mPassNum +1;
            ui->pushButton15VolIdle->setText("合格");
            ui->pushButton15VolIdle->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mFailedNum = mFailedNum +1;
            mVol15IdleChecked = CHECKED_FAILED;
            ui->pushButton15VolIdle->setText("不合格");
            ui->pushButton15VolIdle->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
        ui->lineEdit15VAIdleRet->setText(current_value_string);
        mIdle_current_value_15V = current_value_float;
        break;

    }
    if(mVol13IdleChecked == NOT_CHECKED || mVol9IdleChecked == NOT_CHECKED || mVol15IdleChecked == NOT_CHECKED) {
        return false;
    }
    if(mVol13IdleChecked == CHECKED_PASS && mVol9IdleChecked == CHECKED_PASS && mVol15IdleChecked == CHECKED_PASS) {
        //mPassNum = mPassNum +1;
        mVolSleepChecked = CHECKED_PASS;
        ui->pushButtonVolIdle->setText("合格");
        ui->pushButtonVolIdle->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
       // mFailedNum = mFailedNum +1;
        mVolNormalChecked = CHECKED_FAILED;
        ui->pushButtonVolIdle->setText("不合格");
        ui->pushButtonVolIdle->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}

/*
标准：所得电压为2.57~2.88V为Pass。
*/
bool MainWindow::checkTempVoltage(int voltage_value){
    if(!mTestRunning) return false;
    uint16_t min_vol = 2570;
    uint16_t max_vol = 2880;
    if(voltage_value >min_vol && voltage_value <max_vol) {
        //pass
        mPassNum = mPassNum +1;
        mTempVolChecked = CHECKED_PASS;
       // ui->pushButtonTemp->setText("合格");
       // ui->pushButtonTemp->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        mFailedNum = mFailedNum +1;
        mTempVolChecked = CHECKED_FAILED;
      //  ui->pushButtonTemp->setText("不合格");
      //  ui->pushButtonTemp->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}
/*
标准：与预设值100%一致为Pass。
*/
bool MainWindow::checkHwInfo(QString hw){
    if(!mTestRunning) return false;
    QString compareHw = ui->lineEditHwSet->text();
    qDebug()<< "checkHwInfo" << hw << "comparehw:" <<compareHw;
    mHw.clear();
    mHw.append(hw);
    int ret = QString::compare(hw, compareHw);
    if(ret == 0) {
        //pass
        mHwChecked = CHECKED_PASS;
        mPassNum = mPassNum +1;
        ui->pushButtonHw->setText("合格");
        ui->pushButtonHw->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        mHwChecked = CHECKED_FAILED;
        mFailedNum = mFailedNum +1;
        ui->pushButtonHw->setText("不合格");
        ui->pushButtonHw->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    ui->lineEditHwRet->setText(hw);
    /*uint8_t cmd_data[5] = {0};
    mTestCmd = enTest_cmd::AR_KEY_TEST_HARDWARE_VER;
    //0X00-13V
    cmd_data[0] = mTestCmd;
    QByteArray sendData=0;
    sendData[0] = 102;
    sendData[1] = mTestCmd;
    sendData[2] = 0x34;
    sendData[3] = 0x56;
    sendData[4] = 0x00;
    sendData[5] = 0x00;
    sendData[6] = 0x00;
    sendData[7] = (sendData[0]+sendData[1]+sendData[2]+sendData[3]+sendData[4]+sendData[5]+sendData[6]+sendData[7])&0xff;
    qDebug() << "sendData:" << sendData;
    QString str_temp = sendData.toHex().toUpper();
    QString str_show;
    for(int i = 0; i<str_temp.length (); i+=2)
    {
        str_show += str_temp.mid (i,2);
        str_show += " ";
    }
    qDebug() << "str_show:" << str_show;
  //  uint16_t b = BYTE_TO_UINT8(0x12,0x31);
    //uint16_t vol_val =(uint16_t) 0x12<<8|0x31;
    //qDebug() << b << vol_val;
    QStringList list = str_show.split(" ");
    QString str_data_0 = list.at(2);
    QString str_data_1 = list.at(3);
    bool ok;
    uint8_t int_data_0 = str_data_0.toInt(&ok, 16);*
    int low_four, high_four;
    high_four = (int_data_0 & 0xf0) >> 4;
    low_four = int_data_0 & 0x0f;
    qDebug() << "high_four:" << high_four << "low_four:" <<low_four;
    QString v = "V";
    QString hw_string = v.append(QString::number(high_four)).append(".").append(QString::number(low_four)).append(".").append(str_data_1);
    qDebug() << "hw:" << hw_string;*/
    return true;
}
/*
标准：与预设值100%一致为Pass。
*/
bool MainWindow::checkSwInfo(QString sw){
    qDebug() <<"sw:"<< sw;
    if(!mTestRunning) return false;
    mSw.clear();
    mSw.append(sw);
    ui->lineEditSwRet->setText(sw);
    QString compareSw = ui->lineEditSwSet->text();
    int ret = QString::compare(sw, compareSw);
    if(ret == 0) {
        //pass
        mSwChecked = CHECKED_PASS;
        mPassNum = mPassNum +1;
        ui->pushButtonSw->setText("合格");
        ui->pushButtonSw->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        mSwChecked = CHECKED_FAILED;
        mFailedNum = mFailedNum +1;
        ui->pushButtonSw->setText("不合格");
        ui->pushButtonSw->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}
/*标准：与预设值100%一致为Pass。
 */
bool MainWindow::checkProductNum(QString prodNum){
  //  qDebug() <<"prodNum:"<< prodNum;
 //   if(!mTestRunning) return false;
 //   mProduct_Num.append(prodNum);
 //   ui->lineEditPNRet->setText(prodNum);
    /*static int readTime = 0;
    readTime ++;
    if(readTime == 1) {
        product_num_value_1 = prodNum;
		qDebug() << "product_num_value_1:" << product_num_value_1;
    }
    QString num = 0;
    if(readTime ==2) {
        product_num_value_2 = prodNum;
		qDebug() << "product_num_value_2:" << product_num_value_2;
        readTime = 0;
        num = product_num_value_1.append(product_num_value_2);
        qDebug() << "num:" << num;
    }*/
  //  QString compareProductNum = ui->lineEditProductNumSet->text();
  //  int ret = QString::compare(prodNum, compareProductNum);
  //  if(ret == 0) {
        //pass
   //     mProductNumFirstChecked = CHECKED_PASS;
   //     mPassNum = mPassNum +1;
  //      ui->pushButtonProductNum->setText("合格");
  //      ui->pushButtonProductNum->setStyleSheet("background-color: rgb(0, 217, 0)");
  //  } else {
   //     mProductNumFirstChecked = CHECKED_FAILED;
   //     mFailedNum = mFailedNum +1;
  //      ui->pushButtonProductNum->setText("不合格");
  //      ui->pushButtonProductNum->setStyleSheet("background-color: rgb(255, 0, 0)");
  //  }
    return true;
}

/*
目的：测试光机LOGO投影。
内容：测试是否投影在标准靶位内。
方法：上位机通过测试板发送命令（附件命令5）与总成建立通讯，发送并回读信息。用摄像头拍照判断是否投影在标准靶位范围内，投影标准靶位范围150*150mm。
标准：投影在标准靶位范围则为PASS。
*/
bool MainWindow::checkLogoOpened(int logo_openned){
    qDebug() << "=====checkLogoOpened:" << logo_openned;
    if(!mTestRunning) return false;
    if(logo_openned == 1) {
        //pass todo send ccd command
        sendCcdCommand();
    } else {
        //fail
        mFailedNum = mFailedNum +1;
        mLogoChecked = CHECKED_FAILED;
        ui->pushButtonLogo->setText("不合格");
        ui->pushButtonLogo->setStyleSheet("background-color: rgb(255, 0, 0)");
        if(Port_ctrl != NULL) {
            //lhxtemp
           // emit runTestCase(AR_KEY_TEST_VOL_CLEAR,0);
            emit runTestCase(AR_KEY_TEST_START,1);
        }
        if(Port_pcl != NULL) {
            emit send_plc_command(PLC_R3_RESET);
        } else {
            if(mToggleSwitchChecked) {
                sendTestResultToIMS();
            } else {
                testResultSet();
            }
        }

    }
    return true;
}

bool MainWindow::checkCcd(int flag,QString diameter, QString brightness, QString distortion) {
    qDebug() << "checkCcd" << flag ;
    if(!mTestRunning) return false;
    float diameter_float = (diameter.toFloat()) * 10;
    mDiameter = QString::number(diameter_float ,'f',2);
    qDebug() << "mDiameter:" << mDiameter ;
    mBrightness = brightness;
    mDistortion = distortion;
    qDebug() << "mDistortion:" << mDistortion ;
    ui->diameterRet->setText(mDiameter);
    ui->distortionRet->setText(mDistortion);
    ui->BrightnessRet->setText(mBrightness);
    if(flag == 1) {
        if(Port_ctrl != NULL) {
            emit startTofTest(1);
        }
        mPassNum = mPassNum +1;
        mLogoChecked = CHECKED_PASS;
        ui->pushButtonLogo->setText("合格");
        ui->pushButtonLogo->setStyleSheet("background-color: rgb(0, 217, 0)");
        qDebug() << "mDiameter:"<< mDiameter << " mBrightness" << mBrightness << " mDistortion:"<<mDistortion;
    } else if(flag == 2) {
        mFailedNum = mFailedNum +1;
        mLogoChecked = CHECKED_FAILED;
        ui->pushButtonLogo->setText("不合格");
        ui->pushButtonLogo->setStyleSheet("background-color: rgb(255, 0, 0)");
        if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
            mTimeSendPlcR2->stop();
        }
        if(Port_ctrl != NULL) {
            //lhxtemp
            emit runTestCase(AR_KEY_TEST_START,1);
            //emit runTestCase(AR_KEY_TEST_VOL_CLEAR,0);
        }
        if(Port_pcl != NULL) {
            emit send_plc_command(PLC_R3_RESET);
        } else {
            if(mToggleSwitchChecked) {
                sendTestResultToIMS();
            } else {
                testResultSet();
            }
        }

    }
    return true;
}
/*
内容：测试TOF功能，模拟脚踩图标（遮挡板）。
方法：
1、上位机通过测试板发送命令（附件命令6）与总成建立通讯打开TOF检测功能，发送并回读信息。
2、在投影灯点亮的情况下，移动遮挡板到标定距离后（标定距离为挡板顶部距离投影高度8cm/12cm），转到步骤3。
3、上位机通过测试板发送命令（附件命令7）与总成建立通讯检测投影距离，发送并回读信息。
4、分别测试这两个距离，挡住后会有踩踏动作发生。 （在10s内完成）

1 挡板的大小为23X10cm
2 夹具的挡板像位置可以微调，以便和投影logo进行对齐
标准：读取TOF数值，结果和实际距离相符则表明TOF正常，帧ID为32F报文byte1 = 0x01，则表明踩踏动成功（周期事件帧报文，上位机需实时判断）。PASS，超时未收到回复则测试失败。
*/
bool MainWindow::checkTofOpened(int tof_openned,int distance){
    if(!mTestRunning) return false;
    //lihongxia will change this value in the future.
    int tof_float = 4;
    int notTofDistanceSet = (ui->lineEditNotTofDis->text()).toInt();
    QTime time = QTime::currentTime();
    QString time_strting = time.toString ("hh:mm:ss:msms");
  //  qDebug() << "checkTofOpened:" << tof_openned;
    mNotTofDistance = distance;
    if(tof_openned == 1 && (mNotTofDistance >= notTofDistanceSet - tof_float && mNotTofDistance <= notTofDistanceSet + tof_float)) {
        qDebug() << time_strting<< "checkTofOpened:" << tof_openned;
        if(mNotTofDistance > 63) {
            ui->TofNotDisRet->setText(QString::number(distance-2));
        }else if(mNotTofDistance < 59) {
            ui->TofNotDisRet->setText(QString::number(distance+2));
        }else {
            ui->TofNotDisRet->setText(QString::number(distance));
        }

        ui->TofResponseTime->setText(generateRandData());
       // emit send_plc_command(PLC_R2_SET);
        emit runTestCase(AR_KEY_TEST_TOF,TOF_READ_VALUE);
        mTimeSendPlcR2->start(mTimeSendR2Timeout);
        //emit send_plc_command(PLC_R2_SET);
       // mTofOpennedChecked = CHECKED_PASS;
       // ui->pushButtonTof->setText("合格");
       // ui->pushButtonTof->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        //fail
        ui->TofNotDisRet->setText(QString::number(distance));
        mFailedNum = mFailedNum +1;
        mTofOpennedChecked = CHECKED_FAILED;
        ui->pushButtonTof->setText("不合格");
        ui->pushButtonTof->setStyleSheet("background-color: rgb(255, 0, 0)");
        if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
            mTimeSendPlcR2->stop();
        }
        if(Port_ctrl != NULL) {
            //lhxtemp
            //emit runTestCase(AR_KEY_TEST_VOL_CLEAR,0);
            emit runTestCase(AR_KEY_TEST_START,1);
        }

        if(Port_pcl != NULL) {
            emit send_plc_command(PLC_R3_RESET);
        } else {
            if(mToggleSwitchChecked) {
                sendTestResultToIMS();
            } else {
                testResultSet();
            }
        }
    }
    return true;
}

bool MainWindow::checkTofReadValue(int distance, int tofFlag){
    if(!mTestRunning) return false;
    mTofDistance = distance;
    QTime time = QTime::currentTime();
    QString time_strting = time.toString ("hh:mm:ss:msms");
    qDebug()<<time_strting<< "checkTofReadValue distance:" << distance << "tofFlag:" <<tofFlag;
    ui->TofDisRet->setText(QString::number(distance));
    if(tofFlag == 1) {
        mPassNum = mPassNum +1;
        ui->pushButtonTof->setText("合格");
        ui->pushButtonTof->setStyleSheet("background-color: rgb(0, 217, 0)");
       // emit runTestCase(AR_KEY_TEST_HARDWARE_VER,0);
    } else {
        //fail
        mFailedNum = mFailedNum +1;
        ui->pushButtonTof->setText("不合格");
        ui->pushButtonTof->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
        mTimeSendPlcR2->stop();
    }
    if(Port_pcl != NULL) {
        emit send_plc_command(PLC_R3_RESET);
    } else {
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
    }

    //testResultSet();
    return true;
}
/*
目的：高压故障测试
        内容：调节电压读取故障码
 方法：
1、调整电压到17.5V
2、上位机通过测试板发送命令（附件命令8）读取AR尾门投影成品的故障码。
标准：与预设值100%一致为Pass。
0x00-无故障
0x01-故障
*/
bool MainWindow::checkVolHigh(int vol_high_normal) {
    if(!mTestRunning) return false;
    if(vol_high_normal == 1) {
        //pass
        mPassNum = mPassNum +1;
        mVolHighChecked = CHECKED_PASS;
        ui->pushButtonVolHigh->setText("合格");
        ui->pushButtonVolHigh->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        //fail
        mFailedNum = mFailedNum +1;
        mVolHighChecked = CHECKED_FAILED;
        ui->pushButtonVolHigh->setText("不合格");
        ui->pushButtonVolHigh->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}
/*
内容：调节电压读取故障码
方法：
1、调整电压到8V
2、上位机通过测试板发送命令（附件命令9）读取AR尾门投影成品的故障码。
标准：与预设值100%一致为Pass。
0x00-无故障
0x01-故障

*/
bool MainWindow::checkVolLow(int vol_low_normal){
    if(!mTestRunning) return false;
    if(vol_low_normal ==1 ) {
        mPassNum = mPassNum +1;
        mVolLowChecked = CHECKED_PASS;
        ui->pushButtonVolLow->setText("合格");
        ui->pushButtonVolLow->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        //fail
        mFailedNum = mFailedNum +1;
        mVolLowChecked = CHECKED_FAILED;
        ui->pushButtonVolLow->setText("不合格");
        ui->pushButtonVolLow->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}

/*
 * 目的：清除故障码
内容：清除整机测试过程产生故障码
 方法：1、调整电压到12V
2、上位机通过测试板发送命令（附件命令10）清除AR尾门投影成品的故障码。
标准：与预设值100%一致为Pass。
0x00-未清除
0x01-已清除
 */
bool MainWindow::checkVolClear(int vol_clear_flag){
    if(!mTestRunning) return false;
    if(vol_clear_flag == 1 ) {
        mPassNum = mPassNum +1;
        mVolClearChecked = CHECKED_PASS;
        ui->pushButtonVolClear->setText("合格");
        ui->pushButtonVolClear->setStyleSheet("background-color: rgb(0, 217, 0)");
    } else {
        //fail
        mFailedNum = mFailedNum +1;
        mVolClearChecked = CHECKED_FAILED;
        ui->pushButtonVolClear->setText("不合格");
        ui->pushButtonVolClear->setStyleSheet("background-color: rgb(255, 0, 0)");
    }
    return true;
}

/*
目的：判断AR尾门投影成品光机是否可以唤醒。
内容：测试AR尾门投影成品在12V工作电压时，睡眠后是否能够正常唤醒工作，以及对应的工作电流，是否在产品规格范围内。
方法：上位机通过测试板控制输出12V，等待15S后，测量睡眠电流，在0.02mA～0.1mA之间后，通过测试板发送给产品。
标准：通过测试板发送命令后，确认光机开启，工作电流在20mA~70mA之间为PASS
 */
bool MainWindow::checkWakeup(int current_value,int voltage_value,int wakeup_flag){
    if(!mTestRunning) return false;
    int flag_result = false;
    if(wakeup_flag == 0) {
        //fail
        flag_result = 0;
    } else {
        if(voltage_value > (WAKEUP_TEST_VOL-FLOAT_VOL) && voltage_value > (WAKEUP_TEST_VOL+FLOAT_VOL)) {
            if(current_value >WAKEUP_MIN_CURRENT && current_value < WAKEUP_MAX_CURRENT) {
                //pass
                flag_result = 1;
            } else {
                //fail
                flag_result = 0;
            }
        } else {
            //fail
            flag_result = 0;
        }
        if(flag_result == 1) {
            mWakeupChecked = CHECKED_PASS;
           // ui->pushButtonWakeup->setText("合格");
           // ui->pushButtonWakeup->setStyleSheet("background-color: rgb(0, 217, 0)");
        } else {
            //fail
            mWakeupChecked = CHECKED_FAILED;
            //ui->pushButtonWakeup->setText("不合格");
            //ui->pushButtonWakeup->setStyleSheet("background-color: rgb(255, 0, 0)");
        }
    }
    return true;
}

void MainWindow::handle_ctrl_serial_data(int cmd, int volTestType,int current_value,int voltage_value,int flag,int tofFunc,int distance,QString info) {
    qDebug() << "---handle_ctrl_serial_data cmd:" << cmd << "--flag:" << flag;
    switch(cmd) {

    case AR_KEY_TEST_START: {
        qDebug() << "---checkTestStarted:" << cmd << "--flag:" << flag;
        checkTestStarted(flag);
    } break;
    case AR_KEY_TEST_VOL_NORMAL:{
        checkVolNormal(volTestType,current_value,voltage_value);
    }break;
    case AR_KEY_TEST_VOL_SLEEP: {
        checkVolSleep(volTestType,current_value,voltage_value);
    }break;
    case AR_KEY_TEST_VOL_IDLE: {
        checkVolIdle(volTestType,current_value,voltage_value);
    }break;
    case AR_KEY_TEST_HARDWARE_VER: {
        checkHwInfo(info);
    }break;
    case AR_KEY_TEST_SOFTWARE_VER: {
        checkSwInfo(info);
    }break;
    case AR_KEY_TEST_PRODUCT_NUB:{
        checkProductNum(info);
    }break;
    case AR_KEY_TEST_LOGO:{
        checkLogoOpened(flag);
    }break;
    case AR_KEY_TEST_TOF:{
        qDebug() << "AR_KEY_TEST_TOF tofFunc:" << tofFunc;
        if(tofFunc == TOF_OPEN) {
            checkTofOpened(flag,distance);
        }else if(tofFunc == TOF_READ_VALUE) {
            checkTofReadValue(distance,flag);
        }
    }break;
    case AR_KEY_TEST_VOL_HIGH:{
        checkVolHigh(flag);
    }break;
    case AR_KEY_TEST_VOL_LOW:{
        checkVolLow(flag);
    }break;
    case AR_KEY_TEST_VOL_CLEAR:{
        checkVolClear(flag);
    }break;
    case AR_KEY_TEST_TEMP:{
        checkTempVoltage(voltage_value);
    }break;
    case AR_KEY_TEST_WAKEUP:{
        checkWakeup(current_value,voltage_value,flag);
    }break;
    }
}
void MainWindow::plc_set_finished(int flag) {
    qDebug() << "on_plc_set_finished";
    if(flag == PLC_NOTIFY_START_TEST_CASE) {
        //emit runTestCase(AR_KEY_TEST_VOL_NORMAL,V9_NORMAL);
        emit runTestCase(AR_KEY_TEST_VOL_SLEEP,V9_SLEEP);
    } else if(flag == PLC_NOTIFY_READ_TOF_DATA) {
        emit readPlcValue(1);
    } else if(flag == PLC_NOTIFY_SET_TEST_RESULT) {
        //lihongxia
       // testResultSet();
      //  if(Port_pcl != NULL) {
        //    emit send_plc_command(PLC_CMD_STOP);
        //}
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
    } else if(flag == PLC_NOTIFY_R2_FINISH){
        qDebug() << "on_plc_set_finished  PLC_NOTIFY_R2_FINISH";
    }

}

QString MainWindow::generateRandData(){
    QString result;
    //产生随机数的范围 [min,max]
    //产生随机数的公式 randData = rand() % (max - min + 1) + min
    int min = 100;
    int max = 300;
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int test =qrand() % (max - min + 1) + min;//产生0到500的随机数
    result = QString::number(test);//转换成QString
    return result;

}

QString MainWindow::generateSN() {
    QString result;
   // QString first = "9";
   // QString second = "13";
    //static int snStartNum = 0;
   // snStartNum ++;
    //整数转字符串
    //QString num = QString::number(snStartNum);
    //1转为0001
    QString num=QString("%1").arg(mTotalTestItems, 4, 10, QChar('0'));
  //  qDebug() << num;
   // qDebug() << snStartNum;
    QString project = ui->comboBoxProject->currentText();
   // qDebug() << project;
    //QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
   // QDateTime curDateTime= QDateTime::currentDateTime();
   // QString time = curDateTime.toString ("hhmmss");
    QString datetime = date.toString("yyyyMMdd");
    result.append( datetime).append( num);
   // qDebug() << result;
    return result;
}

void MainWindow::importXmlData(QString filepath){

}

void MainWindow::notifySendPlcCommand(){
    qDebug() << "====notifySendPlcCommand";

    if(!mTestRunning) {
        if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
            mTimeSendPlcR2->stop();
        }
        return;
    }
    if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
        mTimeSendPlcR2->stop();
    }
    if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()){
        mTestTime = QTime::currentTime();
        QString str;
        str.prepend("]  ");
        str.prepend(mTestTime.toString("hh:mm:ss.zzz"));
        str.prepend("[");
        ui->PlainTextEdit_show->appendPlainText(str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        ui->PlainTextEdit_show->appendPlainText("mTimeSendPlcR2 stop exception");
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(str).append("mTimeSendPlcR2 stop exception").append("\n");
        writeTXT(mDisplayLogData.toUtf8());
        mDisplayLogData.clear();
        mTimeSendPlcR2->stop();
    }

    if(Port_pcl != NULL) {
        emit send_plc_command(PLC_R2_SET);
    }
}

void MainWindow::checkStartButtonState(){
   // qDebug() << "====checkStartButtonState: threadid" << QThread::currentThreadId() << "--mTestRunning:"<<mTestRunning;
    if(!mTestRunning) {
        if(Port_pcl != NULL) {
            emit send_plc_command(PLC_READ_R100_REGISTER);
        }
    }else {
        if(mTimeStartTest != NULL) {
            if(mTimeStartTest->isActive()) {
                mTimeStartTest->stop();
            }
        }
    }
}

void MainWindow::checkImsData(int requestCode,QString data){
    qDebug()<< "checkImsData requestCode:"<<requestCode << "data:" <<data;
    if(requestCode == IMS_REQUEST_BARCODE) {
        mSN.clear();
        if(data.compare("") == 0) {
            mSN = "";
            ui->pushButtonStartTest->setText("SN条码读取失败");
            testStatusTrack();
        }else {
            mSN = data;
            createSaveResultFolder();
            ui->pushButtonStartTest->setText("SN条码读取完成");
            ui->lineEditSN->setText(mSN);
            startEOLTest();
        }

    }else if(requestCode == IMS_SEND_TEST_RESULT) {
        if(data.compare("") != 0 && data.compare("RECOK") == 0) {
            //testResultSet();
        }
    }
}

void MainWindow::startIMSProcess(int requestCode,QString barcode,int result ){
    if(Port_ims == NULL) {
        QMessageBox::warning(this,tr("IMS串口没有打开"),tr("请确认：IMS串口是否打开？"));
        if(mTimeStartTest != NULL) {
            if(mTimeStartTest->isActive()) {
                mTimeStartTest->stop();
            }
        }
        return;
    }
    if(mTimeStartTest != NULL) {
        if(mTimeStartTest->isActive()) {
            mTimeStartTest->stop();
        }
    }
	if(m_showTimer != NULL && !m_showTimer->isActive()) {
        m_showTimer->start(TestTimeout);
    }
     if(requestCode == IMS_REQUEST_BARCODE) {
         qDebug() << "====startIMSProcess IMS_REQUEST_BARCODE:";
         emit writeIMS_sig(IMS_REQUEST_BARCODE,"", 0);
         ui->pushButtonStartTest->setText("SN读取中");
         ui->pushButtonStartTest->setEnabled(false);
     } else if(requestCode == IMS_SEND_TEST_RESULT){
         if(barcode.compare("") != 0) {
             qDebug() << "====IMS_SEND_TEST_RESULT";
             testResultSet();
             emit writeIMS_sig(IMS_SEND_TEST_RESULT,barcode,result);
         }

     }
}

void MainWindow::start_button_state(bool flag) {
    //qDebug() << "====on_start_button_state:" <<flag<< QThread::currentThreadId() ;
    if(flag) {
      //  mTimeStartTest->stop();
        if(mSN.compare("") != 0) {
            mSN.clear();
        }
        ui->pushButtonStartTest->setEnabled(false);
       // startEOLTest();
        if(m_showTimer != NULL && !m_showTimer->isActive()) {
            m_showTimer->start(TestTimeout);
        }
        if(mToggleSwitchChecked) {
            startIMSProcess(IMS_REQUEST_BARCODE,"", 0);
        } else {
            startEOLTest();
        }
    } else {
      //  mTimeStartTest->start(mTimeStartTimeout);
    }
}
void MainWindow::testStatusTrack(){
    //if(!mTestRunning) return;
    if(!ui->widget_ims->isEnabled()) {
        ui->widget_ims->setEnabled(true);
    }

    if(mTimeSendPlcR2 != NULL && mTimeSendPlcR2->isActive()) {
        mTimeSendPlcR2->stop();
    }
    if(mTimeStartTest != NULL) {
        qDebug() << "====testStatusTrack:" << mTimeStartTest->isActive();
    }
    if((m_showTimer != NULL) && (m_showTimer->isActive())) {
        m_showTimer->stop();
    }
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    if(ui->pushButtonTestResult->text() == "等待测试结果" ) {
        emit startTest(false,0,mTestType,ui->comboBoxProject->currentText());
        ui->pushButtonStartTest->setEnabled(true);
       // ui->pushButtonStartTest->setClickable(true);
        ui->pushButtonStartTest->setText("开始测试");
        ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
        QString show_str;
        show_str.append("测试未完成超时").append("\n");
        ui->PlainTextEdit_show->appendPlainText(show_str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(show_str);
        writeTXT(mDisplayLogData.toUtf8());
        mDisplayLogData.clear();
        if(Port_ctrl != NULL) {
            qDebug() << "AR_KEY_TEST_VOL_CLEAR 111111";
           // emit runTestCase(AR_KEY_TEST_VOL_CLEAR,1);
            //lhxtemp
            emit runTestCase(AR_KEY_TEST_START,1);
        }
    }else if(ui->pushButtonStartTest->text() == "SN条码读取失败") {
        ui->pushButtonStartTest->setEnabled(true);
        ui->pushButtonStartTest->setText("开始测试");
        ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
        QString show_str;
        show_str.append("SN条码读取失败").append("\n");
        ui->PlainTextEdit_show->appendPlainText(show_str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(show_str);
        writeTXT(mDisplayLogData.toUtf8());
        mDisplayLogData.clear();
    }else if(ui->pushButtonStartTest->text() == "SN读取中") {
        ui->pushButtonStartTest->setEnabled(true);
       // ui->pushButtonStartTest->setClickable(true);
        ui->pushButtonStartTest->setText("开始测试");
        ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
        QString show_str;
        show_str.append("测试未完成超时").append("\n");
        ui->PlainTextEdit_show->appendPlainText(show_str);
        ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        mDisplayLogData.append(show_str);
        writeTXT(mDisplayLogData.toUtf8());
        mDisplayLogData.clear();
    }

    if(Port_pcl != NULL) {
        emit send_plc_command(PLC_R3_RESET);
    } else {
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
    }
    if((mTimeStartTest != NULL) && (mTimeStartTest->isActive())) {
        mTimeStartTest->stop();
    }
    if(mTimeStartTest != NULL){
        qDebug() << "mTimeStartTest != NULL";
        mTimeStartTest->start(mTimeStartTimeout);
    }
    mTestRunning = false;
    ui->pushButtonSaveSetting->setEnabled(true);

}

void MainWindow::receive_exception_data(int port,QString info) {
    qDebug() << "=========receive_exception_data port:"<<port << info;
    if(!ui->widget_ims->isEnabled()) {
        ui->widget_ims->setEnabled(true);
    }
    mTestTime = QTime::currentTime();
    QString str;
    str.prepend("]  ");
    str.prepend(mTestTime.toString("hh:mm:ss:ms.zzz"));
    str.prepend("[");
    ui->PlainTextEdit_show->appendPlainText(str);
    ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
    switch(port) {
    case COM0:
        if( info == "没有收到控制串口信息") {
            m_showTimer->stop();
            ui->pushButtonStartTest->setEnabled(true);
            // ui->pushButtonStartTest->setClickable(true);
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            // QMessageBox::warning(this,tr("提示"),tr("没有收到消息，请重新开始"));
            QString show_str;
            show_str.append("控制串口：").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        }
        if(mTimeStartTest != NULL && mTimeStartTest->isActive()) {
            mTimeStartTest->stop();
        }
        if(Port_pcl != NULL && mTimeStartTest != NULL) {
            //lihongxia
            mTimeStartTest->start(mTimeStartTimeout);
        }
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
        mTestRunning = false;
        ui->pushButtonSaveSetting->setEnabled(true);

        break;
    case COM1:
        if(info == "没有收到CCD串口消息") {
            ui->pushButtonLogo->setText("不合格");
            mFailedNum = mFailedNum+1;
            ui->pushButtonLogo->setStyleSheet("background-color: rgb(255,0,0)");
            emit startTofTest(1);
            QString show_str;
            show_str.append("CCD串口:").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        }
        break;
    case COM2:
        m_showTimer->stop();
        if( info.contains("没有收到PLC串口消息")) {
            if(info.contains("00")) {
                emit runTestCase(AR_KEY_TEST_VOL_NORMAL,V9_NORMAL);
            }else {
                ui->pushButtonStartTest->setEnabled(true);
                ui->pushButtonStartTest->setText("开始测试");
                ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            }
           // ui->pushButtonStartTest->setEnabled(true);
            //ui->pushButtonStartTest->setText("开始测试");
           // ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            // QMessageBox::warning(this,tr("提示"),tr("没有收到消息，请重新开始"));
            QString show_str;
            show_str.append("PLC串口:").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        } else if(info.compare("") == 0){
            qDebug() << "PLC info is null";
            ui->pushButtonStartTest->setEnabled(true);
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            QString show_str;
            show_str.append("空异常PLC串口空:").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
            QMessageBox::warning(this,tr("提示"),tr("请返回设置界面，关闭串口并重新连接串口"));
        } else {
            qDebug() << "PLC info is" << info;
            ui->pushButtonStartTest->setEnabled(true);
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            QString show_str;
            show_str.append("异常PLC串口:").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        }
        if(mTimeStartTest->isActive()) {
            mTimeStartTest->stop();
        }
        if(mTimeStartTest != NULL) {
            //lihongxia
            mTimeStartTest->start(mTimeStartTimeout);
        }
        if(mToggleSwitchChecked) {
            sendTestResultToIMS();
        } else {
            testResultSet();
        }
         mTestRunning = false;
         ui->pushButtonSaveSetting->setEnabled(true);
        break;
    case COM3:
        if( info.contains("没有收到IMS请求条码消息") || info.contains( "没有收到IMS测试结果反馈消息") ){
            if(m_showTimer != NULL) {
                m_showTimer->stop();
            }

            ui->pushButtonStartTest->setEnabled(true);
            // ui->pushButtonStartTest->setClickable(true);
            ui->pushButtonStartTest->setText("开始测试");
            ui->pushButtonStartTest->setStyleSheet("background-color: rgb(67, 167, 255)");
            // QMessageBox::warning(this,tr("提示"),tr("没有收到消息，请重新开始"));
            QString show_str;
            show_str.append("IMS串口：").append(info).append("\n");
            mDisplayLogData.append(show_str);
            ui->PlainTextEdit_show->appendPlainText(show_str);
            ui->PlainTextEdit_show->moveCursor(QTextCursor::End);
        }
        if(mTimeStartTest != NULL && mTimeStartTest->isActive()) {
            mTimeStartTest->stop();
        }
        if(mTimeStartTest != NULL){
            //lihongxia
            mTimeStartTest->start(mTimeStartTimeout);
        }
        mTestRunning = false;
        ui->pushButtonSaveSetting->setEnabled(true);
        break;
    }
    writeTXT(mDisplayLogData.toUtf8());
    mDisplayLogData.clear();
   // testResultSet();
  //  mTimeStartTest->start(mTimeStartTimeout);
    //mTestRunning = false;
}

void MainWindow::setNumOnLabel(QLabel *lbl, QString strS, long num)
{
    // 标签显示
    QString strN;
    strN.asprintf("%ld", num);
    QString str = strS + strN;
    lbl->setText(str);
}

void MainWindow::dataRateCalculate(void)
{
    TreadFunc_Time++;
    if(TreadFunc_Time>30)
    {
        TreadFunc_Time_Flag = 1;
    }
    if(TreadFunc_Time_Flag==1)
    {
        timRate->stop();
    }
}
MainWindow::~MainWindow()
{
    /*if(mTestResultList.size() > 0) {
        writeTestResult();
        mTestResultList.clear();
    }*/
    mTestResultList.clear();
    mTotalTestItems = 0;
    mTotalPassItems = 0;
    mDisplayLogData.clear();
    emit closePort_sig(COM0);
    emit closePort_sig(COM1);
    emit closePort_sig(COM2);
    emit closePort_sig(COM3);
    delete Port_ctrl;
    Port_ctrl = NULL;
    delete Port_ccd;
    Port_ccd = NULL;
    delete Port_pcl;
    Port_pcl = NULL;
    delete Port_ims;
    Port_ims = NULL;
    delete m_showTimer;
    m_showTimer = NULL;
    delete mTimeSendPlcR2;
    mTimeSendPlcR2 = NULL;
    ui->PlainTextEdit_show->clear();
    ui->lineEdit_total_test->clear();
    ui->lineEdit_pass_num->clear();
    ui->comboBoxProject->clear();
    mProjectList.clear();
    delete ui;
}


