#ifndef DATATYPEDEF_H
#define DATATYPEDEF_H
#include <QMetaType>
#include <QDateTime>
#define CMD_TEST_MAX_NUB                33

#define BYTE_TO_UINT8(MSB,LSB) ((((uint16_t)MSB) << 8 & 0xFF00) | (((uint16_t)LSB) & 0xFF))
#define BYTE_TO_UINT16(MSB,LSB) ((((uint16_t)MSB) << 8 & 0xFF00) | (((uint16_t)LSB) & 0xFF))
#define BYTE_TO_UINT32(HH,HL,LH,LL) ((((HH & 0xff) << 24) & 0xff000000) | \
                                    (((HL & 0xff) << 16) & 0x00ff0000) | \
                                    (((LH & 0xff) << 8) & 0x0000ff00) | \
                                    (((LL & 0xff)) & 0x000000ff))
#define EOL_TEST_CASE_NUM
#define TOTAL_TEST_CASE_EOL_C206 16
#define TOTAL_TEST_CASE_EOL_C03 16
#define TOTAL_TEST_CASE_EOL_N61 16
#define TOTAL_TEST_CASE_EOL_N60 16
#define TOTAL_TEST_CASE_EOL_C46 16
//上下浮动电压
#define FLOAT_VOL  1
#define TEST_VOL_9 9
#define TEST_VOL_13 13
#define TEST_VOL_15 15
#define WAKEUP_TEST_VOL 12
//vol_normal test
/* 标准：9V时工作电流小于1.3A大于1.1A, 即为PASS。
* 13V时工作电流小于0.85A大于0.6A, 即为PASS。
*15V时工作电流小于0.75A大于0.5A, 即为PASS。
*/
#define VOL_9_NORMAL_MIN_CURRENT 1100
#define VOL_9_NORMAL_MAX_CURRENT 1300
#define VOL_13_NORMAL_MIN_CURRENT 600
#define VOL_13_NORMAL_MAX_CURRENT 850
#define VOL_15_NORMAL_MIN_CURRENT 500
#define VOL_15_NORMAL_MAX_CURRENT 750

//vol_sleep test
/*标准：13V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。
标准：15V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。
标准：9V时工作电流小于 0.1mA 大于0.02mA, 即为PASS。*/
#define VOL_9_SLEEP_MIN_CURRENT 20
#define VOL_9_SLEEP_MAX_CURRENT 100
#define VOL_13_SLEEP_MIN_CURRENT 20
#define VOL_13_SLEEP_MAX_CURRENT 100
#define VOL_15_SLEEP_MIN_CURRENT 20
#define VOL_15_SLEEP_MAX_CURRENT 100
//Vol_idle_test
/*
标准：13V时工作电流小于 60mA 大于20mA, 即为PASS。
标准：15V时工作电流小于 60mA 大于20mA, 即为PASS。
标准：9V时工作电流小于 70mA 大于20mA, 即为PASS。
*/
#define VOL_9_IDLE_MIN_CURRENT 20
#define VOL_9_IDLE_MAX_CURRENT 70
#define VOL_13_IDLE_MIN_CURRENT 20
#define  VOL_13_IDLE_MAX_CURRENT 60
#define VOL_15_IDLE_MIN_CURRENT 20
#define VOL_15_IDLE_MAX_CURRENT 60

//wakeupN
#define WAKEUP_MIN_CURRENT 20
#define WAKEUP_MAX_CURRENT 70
enum COMId
{
    COM_NULL =-1,
    COM0 = 0,
    COM1 = 1,
    COM2 = 2,
    COM3 = 3
};

typedef enum{
    G59 = 0,
    C206   = 1,
    C03 = 2,
    N61 = 3,
    N60 = 4,
    C46 = 5
} enTestProject;

typedef enum{
    NOT_CHECKED = 0,
    CHECKED = 1,
    CHECKED_PASS = 2,
    CHECKED_FAILED = 3,
} caseCheckedState;

typedef enum{
    V13 = 0,
    V9   = 1,
    V15 = 2
} enVoltageValue;

typedef enum{
    V13_NORMAL = 0,
    V9_NORMAL  = 1,
    V15_NORMAL = 2,
    V9_SLEEP = 3,
    V13_SLEEP  = 4,
    V15_SLEEP = 5,
    V9_IDLE  = 6,
    V13_IDLE = 7,
    V15_IDLE = 8,
} enVolTestType;

typedef enum{
    TOF_OPEN = 0,
    TOF_READ_VALUE =1,
    TOF_TEST=2
} enTofFunc;
typedef enum{
    FUNC_TEST = 0,
    DV_TEST   = 1,
    EOL_TEST  = 2,
    BOARD_TEST =3
} enTestFun;

typedef enum{
    AR_KEY_SEND_HEAD = 102,
    AR_KEY_RECEIVE_HEAD = 101
} enFrame_head;
typedef enum{
    AR_KEY_NO_CMD = 0,
    AR_KEY_TEST_START  = 1,
    AR_KEY_TEST_VOL_NORMAL    = 2,
    AR_KEY_TEST_VOL_SLEEP     =  3,
    AR_KEY_TEST_VOL_IDLE      =  4,
    AR_KEY_TEST_TEMP         =   5,
    AR_KEY_TEST_HARDWARE_VER = 6,
    AR_KEY_TEST_SOFTWARE_VER  =7,
    AR_KEY_TEST_PRODUCT_NUB  =8,
    AR_KEY_TEST_LOGO       =   9,
    AR_KEY_TEST_TOF        =    10,
    AR_KEY_TEST_VOL_HIGH     =  11,
    AR_KEY_TEST_VOL_LOW    =   12,
    AR_KEY_TEST_VOL_CLEAR  =    13,
    AR_KEY_TEST_WAKEUP     =   14,
} enTest_cmd;

