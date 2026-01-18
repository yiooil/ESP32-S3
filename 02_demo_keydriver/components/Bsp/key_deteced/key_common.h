/**
 * @file key_common.h
 * @brief 按键检测系统公共类型和宏定义
 * @details 定义了按键事件类型、触发电平、状态等公共枚举和类型
 */
#ifndef __KEY_COMMON_H__
#define __KEY_COMMON_H__

#include <stdint.h>            // 标准整数类型
#include <stdbool.h>           // 布尔类型
#include "freertos/FreeRTOS.h" // FreeRTOS头文件
#include "freertos/task.h"     // FreeRTOS任务相关
#include "freertos/queue.h"    // FreeRTOS队列相关

// 按键事件类型枚举
typedef enum
{
    KEY_EVENT_NONE = 0,         // 无按键事件发生
    KEY_EVENT_PRESS_DOWN,       // 按键按下事件（按下瞬间）
    KEY_EVENT_PRESS_UP,         // 按键释放事件（释放瞬间）
    KEY_EVENT_PRESS_REPEAT,     // 重复按下事件（用于连按功能）
    KEY_EVENT_SINGLE_CLICK,     // 单击事件（按下后短时间释放）
    KEY_EVENT_DOUBLE_CLICK,     // 双击事件（快速按两次）
    KEY_EVENT_LONG_PRESS_START, // 长按开始事件（达到长按时间阈值）
    KEY_EVENT_LONG_PRESS_HOLD,  // 长按保持事件（长按过程中周期性触发）
    KEY_EVENT_LONG_PRESS_END,   // 长按结束事件（长按后释放）
    KEY_EVENT_MAX               // 事件类型最大值（用于边界检查）
} key_event_t;

// 按键触发电平枚举
typedef enum
{
    KEY_ACTIVE_LOW = 0, // 低电平触发（按键按下时GPIO为低电平）
    KEY_ACTIVE_HIGH     // 高电平触发（按键按下时GPIO为高电平）
} key_active_level_t;

// 按键内部状态枚举
typedef enum
{
    KEY_STATE_RELEASED = 0, // 按键释放状态（稳定状态）
    KEY_STATE_PRESSED,      // 按键按下状态（稳定状态）
    KEY_STATE_DEBOUNCE,     // 按键消抖中（状态切换中的临时状态）
    KEY_STATE_WAIT_RELEASE  // 等待释放状态（用于检测双击等复合事件）
} key_state_t;

// 按键事件回调函数类型定义
typedef void (*key_callback_t)(uint8_t key_id, key_event_t event);

// 按键事件消息结构体（用于通过队列传递）
typedef struct
{
    uint8_t key_id;     // 按键ID（对应按键配置中的索引）
    key_event_t event;  // 按键事件类型
    uint32_t timestamp; // 事件时间戳（单位：毫秒）
} key_event_msg_t;

// 按键消抖时间常量（单位：毫秒）
#define KEY_DEBOUNCE_TIME_MS 20 // 消抖时间，防止机械抖动
// 单击时间阈值常量（单位：毫秒）
#define KEY_CLICK_TIME_MS 300 // 单击最大持续时间
// 双击时间间隔阈值常量（单位：毫秒）
#define KEY_DOUBLE_CLICK_INTERVAL_MS 500 // 双击间隔最大时间
// 长按开始时间阈值常量（单位：毫秒）
#define KEY_LONG_PRESS_TIME_MS 1000 // 长按开始时间阈值
// 长按保持触发间隔常量（单位：毫秒）
#define KEY_LONG_PRESS_HOLD_INTERVAL_MS 200 // 长按保持事件触发间隔





#endif                                      /* __KEY_COMMON_H__ */