#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "key_driver.h"
#include "esp_log.h"
void app_main(void)
{
    key_init();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
