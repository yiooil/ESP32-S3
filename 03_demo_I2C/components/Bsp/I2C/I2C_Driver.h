#pragma once  // 防止头文件被重复包含

// 标准库头文件
#include <stdint.h>   // 标准整数类型定义
#include <string.h>   // 内存操作函数（memcpy等）
#include "esp_log.h"  // ESP32日志系统
#include "driver/gpio.h"  // GPIO驱动
#include "driver/i2c.h"   // I2C驱动

/********************* I2C0 配置（用于第一组外设）*********************/
#define I2C0_SCL_IO                  10         /*!< I2C0时钟线使用的GPIO引脚编号 */
#define I2C0_SDA_IO                  11         /*!< I2C0数据线使用的GPIO引脚编号 */
#define I2C0_MASTER_NUM              I2C_NUM_0  /*!< I2C0主机端口号：I2C_NUM_0 */
#define I2C0_MASTER_FREQ_HZ          400000     /*!< I2C0主机时钟频率：400kHz（快速模式）*/
#define I2C0_MASTER_TX_BUF_DISABLE   0          /*!< I2C0发送缓冲区大小：0表示禁用缓冲区 */
#define I2C0_MASTER_RX_BUF_DISABLE   0          /*!< I2C0接收缓冲区大小：0表示禁用缓冲区 */
#define I2C0_MASTER_TIMEOUT_MS       1000       /*!< I2C0操作超时时间：1000毫秒 */

/********************* I2C1 配置（用于第二组外设）*********************/
#define I2C1_SCL_IO                  12         /*!< I2C1时钟线使用的GPIO引脚编号 */
#define I2C1_SDA_IO                  13         /*!< I2C1数据线使用的GPIO引脚编号 */
#define I2C1_MASTER_NUM              I2C_NUM_1  /*!< I2C1主机端口号：I2C_NUM_1 */
#define I2C1_MASTER_FREQ_HZ          400000     /*!< I2C1主机时钟频率：400kHz（快速模式）*/
#define I2C1_MASTER_TX_BUF_DISABLE   0          /*!< I2C1发送缓冲区大小：0表示禁用缓冲区 */
#define I2C1_MASTER_RX_BUF_DISABLE   0          /*!< I2C1接收缓冲区大小：0表示禁用缓冲区 */
#define I2C1_MASTER_TIMEOUT_MS       1000       /*!< I2C1操作超时时间：1000毫秒 */

/********************* 函数声明 *********************/

/* I2C0 相关函数 */
/**
 * @brief 初始化I2C0总线
 * @note 配置I2C0为主机模式，设置引脚、上拉电阻和时钟频率
 */
void I2C0_Init(void);

/**
 * @brief 通过I2C0总线写入数据到设备
 * @param Driver_addr 从设备地址（7位地址，不包含读写位）
 * @param Reg_addr 要写入的寄存器地址
 * @param Reg_data 要写入的数据缓冲区指针
 * @param Length 要写入的数据长度（字节数）
 * @return esp_err_t ESP_OK表示成功，其他值为错误码
 * @note 函数内部会将寄存器地址和数据打包发送
 */
esp_err_t I2C0_Write(uint8_t Driver_addr, uint8_t Reg_addr, 
                     const uint8_t *Reg_data, uint32_t Length);

/**
 * @brief 通过I2C0总线从设备读取数据
 * @param Driver_addr 从设备地址（7位地址，不包含读写位）
 * @param Reg_addr 要读取的寄存器地址
 * @param Reg_data 读取数据存储缓冲区指针
 * @param Length 要读取的数据长度（字节数）
 * @return esp_err_t ESP_OK表示成功，其他值为错误码
 * @note 先发送寄存器地址，然后读取数据
 */
esp_err_t I2C0_Read(uint8_t Driver_addr, uint8_t Reg_addr, 
                    uint8_t *Reg_data, uint32_t Length);

/* I2C1 相关函数 */
/**
 * @brief 初始化I2C1总线
 * @note 配置I2C1为主机模式，设置引脚、上拉电阻和时钟频率
 */
void I2C1_Init(void);

/**
 * @brief 通过I2C1总线写入数据到设备
 * @param Driver_addr 从设备地址（7位地址，不包含读写位）
 * @param Reg_addr 要写入的寄存器地址
 * @param Reg_data 要写入的数据缓冲区指针
 * @param Length 要写入的数据长度（字节数）
 * @return esp_err_t ESP_OK表示成功，其他值为错误码
 * @note 函数内部会将寄存器地址和数据打包发送
 */
esp_err_t I2C1_Write(uint8_t Driver_addr, uint8_t Reg_addr, 
                     const uint8_t *Reg_data, uint32_t Length);

/**
 * @brief 通过I2C1总线从设备读取数据
 * @param Driver_addr 从设备地址（7位地址，不包含读写位）
 * @param Reg_addr 要读取的寄存器地址
 * @param Reg_data 读取数据存储缓冲区指针
 * @param Length 要读取的数据长度（字节数）
 * @return esp_err_t ESP_OK表示成功，其他值为错误码
 * @note 先发送寄存器地址，然后读取数据
 */
esp_err_t I2C1_Read(uint8_t Driver_addr, uint8_t Reg_addr, 
                    uint8_t *Reg_data, uint32_t Length);
