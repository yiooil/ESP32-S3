/**
 * @file key_detector.c
 * @brief 按键检测器核心实现
 * @details 实现按键状态机、事件检测、消抖处理和消息发送功能
 */

#include "key_detector.h"
#include "esp_log.h"        // ESP32日志系统
#include "esp_timer.h"      // ESP32高精度定时器

// 日志标签定义
static const char* TAG = "key_detector";



// 按键信息结构体（每个按键一个实例）
typedef struct {
    uint8_t key_id;                 // 按键ID（索引）
    key_state_t state;              // 当前状态
    key_state_t last_state;         // 上一次状态
    uint32_t press_timestamp;       // 按下时间戳（毫秒）
    uint32_t release_timestamp;     // 释放时间戳（毫秒）
    uint32_t last_event_timestamp;  // 上一次事件时间戳（毫秒）
    uint8_t click_count;            // 点击计数（用于检测双击）
    bool long_press_detected;       // 长按是否已检测
} key_info_t;

// 按键检测器结构体
struct key_detector_t{
    key_detector_config_t config;           // 配置信息
    key_info_t key_infos[MAX_KEY_COUNT];    // 按键信息数组
    TaskHandle_t scan_task_handle;          // 扫描任务句柄
    bool task_running;                      // 任务运行标志
    uint32_t tick_counter;                  // 时间滴答计数器
};

/**
 * @brief 检查按键是否被按下（考虑触发电平）
 * @param detector 按键检测器指针
 * @param key_id 按键ID
 * @return bool 按下返回true，释放返回false
 */
static bool is_key_pressed(struct key_detector_t* detector, uint8_t key_id)
{
    // 参数边界检查
    if (detector == NULL || key_id >= detector->config.key_count) {
        return false;
    }
    
    // 获取按键硬件配置
    const key_hw_config_t* hw_cfg = &detector->config.hw_configs[key_id];
    
    // 读取GPIO电平
    uint8_t level = key_gpio_read(hw_cfg->gpio_num);
    
    // 根据触发电平判断是否按下
    if (hw_cfg->active_level == KEY_ACTIVE_LOW) {
        return (level == 0);  // 低电平触发：低电平表示按下
    } else {
        return (level == 1);  // 高电平触发：高电平表示按下
    }
}

/**
 * @brief 获取当前时间（毫秒）
 * @return uint32_t 当前时间戳（毫秒）
 */