typedef enum{
    PLC_CMD_NULL = -1,
    PLC_R1_SET = 0,
    PLC_R2_SET = 1,
    PLC_R3_SET = 2,
    PLC_R11_SET= 3,
    PLC_R12_SET = 4,
    PLC_R1_RESET = 5,
    PLC_R2_RESET = 6,
    PLC_R3_RESET = 7,
    PLC_R11_RESET = 8,
    PLC_R12_RESET = 9,
    PLC_CMD_STOP = 100,
} enPlc_command;

typedef enum{
    PLC_READ_NULL = -1,
    PLC_READ_R100_REGISTER = 20,
    PLC_READ_R101_REGISTER = 21,
    PLC_READ_R102_REGISTER= 22,
    PLC_READ_R103_REGISTER = 23,
    PLC_READ_R104_REGISTER = 24,
    PLC_READ_R105_REGISTER = 25,
    PLC_READ_R106_REGISTER = 26,
    PLC_READ_R107_REGISTER = 27,
} enPlc_register_command;

typedef enum{
    PLC_NOTIFY_START_TEST_CASE= 0,
    PLC_NOTIFY_SET_TEST_RESULT = 1,
    PLC_NOTIFY_READ_TOF_DATA =2,
    PLC_NOTIFY_R2_FINISH = 3
} plc_nofify_message;

typedef enum{
    IMS_DEFAULT = -1,
    IMS_REQUEST_BARCODE= 0,
    IMS_SEND_TEST_RESULT = 1
} ims_request_message;

typedef struct {
    int       id;
    QString   dateTime;
    QString     SN;
    QString     result;
    int         totalTestItems;
    int         totalPassItems;
    int         normal_current_value_9V;
    int         normal_current_value_13V;
    int         normal_current_value_15V;
    float         idle_current_value_9V;
    float         idle_current_value_13V;
    float         idle_current_value_15V;
    float         idle_current_value;
    float         sleep_current_value_9V;
    float         sleep_current_value_13V;
    float         sleep_current_value_15V;
    int         nottofdistance;
    int         tofdistance;
    QString     hw;
    QString     sw;
    QString     produce_num;
    QString     diamater;
    QString     brightness;
    QString     distortion;
} TestResultEntry;

/*typedef enum{
    Default = -1,
    AR_KEY_TEST_VOL_NOMAL_9V = 0,
    AR_KEY_TEST_VOL_NOMAL_13V  = 1,
    AR_KEY_TEST_VOL_NOMAL_15V = 2,
    AR_KEY_TEST_VOL_SLEEP_9V = 3,
    AR_KEY_TEST_VOL_SLEEP_13V  = 4,
    AR_KEY_TEST_VOL_SLEEP_15V = 5,
    AR_KEY_TEST_VOL_IDLE_9V   = 6,
    AR_KEY_TEST_VOL_IDLE_13V = 7,
    AR_KEY_TEST_VOL_IDLE_15V = 8,
    AR_KEY_TEST_TEMP = 9,
    AR_KEY_TEST_HARDWARE_VER = 10,
    AR_KEY_TEST_SOFTWARE_VER = 11,
    AR_KEY_TEST_PRODUCT_NUB = 12,
    AR_KEY_TEST_LOGO = 13,
    AR_KEY_TEST_TOF = 14,
    AR_KEY_TEST_VOL_HIGH = 15,
    AR_KEY_TEST_VOL_LOW = 16,
    AR_KEY_TEST_VOL_CLEAR = 17,
    AR_KEY_TEST_WAKEUP = 18,
    CCD = 19,
    IMS_BARCODE = 20,
    IMS_TEST_RESULT = 21,
    PLC_R1_SET = 22,
    PLC_R2_SET = 23,
    PLC_R3_SET = 24,
    PLC_R4_RESET = 25,
    PLC_R5_RESET = 26,
    PLC_R6_RESET = 27,
    PLC_R11_SET = 28,
    PLC_R12_SET = 29,
    PLC_R11_RESET = 30,
    PLC_R12_RESET = 31,
} allTestCaseStatus;*/

typedef struct {
    QString     project_name;
    QString         normal_value_9V_min;
    QString         normal_value_9V_max;
    QString         normal_value_15V_min;
    QString         normal_value_15V_max;
    QString         normal_value_13V_min;
    QString         normal_value_13V_max;
    QString         idle_value_9V_min;
    QString         idle_value_9V_max;
    QString         idle_value_15V_min;
    QString         idle_value_15V_max;
    QString         idle_value_13V_min;
    QString         idle_value_13V_max;
    QString         sleep_value_9V_min;
    QString         sleep_value_9V_max;
    QString         sleep_value_15V_min;
    QString         sleep_value_15V_max;
    QString         sleep_value_13V_min;
    QString         sleep_value_13V_max;
    QString         nottofdistance;
    QString         tofdistance;
    QString     hw;
    QString     sw;
    QString     produce_num;
    QString     vol_float;
} ParamSettingEntry;
#endif // DATATYPEDEF_H
