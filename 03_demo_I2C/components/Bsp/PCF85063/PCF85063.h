#pragma once  // 防止头文件被重复包含

#include "I2C_Driver.h"  // 包含I2C驱动头文件


#ifndef PCF85063_USE_I2C0
#define PCF85063_USE_I2C0  1  // 1=使用I2CO，0=使用I2C1
#endif

#if PCF85063_USE_I2C0
    #define I2C_WRITE I2C0_Write
    #define I2C_READ  I2C0_Read
#else
    #define I2C_WRITE I2C1_Write
    #define I2C_READ  I2C1_Read
#endif


/********************* PCF85063 设备地址定义 *********************/
#define PCF85063_ADDRESS   (0x51)  /*!< PCF85063 RTC设备的I2C地址（7位地址）*/

/********************* 时间偏移常量定义 *********************/
#define YEAR_OFFSET         (1970)  /*!< 年份偏移量，PCF85063只存储0-99年，真实年份=存储值+1970 */

/********************* 寄存器地址定义 *********************/

/* 控制与状态寄存器 */
#define RTC_CTRL_1_ADDR     (0x00)  /*!< 控制寄存器1地址 */
#define RTC_CTRL_2_ADDR     (0x01)  /*!< 控制寄存器2地址 */
#define RTC_OFFSET_ADDR     (0x02)  /*!< 时钟偏移调整寄存器地址 */
#define RTC_RAM_by_ADDR     (0x03)  /*!< RAM字节寄存器地址 */

/* 时间与日期寄存器 */
#define RTC_SECOND_ADDR     (0x04)  /*!< 秒寄存器地址（0-59） */
#define RTC_MINUTE_ADDR     (0x05)  /*!< 分寄存器地址（0-59） */
#define RTC_HOUR_ADDR       (0x06)  /*!< 时寄存器地址（0-23或1-12） */
#define RTC_DAY_ADDR        (0x07)  /*!< 日寄存器地址（1-31） */
#define RTC_WDAY_ADDR       (0x08)  /*!< 星期寄存器地址（0-6，0=星期天） */
#define RTC_MONTH_ADDR      (0x09)  /*!< 月寄存器地址（1-12） */
#define RTC_YEAR_ADDR       (0x0A)  /*!< 年寄存器地址（0-99），真实年份=1970+寄存器值 */

/* 闹钟寄存器 */
#define RTC_SECOND_ALARM    (0x0B)  /*!< 闹钟秒寄存器地址 */
#define RTC_MINUTE_ALARM    (0x0C)  /*!< 闹钟分寄存器地址 */
#define RTC_HOUR_ALARM      (0x0D)  /*!< 闹钟时寄存器地址 */
#define RTC_DAY_ALARM       (0x0E)  /*!< 闹钟日寄存器地址 */
#define RTC_WDAY_ALARM      (0x0F)  /*!< 闹钟星期寄存器地址 */

/* 定时器寄存器 */
#define RTC_TIMER_VAL       (0x10)  /*!< 定时器值寄存器地址 */
#define RTC_TIMER_MODE      (0x11)  /*!< 定时器模式寄存器地址 */

/********************* RTC_CTRL_1 寄存器位定义 *********************/
#define RTC_CTRL_1_EXT_TEST (0x80)  /*!< 扩展测试模式：0=正常模式，1=测试模式 */
#define RTC_CTRL_1_STOP     (0x20)  /*!< 停止位：0=RTC时钟运行，1=RTC时钟停止 */
#define RTC_CTRL_1_SR       (0X10)  /*!< 软件复位位：0=不复位，1=触发软件复位 */
#define RTC_CTRL_1_CIE      (0X04)  /*!< 校准中断使能：0=不产生中断，1=每次校准周期产生中断脉冲 */
#define RTC_CTRL_1_12_24    (0X02)  /*!< 小时格式选择：0=24小时制，1=12小时制 */
#define RTC_CTRL_1_CAP_SEL  (0X01)  /*!< 内部负载电容选择：0=7pF，1=12.5pF */

/********************* RTC_CTRL_2 寄存器位定义 *********************/
#define RTC_CTRL_2_AIE      (0X80)  /*!< 闹钟中断使能：0=禁用，1=使能 */
#define RTC_CTRL_2_AF       (0X40)  /*!< 闹钟标志位：0=未触发/已清除，1=已触发/未清除 */
#define RTC_CTRL_2_MI       (0X20)  /*!< 分钟中断使能：0=禁用，1=使能 */
#define RTC_CTRL_2_HMI      (0X10)  /*!< 半分钟中断使能 */
#define RTC_CTRL_2_TF       (0X08)  /*!< 定时器标志位：0=定时器未超时，1=定时器超时 */

