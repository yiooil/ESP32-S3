/**
 * @file key_config.h
 * @brief 按键硬件配置定义
 * @details 定义按键的GPIO配置、触发电平等硬件相关参数
 */

#ifndef __KEY_CONFIG_H__
#define __KEY_CONFIG_H__

#include "key_common.h"  // 包含公共类型定义

#ifdef __cplusplus
extern "C" {
#endif

// 最大按键数量常量定义
#define MAX_KEY_COUNT    8  // 系统支持的最大按键数量
#define GPIO_INPUT_IO_0   0
// 按键配置结构体
typedef struct {
    uint8_t gpio_num;             // GPIO引脚编号（ESP32-S3的GPIO号）
    key_active_level_t active_level; // 触发电平（低电平触发或高电平触发）
    uint8_t pull_enable;          // 是否启用内部上拉/下拉电阻（1启用，0禁用）
    uint8_t pull_up;              // 启用上拉还是下拉（1上拉，0下拉）
    const char* key_name;         // 按键名称（用于调试和日志）
} key_hw_config_t;

// 按键检测器配置结构体
typedef struct {
    const key_hw_config_t* hw_configs;  // 硬件配置数组指针
    uint8_t key_count;                  // 按键数量
    uint32_t scan_interval_ms;          // 轮询扫描间隔（单位：毫秒）
    QueueHandle_t event_queue;          // 事件队列句柄（FreeRTOS队列）
    key_callback_t event_callback;      // 事件回调函数指针
} key_detector_config_t;

/**
 * @brief 初始化按键GPIO硬件
 * @param config 按键硬件配置指针
 * @param count 按键数量
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_gpio_init(const key_hw_config_t* config, uint8_t count);

/**
 * @brief 读取按键当前电平状态
 * @param gpio_num GPIO引脚编号
 * @return uint8_t 返回当前电平（0或1）
 */
uint8_t key_gpio_read(uint8_t gpio_num);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_CONFIG_H__ */