/**
 * @file diever.c
 * @brief 按键检测系统使用示例
 * @details 演示如何初始化、配置和使用按键检测系统
 */

#include <stdio.h>              // 标准输入输出
#include "freertos/FreeRTOS.h"  // FreeRTOS头文件
#include "freertos/task.h"      // FreeRTOS任务相关
#include "esp_log.h"            // ESP32日志系统
#include "key_detector.h"       // 按键检测器头文件
#include "key_config.h"         // 按键配置头文件
#include "key_driver.h"         // 按键驱动头文件
// 日志标签定义
static const char* TAG = "key_driver";
#define GPIO_INPUT_IO_0   0
// 定义按键硬件配置（示例：3个按键，触发电平不同）
static const key_hw_config_t key_hw_configs[] = {
    // 按键0: GPIO0，低电平触发，启用内部上拉
    {
        .gpio_num = GPIO_INPUT_IO_0,
        .active_level = KEY_ACTIVE_LOW,
        .pull_enable = 0,
        .pull_up = 1,
        .key_name = "BOOT按键"
    },
};

// 按键事件处理回调函数
static void key_event_callback(uint8_t key_id, key_event_t event)
{
    // 获取按键名称
    const char* key_name = (key_id < sizeof(key_hw_configs)/sizeof(key_hw_configs[0])) ? 
                          key_hw_configs[key_id].key_name : "未知按键";
    
    // 根据事件类型输出不同信息
    switch (event) {
        case KEY_EVENT_PRESS_DOWN:
            ESP_LOGI(TAG, "[回调] %s 按下", key_name);
            break;
        case KEY_EVENT_PRESS_UP:
            ESP_LOGI(TAG, "[回调] %s 释放", key_name);
            break;
        case KEY_EVENT_SINGLE_CLICK:
            ESP_LOGI(TAG, "[回调] %s 单击", key_name);
            break;
        case KEY_EVENT_DOUBLE_CLICK:
            ESP_LOGI(TAG, "[回调] %s 双击", key_name);
            break;
        case KEY_EVENT_LONG_PRESS_START:
            ESP_LOGI(TAG, "[回调] %s 长按开始", key_name);
            break;
        case KEY_EVENT_LONG_PRESS_HOLD:
            ESP_LOGI(TAG, "[回调] %s 长按保持", key_name);
            break;
        case KEY_EVENT_LONG_PRESS_END:
            ESP_LOGI(TAG, "[回调] %s 长按结束", key_name);
            break;
        case KEY_EVENT_PRESS_REPEAT:
            ESP_LOGI(TAG, "[回调] %s 重复按下", key_name);
            break;
        default:
            break;
    }
}

// 按键事件处理任务
void key_event_task(void* arg)
{
    key_detector_handle_t detector = (key_detector_handle_t)arg;
    key_event_msg_t event_msg;
    
    ESP_LOGI(TAG, "按键事件处理任务启动");
    
    while (1) {
        // 从队列获取按键事件（阻塞等待）
        if (key_detector_get_event(detector, &event_msg, portMAX_DELAY)) {
            // 根据按键ID获取按键名称
            const char* key_name = "未知按键";
            if (event_msg.key_id < sizeof(key_hw_configs)/sizeof(key_hw_configs[0])) {
                key_name = key_hw_configs[event_msg.key_id].key_name;
            }
            
            // 根据事件类型处理
            switch (event_msg.event) {
                case KEY_EVENT_PRESS_DOWN:
                    ESP_LOGI(TAG, "[队列] %s 按下 @%d", key_name, event_msg.timestamp);
                    break;
                case KEY_EVENT_PRESS_UP:
                    ESP_LOGI(TAG, "[队列] %s 释放 @%d", key_name, event_msg.timestamp);
                    break;
                case KEY_EVENT_SINGLE_CLICK:
                    ESP_LOGI(TAG, "[队列] %s 单击 @%d", key_name, event_msg.timestamp);
                    // 单击事件处理示例
                    if (event_msg.key_id == 0) {
                        ESP_LOGI(TAG, "示例: 按键0单击执行特定操作");
                    }
                    break;
                case KEY_EVENT_DOUBLE_CLICK:
                    ESP_LOGI(TAG, "[队列] %s 双击 @%d", key_name, event_msg.timestamp);
                    // 双击事件处理示例
                    if (event_msg.key_id == 1) {
                        ESP_LOGI(TAG, "示例: 按键1双击执行特定操作");
                    }
                    break;
                case KEY_EVENT_LONG_PRESS_START:
                    ESP_LOGI(TAG, "[队列] %s 长按开始 @%d", key_name, event_msg.timestamp);
                    // 长按开始处理示例
                    if (event_msg.key_id == 2) {
                        ESP_LOGI(TAG, "示例: 按键2长按开始，启动某项功能");
                    }
                    break;
                case KEY_EVENT_LONG_PRESS_HOLD:
                    // 长按保持事件较多，使用DEBUG级别减少日志输出
                    ESP_LOGD(TAG, "[队列] %s 长按保持 @%d", key_name, event_msg.timestamp);
                    break;
                case KEY_EVENT_LONG_PRESS_END:
                    ESP_LOGI(TAG, "[队列] %s 长按结束 @%d", key_name, event_msg.timestamp);
                    // 长按结束处理示例
                    if (event_msg.key_id == 2) {
                        ESP_LOGI(TAG, "示例: 按键2长按结束，停止某项功能");
                    }
                    break;
                default:
                    ESP_LOGD(TAG, "[队列] %s 未知事件%d @%d", 
                            key_name, event_msg.event, event_msg.timestamp);
                    break;
            }
        }
    }
}


void key_init(void)
{
    ESP_LOGI(TAG, "ESP32-S3 按键检测系统示例");
    ESP_LOGI(TAG, "编译时间: %s %s", __DATE__, __TIME__);


    // 创建事件队列
    QueueHandle_t event_queue = xQueueCreate(20, sizeof(key_event_msg_t));
    if (event_queue == NULL) {
        ESP_LOGE(TAG, "创建事件队列失败");
        return;
    }
    
    // 配置按键检测器
    key_detector_config_t detector_config = {
        .hw_configs = key_hw_configs,
        .key_count = sizeof(key_hw_configs) / sizeof(key_hw_configs[0]),
        .scan_interval_ms = 10,  // 10ms扫描间隔
        .event_queue = event_queue,
        .event_callback = key_event_callback
    };
    
    // 创建按键检测器
    key_detector_handle_t detector = key_detector_create(&detector_config);
    if (detector == NULL) {
        ESP_LOGE(TAG, "创建按键检测器失败");
        vQueueDelete(event_queue);
        return;
    }
    
    // 启动按键检测
    esp_err_t err = key_detector_start(detector);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "启动按键检测失败: %s", esp_err_to_name(err));
        key_detector_delete(detector);
        vQueueDelete(event_queue);
        return;
    }
    
    // 创建事件处理任务
    xTaskCreate(key_event_task, "key_event", 4096, detector, tskIDLE_PRIORITY + 1, NULL);
}

