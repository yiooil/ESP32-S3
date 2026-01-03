/**
 * @file key_detector.h
 * @brief 按键检测器接口定义
 * @details 定义按键检测器的初始化、启动、停止等公共接口函数
 */

#ifndef __KEY_DETECTOR_H__
#define __KEY_DETECTOR_H__

#include "key_common.h"   // 包含公共类型定义
#include "key_config.h"   // 包含配置定义

#ifdef __cplusplus
extern "C" {
#endif

// 前向声明按键检测器句柄类型
typedef struct key_detector_t* key_detector_handle_t;

/**
 * @brief 创建并初始化按键检测器
 * @param config 检测器配置指针
 * @return key_detector_handle_t 检测器句柄，失败返回NULL
 */
key_detector_handle_t key_detector_create(const key_detector_config_t* config);

/**
 * @brief 启动按键检测任务
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_start(key_detector_handle_t detector);

/**
 * @brief 停止按键检测任务
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_stop(key_detector_handle_t detector);

/**
 * @brief 删除按键检测器并释放资源
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_delete(key_detector_handle_t detector);

/**
 * @brief 从事件队列获取按键事件（非阻塞）
 * @param detector 按键检测器句柄
 * @param event_msg 事件消息结构体指针
 * @param timeout_ms 超时时间（毫秒）
 * @return bool 成功获取返回true，超时或失败返回false
 */
bool key_detector_get_event(key_detector_handle_t detector, 
                           key_event_msg_t* event_msg, 
                           uint32_t timeout_ms);

/**
 * @brief 获取指定按键的当前状态
 * @param detector 按键检测器句柄
 * @param key_id 按键ID
 * @return key_state_t 按键当前状态，无效ID返回KEY_STATE_RELEASED
 */
key_state_t key_detector_get_state(key_detector_handle_t detector, uint8_t key_id);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_DETECTOR_H__ */