static uint32_t get_current_time_ms(void)
{
    // 使用FreeRTOS的tick计数转换为毫秒
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

/**
 * @brief 计算时间差（毫秒）
 * @param start 开始时间
 * @param end 结束时间
 * @return uint32_t 时间差（毫秒）
 */
static uint32_t time_diff_ms(uint32_t earlier, uint32_t later) {
    if (later >= earlier) {
        return later - earlier;
    } else {
        // 处理时间回绕
        return (UINT32_MAX - earlier) + later;
    }
}

/**
 * @brief 发送按键事件到队列
 * @param detector 按键检测器指针
 * @param key_id 按键ID
 * @param event 按键事件
 * @return bool 发送成功返回true，失败返回false
 */
static bool send_key_event(struct key_detector_t* detector, uint8_t key_id, key_event_t event)
{
    // 不发送无意义的事件
    if (event == KEY_EVENT_NONE) {
        return false;
    }
    
    // 创建事件消息
    key_event_msg_t msg = {
        .key_id = key_id,
        .event = event,
        .timestamp = get_current_time_ms()
    };
    
    bool sent = false;
    
    // 如果有事件队列，发送到队列
    if (detector->config.event_queue != NULL) {
        // 非阻塞方式发送，如果队列满则丢弃事件
        sent = xQueueSend(detector->config.event_queue, &msg, 0) == pdTRUE;
        if (!sent) {
            ESP_LOGW(TAG, "事件队列已满，丢弃按键%d的事件%d", key_id, event);
        }
    }
    
    // 如果有回调函数，调用回调
    if (detector->config.event_callback != NULL) {
        detector->config.event_callback(key_id, event);
        sent = true;
    }
    
    // 记录事件时间戳
    if (sent && detector->config.key_count > key_id) {
        detector->key_infos[key_id].last_event_timestamp = msg.timestamp;
        ESP_LOGD(TAG, "按键%d: 事件%d @%d", key_id, event, msg.timestamp);
    }
    
    return sent;
}

/**
 * @brief 按键扫描任务主函数
 * @param pvParameters 参数指针（指向按键检测器实例）
 */
static void key_scan_task(void* pvParameters)
{
    struct key_detector_t* detector = (struct key_detector_t*)pvParameters;
    
    ESP_LOGI(TAG, "按键扫描任务启动");
    
    while (detector->task_running) {
        // 遍历所有按键
        for (uint8_t i = 0; i < detector->config.key_count && detector->task_running; i++) {
            key_info_t* key_info = &detector->key_infos[i];
            const key_hw_config_t* hw_cfg = &detector->config.hw_configs[i];
            
            // 读取按键当前状态
            bool pressed = is_key_pressed(detector, i);
            uint32_t current_time = get_current_time_ms();
            
            // 状态机处理
            switch (key_info->state) {
                case KEY_STATE_RELEASED: {
                    // 释放状态：检测是否有按下动作
                    if (pressed) {
                        key_info->state = KEY_STATE_DEBOUNCE;
                        key_info->press_timestamp = current_time;
                        key_info->last_event_timestamp = current_time;
                        ESP_LOGD(TAG, "按键%d: 释放 -> 消抖", i);
                    }
                    break;
                }
                
                case KEY_STATE_PRESSED: {
                    // 按下状态：检测是否有释放动作或长按事件
                    if (!pressed) {
                        key_info->state = KEY_STATE_DEBOUNCE;
                        key_info->release_timestamp = current_time;
                        key_info->last_event_timestamp = current_time;
                        ESP_LOGD(TAG, "按键%d: 按下 -> 消抖(释放)", i);
                    } else {
                        // 检查长按事件
                        uint32_t press_duration = time_diff_ms(key_info->press_timestamp, current_time);
                        
                        // 长按开始事件
                        if (!key_info->long_press_detected && 
                            press_duration >= KEY_LONG_PRESS_TIME_MS) {
                            send_key_event(detector, i, KEY_EVENT_LONG_PRESS_START);
                            key_info->long_press_detected = true;
                            key_info->last_event_timestamp = current_time;
                        }
                        
                        // 长按保持事件（周期性触发）
                        if (key_info->long_press_detected && 
                            press_duration - time_diff_ms(key_info->last_event_timestamp, current_time) >= 
                            KEY_LONG_PRESS_HOLD_INTERVAL_MS) {
                            send_key_event(detector, i, KEY_EVENT_LONG_PRESS_HOLD);
                            key_info->last_event_timestamp = current_time;
                        }
                    }
                    break;
                }
                
                case KEY_STATE_DEBOUNCE: {
                    // 消抖状态：等待稳定
                    uint32_t debounce_time = pressed ? 
                        time_diff_ms(key_info->press_timestamp, current_time) :
                        time_diff_ms(key_info->release_timestamp, current_time);
                    
                    if (debounce_time >= KEY_DEBOUNCE_TIME_MS) {
                        // 消抖完成，确认状态变化
                        bool still_pressed = is_key_pressed(detector, i);
                        
                        if (pressed == still_pressed) {
                            // 状态确认
                            if (pressed) {
                                // 确认按下
                                key_info->state = KEY_STATE_PRESSED;
                                
                                // 如果是双击中的第二次按下
                                if (key_info->click_count == 1) {
                                    // 双击中的第二次按下，不发送单独的按下事件
                                    ESP_LOGD(TAG, "按键%d: 双击第二次按下", i);
                                } else {
                                    // 普通按下或第一次按下
                                    send_key_event(detector, i, KEY_EVENT_PRESS_DOWN);
                                    key_info->click_count = 0;
                                }
                                
                                key_info->long_press_detected = false;
                                ESP_LOGD(TAG, "按键%d: 消抖 -> 按下", i);
                            } else {
                                // 确认释放
                                send_key_event(detector, i, KEY_EVENT_PRESS_UP);
                                
                                // 处理单击/双击事件
                                uint32_t press_duration = time_diff_ms(
                                    key_info->press_timestamp, 
                                    key_info->release_timestamp
                                );
                                
                                if (press_duration <= KEY_CLICK_TIME_MS) {
                                    // 短按，可能是单击或双击的一部分
                                    key_info->click_count++;
                                    
                                    if (key_info->click_count == 1) {
                                        // 第一次单击，进入等待状态
                                        key_info->state = KEY_STATE_WAIT_RELEASE;
                                        key_info->last_event_timestamp = current_time;
                                        ESP_LOGD(TAG, "按键%d: 消抖 -> 等待双击", i);
                                    } else if (key_info->click_count == 2) {
                                        // 第二次单击，发送双击事件
                                        send_key_event(detector, i, KEY_EVENT_DOUBLE_CLICK);
                                        key_info->click_count = 0;
                                        key_info->state = KEY_STATE_RELEASED;
                                        ESP_LOGD(TAG, "按键%d: 双击完成", i);
                                    }
                                } else if (key_info->long_press_detected) {
                                    // 长按后释放
                                    send_key_event(detector, i, KEY_EVENT_LONG_PRESS_END);
                                    key_info->long_press_detected = false;
                                    key_info->click_count = 0;  // 长按后重置单击计数
                                    key_info->state = KEY_STATE_RELEASED;
                                } else {
                                    // 超过短按时长但不是长按
                                    key_info->click_count = 0;  // 重置单击计数
                                    key_info->state = KEY_STATE_RELEASED;
                                    ESP_LOGD(TAG, "按键%d: 释放", i);
                                }
                            }
                        } else {
                            // 状态变化，继续消抖
                            if (pressed) {
                                key_info->press_timestamp = current_time;
                            } else {
                                key_info->release_timestamp = current_time;
                            }
                        }
                    }
                    break;
                }
                
                case KEY_STATE_WAIT_RELEASE: {
                    // 等待释放状态：检测双击或超时
                    uint32_t wait_time = time_diff_ms(key_info->last_event_timestamp, current_time);
                    
                    if (pressed) {
                        // 再次按下，可能是双击
                        key_info->state = KEY_STATE_DEBOUNCE;
                        key_info->press_timestamp = current_time;
                        ESP_LOGD(TAG, "按键%d: 等待 -> 消抖(第二次按下)", i);
                    } else if (wait_time >= KEY_DOUBLE_CLICK_INTERVAL_MS) {
                        // 超时，发送单击事件
                        send_key_event(detector, i, KEY_EVENT_SINGLE_CLICK);
                        key_info->click_count = 0;
                        key_info->state = KEY_STATE_RELEASED;
                        ESP_LOGD(TAG, "按键%d: 等待 -> 释放(单击超时)", i);
                    }
                    break;
                }
                
                default:
                    // 未知状态，重置为释放状态
                    key_info->state = KEY_STATE_RELEASED;
                    key_info->click_count = 0;
                    key_info->long_press_detected = false;
                    break;
            }
        }
        
        // 任务延时，控制扫描频率
        vTaskDelay(pdMS_TO_TICKS(detector->config.scan_interval_ms));
    }
    
    ESP_LOGI(TAG, "按键扫描任务退出");
    vTaskDelete(NULL);
}
/**
 * @brief 创建并初始化按键检测器
 * @param config 检测器配置指针
 * @return key_detector_handle_t 检测器句柄，失败返回NULL
 */
key_detector_handle_t key_detector_create(const key_detector_config_t* config)
{
    // 参数检查
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数不能为NULL");
        return NULL;
    }
    
    if (config->key_count == 0 || config->key_count > MAX_KEY_COUNT) {
        ESP_LOGE(TAG, "按键数量无效: %d", config->key_count);
        return NULL;
    }
    
    if (config->hw_configs == NULL) {
        ESP_LOGE(TAG, "硬件配置不能为NULL");
        return NULL;
    }
    
    // 分配内存
    struct key_detector_t* detector = (struct key_detector_t*)malloc(sizeof(struct key_detector_t));
    if (detector == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return NULL;
    }
    
    // 初始化结构体
    memset(detector, 0, sizeof(struct key_detector_t));
    detector->config = *config;
    detector->task_running = false;
    detector->tick_counter = 0;
    
    // 初始化按键信息
    for (uint8_t i = 0; i < config->key_count; i++) {
        detector->key_infos[i].key_id = i;
        detector->key_infos[i].state = KEY_STATE_RELEASED;
        detector->key_infos[i].last_state = KEY_STATE_RELEASED;
        detector->key_infos[i].press_timestamp = 0;
        detector->key_infos[i].release_timestamp = 0;
        detector->key_infos[i].last_event_timestamp = 0;
        detector->key_infos[i].click_count = 0;
        detector->key_infos[i].long_press_detected = false;
    }
    
    // 初始化GPIO
    esp_err_t err = key_gpio_init(config->hw_configs, config->key_count);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO初始化失败: %s", esp_err_to_name(err));
        free(detector);
        return NULL;
    }
    
    ESP_LOGI(TAG, "按键检测器创建成功，%d个按键", config->key_count);
    return (key_detector_handle_t)detector;
}

