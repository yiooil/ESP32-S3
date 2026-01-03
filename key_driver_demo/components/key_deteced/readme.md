系统使用说明
1. 配置按键
在main.c中修改key_hw_configs数组来配置按键：

gpio_num: ESP32-S3的GPIO引脚号

active_level: 触发电平（KEY_ACTIVE_LOW或KEY_ACTIVE_HIGH）

pull_enable: 是否启用内部上拉/下拉

pull_up: 启用上拉(1)还是下拉(0)

key_name: 按键名称（用于调试）

2. 事件处理方式
系统支持两种事件处理方式：

回调函数: 在key_event_callback中处理实时事件

消息队列: 在key_event_task中通过队列获取事件

3. 按键事件类型
KEY_EVENT_PRESS_DOWN: 按键按下

KEY_EVENT_PRESS_UP: 按键释放

KEY_EVENT_SINGLE_CLICK: 单击

KEY_EVENT_DOUBLE_CLICK: 双击

KEY_EVENT_LONG_PRESS_START: 长按开始

KEY_EVENT_LONG_PRESS_HOLD: 长按保持（周期性触发）

KEY_EVENT_LONG_PRESS_END: 长按结束

4. 时间参数配置
在key_common.h中可以调整时间参数：

KEY_DEBOUNCE_TIME_MS: 消抖时间（默认20ms）

KEY_CLICK_TIME_MS: 单击最大时间（默认200ms）

KEY_DOUBLE_CLICK_INTERVAL_MS: 双击间隔（默认350ms）

KEY_LONG_PRESS_TIME_MS: 长按开始时间（默认800ms）

KEY_LONG_PRESS_HOLD_INTERVAL_MS: 长按保持触发间隔（默认200ms）

5. 编译和运行
将上述文件添加到ESP-IDF项目中

根据需要修改按键配置

编译并烧录到ESP32-S3

查看串口输出日志，观察按键事件

这个系统提供了完整的按键检测功能，支持不同触发电平的按键，使用状态机实现可靠的消抖和事件检测，并通过FreeRTOS消息队列实现任务间通信。