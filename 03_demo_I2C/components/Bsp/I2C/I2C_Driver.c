#include "I2C_Driver.h"  // 包含自定义头文件

// 定义I2C模块的日志标签
static const char *I2C_TAG = "I2C";

/********************* I2C0 功能实现 *********************/

/**
 * @brief I2C0 主机初始化函数（内部使用）
 * @return esp_err_t ESP_OK表示初始化成功，其他值为错误码
 * @note 此函数配置I2C0为主机模式，设置引脚、时钟和上拉电阻
 */
static esp_err_t i2c0_master_init(void)
{
    // 定义I2C配置结构体
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,           // 主机模式
        .sda_io_num = I2C0_SDA_IO,         // SDA引脚号
        .scl_io_num = I2C0_SCL_IO,         // SCL引脚号
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // 启用SDA上拉电阻
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // 启用SCL上拉电阻
        .master.clk_speed = I2C0_MASTER_FREQ_HZ,  // 时钟频率
        .clk_flags = 0,                    // 时钟标志位，通常为0
    };

    // 设置I2C0参数配置
    esp_err_t err = i2c_param_config(I2C0_MASTER_NUM, &conf);
    if (err != ESP_OK) {  // 检查配置是否成功
        // 记录错误日志，包含错误码描述
        ESP_LOGE(I2C_TAG, "I2C0参数配置失败: %s", esp_err_to_name(err));
        return err;  // 返回错误码
    }

    // 安装I2C0驱动程序
    err = i2c_driver_install(I2C0_MASTER_NUM, conf.mode, 
                             I2C0_MASTER_RX_BUF_DISABLE,  // 接收缓冲区大小
                             I2C0_MASTER_TX_BUF_DISABLE,  // 发送缓冲区大小
                             0);  // 中断分配标志
    if (err != ESP_OK) {  // 检查驱动安装是否成功
        // 记录错误日志
        ESP_LOGE(I2C_TAG, "I2C0驱动安装失败: %s", esp_err_to_name(err));
        return err;  // 返回错误码
    }

    // 记录成功日志，显示使用的引脚
    ESP_LOGI(I2C_TAG, "I2C0初始化成功，SDA引脚:%d, SCL引脚:%d", 
             I2C0_SDA_IO, I2C0_SCL_IO);
    return ESP_OK;  // 返回成功
}

/**
 * @brief I2C0 初始化接口函数
 * @note 包装内部初始化函数，使用ESP_ERROR_CHECK进行错误检查
 */
void I2C0_Init(void)
{
    // 调用I2C0初始化函数，如果失败会触发断言（在调试模式下）
    ESP_ERROR_CHECK(i2c0_master_init());
}

/**
 * @brief 通过I2C0总线写入数据到设备
 * @param Driver_addr 从设备地址（7位）
 * @param Reg_addr 寄存器地址
 * @param Reg_data 要写入的数据缓冲区
 * @param Length 要写入的数据长度
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t I2C0_Write(uint8_t Driver_addr, uint8_t Reg_addr, 
                     const uint8_t *Reg_data, uint32_t Length)
{
    // 检查数据长度是否为0
    if (Length == 0) {
        ESP_LOGE(I2C_TAG, "I2C0写入数据长度为0");
        return ESP_ERR_INVALID_ARG;  // 返回无效参数错误
    }
    
    // 创建发送缓冲区：寄存器地址 + 数据
    uint8_t buf[Length + 1];
    buf[0] = Reg_addr;  // 第一个字节为寄存器地址
    
    // 将数据复制到缓冲区（从buf[1]开始）
    memcpy(&buf[1], Reg_data, Length);
    
    // 调用ESP-IDF的I2C写入函数
    return i2c_master_write_to_device(I2C0_MASTER_NUM,  // I2C端口号
                                      Driver_addr,      // 设备地址
                                      buf,              // 发送缓冲区
                                      Length + 1,       // 发送数据长度
                                      I2C0_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);  // 超时时间
}

/**
 * @brief 通过I2C0总线从设备读取数据
 * @param Driver_addr 从设备地址（7位）
 * @param Reg_addr 寄存器地址
 * @param Reg_data 读取数据存储缓冲区
 * @param Length 要读取的数据长度
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t I2C0_Read(uint8_t Driver_addr, uint8_t Reg_addr, 
                    uint8_t *Reg_data, uint32_t Length)
{
    // 检查数据长度是否为0
    if (Length == 0) {
        ESP_LOGE(I2C_TAG, "I2C0读取数据长度为0");
        return ESP_ERR_INVALID_ARG;  // 返回无效参数错误
    }
    
    // 调用ESP-IDF的I2C写读函数：先写寄存器地址，再读取数据
    return i2c_master_write_read_device(I2C0_MASTER_NUM,  // I2C端口号
                                        Driver_addr,      // 设备地址
                                        &Reg_addr,        // 写入的数据（寄存器地址）
                                        1,                // 写入数据长度
                                        Reg_data,         // 读取数据存储缓冲区
                                        Length,           // 读取数据长度
                                        I2C0_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);  // 超时时间
}

/********************* I2C1 功能实现 *********************/