/**
 * @brief 启动按键检测任务
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_start(key_detector_handle_t detector)
{
    // 参数检查
    if (detector == NULL) {
        ESP_LOGE(TAG, "检测器句柄不能为NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查是否已在运行
    if (detector->task_running) {
        ESP_LOGW(TAG, "检测任务已在运行");
        return ESP_OK;
    }
    
    // 设置运行标志
    detector->task_running = true;
    
    // 创建扫描任务
    BaseType_t result = xTaskCreate(
        key_scan_task,           // 任务函数
        "key_scan",              // 任务名称
        4096,                    // 堆栈大小
        detector,                // 参数
        tskIDLE_PRIORITY + 2,    // 任务优先级
        &detector->scan_task_handle // 任务句柄
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "创建扫描任务失败");
        detector->task_running = false;
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "按键检测任务启动成功");
    return ESP_OK;
}

/**
 * @brief 停止按键检测任务
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_stop(key_detector_handle_t detector)
{
    // 参数检查
    if (detector == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 检查是否在运行
    if (!detector->task_running) {
        return ESP_OK;
    }
    
    // 停止任务
    detector->task_running = false;
    
    // 等待任务结束
    if (detector->scan_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(50));  // 等待任务退出
    }
    
    ESP_LOGI(TAG, "按键检测任务已停止");
    return ESP_OK;
}

/**
 * @brief 删除按键检测器并释放资源
 * @param detector 按键检测器句柄
 * @return esp_err_t ESP32错误码，ESP_OK表示成功
 */
