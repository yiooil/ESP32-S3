#ifndef __KEY_DRIVER_H__
#define __KEY_DRIVER_H__
#include "driver/gpio.h"        // GPIO驱动头文件


void key_event_task(void* arg);
void key_init(void);
#endif /* __KEY_DRIVER_H__ */
