# components/key_deteced/CMakeLists.txt

# 注册按键检测组件
idf_component_register(
    SRCS 
        "key_config.c"
        "key_detector.c"
        "key_driver.c"
    INCLUDE_DIRS 
        "."  # 当前目录作为公共头文件目录
    REQUIRES 
        driver  
        esp_timer
        # 公共依赖（其他组件也可以使用）
        # 私有依赖（仅本组件内部使用）
        # 如果需要 GPIO 功能可以添加：driver
)