/**
 * @brief I2C1 主机初始化函数（内部使用）
 * @return esp_err_t ESP_OK表示初始化成功
 * @note 此函数配置I2C1为主机模式，与I2C0类似但使用不同的引脚
 */
static esp_err_t i2c1_master_init(void)
{
    // 定义I2C配置结构体
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,           // 主机模式
        .sda_io_num = I2C1_SDA_IO,         // SDA引脚号
        .scl_io_num = I2C1_SCL_IO,         // SCL引脚号
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // 启用SDA上拉电阻
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // 启用SCL上拉电阻
        .master.clk_speed = I2C1_MASTER_FREQ_HZ,  // 时钟频率
        .clk_flags = 0,                    // 时钟标志位
    };

    // 设置I2C1参数配置
    esp_err_t err = i2c_param_config(I2C1_MASTER_NUM, &conf);
    if (err != ESP_OK) {  // 检查配置是否成功
        ESP_LOGE(I2C_TAG, "I2C1参数配置失败: %s", esp_err_to_name(err));
        return err;
    }

    // 安装I2C1驱动程序
    err = i2c_driver_install(I2C1_MASTER_NUM, conf.mode, 
                             I2C1_MASTER_RX_BUF_DISABLE,  // 接收缓冲区大小
                             I2C1_MASTER_TX_BUF_DISABLE,  // 发送缓冲区大小
                             0);  // 中断分配标志
    if (err != ESP_OK) {  // 检查驱动安装是否成功
        ESP_LOGE(I2C_TAG, "I2C1驱动安装失败: %s", esp_err_to_name(err));
        return err;
    }

    // 记录成功日志
    ESP_LOGI(I2C_TAG, "I2C1初始化成功，SDA引脚:%d, SCL引脚:%d", 
             I2C1_SDA_IO, I2C1_SCL_IO);
    return ESP_OK;
}

/**
 * @brief I2C1 初始化接口函数
 */
void I2C1_Init(void)
{
    // 调用I2C1初始化函数
    ESP_ERROR_CHECK(i2c1_master_init());
}

/**
 * @brief 通过I2C1总线写入数据到设备
 * @param Driver_addr 从设备地址（7位）
 * @param Reg_addr 寄存器地址
 * @param Reg_data 要写入的数据缓冲区
 * @param Length 要写入的数据长度
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t I2C1_Write(uint8_t Driver_addr, uint8_t Reg_addr, 
                     const uint8_t *Reg_data, uint32_t Length)
{
    // 检查数据长度是否为0
    if (Length == 0) {
        ESP_LOGE(I2C_TAG, "I2C1写入数据长度为0");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 创建发送缓冲区：寄存器地址 + 数据
    uint8_t buf[Length + 1];
    buf[0] = Reg_addr;  // 第一个字节为寄存器地址
    
    // 将数据复制到缓冲区
    memcpy(&buf[1], Reg_data, Length);
    
    // 调用ESP-IDF的I2C写入函数
    return i2c_master_write_to_device(I2C1_MASTER_NUM,  // I2C端口号
                                      Driver_addr,      // 设备地址
                                      buf,              // 发送缓冲区
                                      Length + 1,       // 发送数据长度
                                      I2C1_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);  // 超时时间
}

/**
 * @brief 通过I2C1总线从设备读取数据
 * @param Driver_addr 从设备地址（7位）
 * @param Reg_addr 寄存器地址
 * @param Reg_data 读取数据存储缓冲区
 * @param Length 要读取的数据长度
 * @return esp_err_t ESP_OK表示成功
 */
esp_err_t I2C1_Read(uint8_t Driver_addr, uint8_t Reg_addr, 
                    uint8_t *Reg_data, uint32_t Length)
{
    // 检查数据长度是否为0
    if (Length == 0) {
        ESP_LOGE(I2C_TAG, "I2C1读取数据长度为0");
        return ESP_ERR_INVALID_ARG;
    }
    
    // 调用ESP-IDF的I2C写读函数
    return i2c_master_write_read_device(I2C1_MASTER_NUM,  // I2C端口号
                                        Driver_addr,      // 设备地址
                                        &Reg_addr,        // 写入的数据（寄存器地址）
                                        1,                // 写入数据长度
                                        Reg_data,         // 读取数据存储缓冲区
                                        Length,           // 读取数据长度
                                        I2C1_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);  // 超时时间
}