/********************* 时钟偏移寄存器位定义 *********************/
#define RTC_OFFSET_MODE     (0X80)  /*!< 偏移模式选择位 */

/********************* 定时器模式寄存器位定义 *********************/
#define RTC_TIMER_MODE_TE   (0X04)  /*!< 定时器使能：0=禁用，1=使能 */
#define RTC_TIMER_MODE_TIE  (0X02)  /*!< 定时器中断使能：0=禁用，1=使能 */
#define RTC_TIMER_MODE_TI_TP (0X01) /*!< 定时器中断模式：0=中断跟随定时器标志，1=中断产生脉冲 */

/********************* 格式常量定义 *********************/
#define RTC_ALARM           (0x80)  /*!< 闹钟禁用标志，设置AEN_x寄存器时使用 */
#define RTC_CTRL_1_DEFAULT  (0x00)  /*!< 控制寄存器1默认值 */
#define RTC_CTRL_2_DEFAULT  (0x00)  /*!< 控制寄存器2默认值 */
#define RTC_TIMER_FLAG      (0x08)  /*!< 定时器标志位掩码 */

/********************* 日期时间结构体定义 *********************/
/**
 * @brief 日期时间结构体
 * @note 用于存储和传递日期时间信息
 */
typedef struct {
    uint16_t year;   /*!< 年份（4位数，如2024） */
    uint8_t month;   /*!< 月份（1-12） */
    uint8_t day;     /*!< 日期（1-31） */
    uint8_t dotw;    /*!< 星期几（0-6，0=星期天） */
    uint8_t hour;    /*!< 小时（0-23） */
    uint8_t minute;  /*!< 分钟（0-59） */
    uint8_t second;  /*!< 秒（0-59） */
} datetime_t;

/********************* 外部变量声明 *********************/
extern datetime_t datetime;  /*!< 全局日期时间变量 */

/********************* 函数声明 *********************/

/**
 * @brief PCF85063初始化函数
 * @note 配置RTC为正常模式，时钟运行，24小时制，内部负载电容12.5pF
 */
void PCF85063_Init(void);

/**
 * @brief PCF85063主循环函数
 * @note 用于定期读取和更新当前时间
 */
void PCF85063_Loop(void);

/**
 * @brief PCF85063软件复位函数
 * @note 触发软件复位，重置RTC内部状态
 */
void PCF85063_Reset(void);

/**
 * @brief 设置时间（时、分、秒）
 * @param time 包含时间信息的结构体
 */
void PCF85063_Set_Time(datetime_t time);

/**
 * @brief 设置日期（年、月、日、星期）
 * @param date 包含日期信息的结构体
 */
void PCF85063_Set_Date(datetime_t date);

/**
 * @brief 设置完整的日期和时间
 * @param time 包含完整日期时间信息的结构体
 */
void PCF85063_Set_All(datetime_t time);

/**
 * @brief 读取当前日期和时间
 * @param time 用于存储读取结果的日期时间结构体指针
 */
void PCF85063_Read_Time(datetime_t *time);

/**
 * @brief 使能闹钟功能并清除闹钟标志
 */
void PCF85063_Enable_Alarm(void);

/**
 * @brief 获取闹钟标志位状态
 * @return uint8_t 闹钟标志位状态
 */
uint8_t PCF85063_Get_Alarm_Flag();

/**
 * @brief 设置闹钟时间
 * @param time 包含闹钟时间的结构体
 */
void PCF85063_Set_Alarm(datetime_t time);

/**
 * @brief 读取当前设置的闹钟时间
 * @param time 用于存储读取结果的日期时间结构体指针
 */
void PCF85063_Read_Alarm(datetime_t *time);

/**
 * @brief 将日期时间结构体转换为字符串
 * @param datetime_str 用于存储转换结果的字符串缓冲区
 * @param time 要转换的日期时间结构体
 */
void datetime_to_str(char *datetime_str, datetime_t time);

/********************* 星期格式说明 *********************/
/*
 * 星期格式定义：
 * 0 - 星期天 (Sunday)
 * 1 - 星期一 (Monday)
 * 2 - 星期二 (Tuesday)
 * 3 - 星期三 (Wednesday)
 * 4 - 星期四 (Thursday)
 * 5 - 星期五 (Friday)
 * 6 - 星期六 (Saturday)
 */