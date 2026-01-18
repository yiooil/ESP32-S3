/**
 * @file key_config.c
 * @brief 按键硬件配置实现
 * @details 实现GPIO初始化和读取函数，处理不同触发电平的按键
 */

#include "key_config.h"
#include "esp_log.h"          // ESP32日志系统


// 日志标签定义
static const char* TAG = "key_config";

// GPIO中断禁用宏（轮询模式不需要中断）
#define GPIO_INTR_DISABLE 0

// 全局变量：存储按键硬件配置
static const key_hw_config_t* s_hw_configs = NULL;
// 全局变量：存储按键数量
static uint8_t s_key_count = 0;



/**
 * @brief 初始化所有按键的GPIO硬件
 * @param config 按键硬件配置数组指针
 * @param count 按键数量
 * @return esp_err_t ESP32错误码，ESP_OK表示成功，其他值表示失败
 */
esp_err_t key_gpio_init(const key_hw_config_t* config, uint8_t count)
{
    // 参数有效性检查
    if (config == NULL) {
        ESP_LOGE(TAG, "配置参数不能为NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (count == 0 || count > MAX_KEY_COUNT) {
        ESP_LOGE(TAG, "按键数量无效: %d (最大支持%d)", count, MAX_KEY_COUNT);
        return ESP_ERR_INVALID_ARG;
    }
    
    // 保存配置到全局变量
    s_hw_configs = config;
    s_key_count = count;
    
    // GPIO配置结构体
    gpio_config_t io_conf = {0};
    
    // 配置所有按键GPIO为输入模式
    for (uint8_t i = 0; i < count; i++) {
        const key_hw_config_t* hw_cfg = &config[i];
        
        // 检查GPIO号是否有效
        if (hw_cfg->gpio_num > GPIO_NUM_MAX) {
            ESP_LOGE(TAG, "按键%d的GPIO号无效: %d", i, hw_cfg->gpio_num);
            return ESP_ERR_INVALID_ARG;
        }
        
        // 设置GPIO为输入模式
        io_conf.intr_type = GPIO_INTR_DISABLE;          // 禁用中断（轮询模式）
        io_conf.mode = GPIO_MODE_INPUT;                 // 输入模式
        io_conf.pin_bit_mask = (1ULL << hw_cfg->gpio_num); // 引脚位掩码
        
        // 配置上拉/下拉电阻
        if (hw_cfg->pull_enable) {
            if (hw_cfg->pull_up) {
                io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // 启用上拉
                io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉
            } else {
                io_conf.pull_up_en = GPIO_PULLUP_DISABLE;   // 禁用上拉
                io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // 启用下拉
            }
        } else {
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;       // 禁用上拉
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;  // 禁用下拉
        }
        
        // 应用GPIO配置
        esp_err_t err = gpio_config(&io_conf);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "配置GPIO %d失败: %s", hw_cfg->gpio_num, esp_err_to_name(err));
            return err;
        }
        
        ESP_LOGI(TAG, "按键%d初始化: GPIO=%d, 触发电平=%s, 上拉=%s", 
                i, hw_cfg->gpio_num,
                hw_cfg->active_level == KEY_ACTIVE_LOW ? "低电平" : "高电平",
                hw_cfg->pull_enable ? (hw_cfg->pull_up ? "上拉" : "下拉") : "禁用");
    }
    
    ESP_LOGI(TAG, "按键GPIO初始化完成，共%d个按键", count);
    return ESP_OK;
}



/**
 * @brief 读取指定GPIO的当前电平状态
 * @param gpio_num GPIO引脚编号
 * @return uint8_t 返回当前电平（0=低电平，1=高电平）
 */
uint8_t key_gpio_read(uint8_t gpio_num)
{
    // 读取GPIO电平（返回0或1）
    return gpio_get_level(gpio_num);
}