esp_err_t key_detector_delete(key_detector_handle_t detector)
{
    // 参数检查
    if (detector == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // 先停止任务
    key_detector_stop(detector);
    
    // 释放内存
    free(detector);
    
    ESP_LOGI(TAG, "按键检测器已删除");
    return ESP_OK;
}

/**
 * @brief 从事件队列获取按键事件（非阻塞）
 * @param detector 按键检测器句柄
 * @param event_msg 事件消息结构体指针
 * @param timeout_ms 超时时间（毫秒）
 * @return bool 成功获取返回true，超时或失败返回false
 */
bool key_detector_get_event(key_detector_handle_t detector, 
                           key_event_msg_t* event_msg, 
                           uint32_t timeout_ms)
{
    // 参数检查
    if (detector == NULL || event_msg == NULL) {
        return false;
    }
    
    // 检查是否有事件队列
    if (detector->config.event_queue == NULL) {
        return false;
    }
    
    // 从队列接收事件
    return xQueueReceive(detector->config.event_queue, event_msg, 
                        pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

/**
 * @brief 获取指定按键的当前状态
 * @param detector 按键检测器句柄
 * @param key_id 按键ID
 * @return key_state_t 按键当前状态，无效ID返回KEY_STATE_RELEASED
 */
key_state_t key_detector_get_state(key_detector_handle_t detector, uint8_t key_id)
{
    // 参数检查
    if (detector == NULL || key_id >= detector->config.key_count) {
        return KEY_STATE_RELEASED;
    }
    
    return detector->key_infos[key_id].state;
}