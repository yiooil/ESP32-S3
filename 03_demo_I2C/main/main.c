#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "key_driver.h"
#include "esp_log.h"
#include "I2C_Driver.h"
#include "PCF85063.h"

/**
 * @brief RTC示例任务函数
 * 
 * 这个任务演示如何使用PCF85063 RTC驱动。
 * 包括初始化、设置时间、读取时间、设置闹钟等操作。
 * 
 * @param pvParameters 任务参数（未使用）
 */
// 示例1：使用I2C0连接PCF85063（默认配置）
void example_rtc_i2c0(void* arg)
{
    // 步骤1：初始化I2C0总线
    I2C0_Init();
    
    // 步骤2：初始化PCF85063 RTC
    PCF85063_Init();
    
    // 步骤3：设置初始时间（可选）
    datetime_t init_time = {
        .year = 2024,
        .month = 10,
        .day = 15,
        .dotw = 2,  // 星期二
        .hour = 14,
        .minute = 30,
        .second = 0
    };
    PCF85063_Set_All(init_time);
    
    // 步骤4：主循环中更新时间
    while(1) {
        PCF85063_Loop();  // 更新全局变量datetime
        
        // 使用datetime变量
        char time_str[50];
        datetime_to_str(time_str, datetime);
        ESP_LOGI("RTC", "当前时间: %s", time_str);
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // 延时1秒
    }
}


void app_main(void)
{


    
    key_init();
    xTaskCreate(example_rtc_i2c0, "example_rtc_i2c0", 4096, NULL, 5, NULL);
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
