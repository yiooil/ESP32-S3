#include "PCF85063.h" // 包含PCF85063驱动头文件

/********************* 全局变量定义 *********************/
datetime_t datetime = {0}; /*!< 全局日期时间变量，初始化为0 */

/********************* 静态函数声明 *********************/
static uint8_t decToBcd(int val); /*!< 十进制转BCD码 */
static int bcdToDec(uint8_t val); /*!< BCD码转十进制 */

/********************* 常量定义 *********************/
const unsigned char MonthStr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
									   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; /*!< 月份缩写字符串数组 */

/********************* 函数实现 *********************/

/**
 * @brief PCF85063初始化函数
 * @note 配置RTC为正常模式，时钟运行，24小时制，内部负载电容12.5pF
 */
void PCF85063_Init()
{
	// 设置控制寄存器1：默认值+选择12.5pF内部负载电容
	uint8_t Value = RTC_CTRL_1_DEFAULT | RTC_CTRL_1_CAP_SEL;

	// 使用I2C驱动写入配置到控制寄存器1
	// 注意：这里使用默认的I2C_WRITE函数，对应I2C0总线
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_CTRL_1_ADDR, &Value, 1));

	// 如果需要设置初始时间，可以取消注释以下代码并设置相应时间
	/*
	datetime_t Now_datetime = {0};
	Now_datetime.year = 2024;      // 设置年份
	Now_datetime.month = 9;        // 设置月份
	Now_datetime.day = 20;         // 设置日期
	Now_datetime.dotw = 5;         // 设置星期（5=星期五）
	Now_datetime.hour = 9;         // 设置小时
	Now_datetime.minute = 50;      // 设置分钟
	Now_datetime.second = 0;       // 设置秒
	PCF85063_Set_All(Now_datetime); // 设置完整时间
	*/
}

/**
 * @brief PCF85063主循环函数
 * @note 定期读取和更新当前时间到全局变量datetime
 */
void PCF85063_Loop(void)
{
	// 读取当前时间到全局变量datetime
	PCF85063_Read_Time(&datetime);
}

/**
 * @brief PCF85063软件复位函数
 * @note 触发软件复位，重置RTC内部状态
 */
void PCF85063_Reset()
{
	// 设置控制寄存器1：默认值+选择12.5pF内部负载电容+软件复位位
	uint8_t Value = RTC_CTRL_1_DEFAULT | RTC_CTRL_1_CAP_SEL | RTC_CTRL_1_SR;

	// 写入配置到控制寄存器1，触发软件复位
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_CTRL_1_ADDR, &Value, 1));
}

/**
 * @brief 设置时间（时、分、秒）
 * @param time 包含时间信息的结构体
 */
void PCF85063_Set_Time(datetime_t time)
{
	// 创建时间数据缓冲区：秒、分、时（转换为BCD码）
	uint8_t buf[3] = {decToBcd(time.second), // 秒转换为BCD码
					  decToBcd(time.minute), // 分转换为BCD码
					  decToBcd(time.hour)};	 // 时转换为BCD码

	// 通过I2C写入时间数据到RTC的秒、分、时寄存器
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_SECOND_ADDR, buf, 3));
}

/**
 * @brief 设置日期（年、月、日、星期）
 * @param date 包含日期信息的结构体
 */
void PCF85063_Set_Date(datetime_t date)
{
	// 创建日期数据缓冲区：日、星期、月、年（转换为BCD码）
	uint8_t buf[4] = {decToBcd(date.day),				  // 日转换为BCD码
					  decToBcd(date.dotw),				  // 星期转换为BCD码
					  decToBcd(date.month),				  // 月转换为BCD码
					  decToBcd(date.year - YEAR_OFFSET)}; // 年转换为BCD码（减去偏移量1970）

	// 通过I2C写入日期数据到RTC的日、星期、月、年寄存器
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_DAY_ADDR, buf, 4));
}

/**
 * @brief 设置完整的日期和时间
 * @param time 包含完整日期时间信息的结构体
 */
void PCF85063_Set_All(datetime_t time)
{
	// 创建完整日期时间数据缓冲区：秒、分、时、日、星期、月、年（全部转换为BCD码）
	uint8_t buf[7] = {decToBcd(time.second),			  // 秒
					  decToBcd(time.minute),			  // 分
					  decToBcd(time.hour),				  // 时
					  decToBcd(time.day),				  // 日
					  decToBcd(time.dotw),				  // 星期
					  decToBcd(time.month),				  // 月
					  decToBcd(time.year - YEAR_OFFSET)}; // 年（减去偏移量1970）

	// 通过I2C写入完整日期时间数据到RTC的秒寄存器开始的连续7个寄存器
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_SECOND_ADDR, buf, 7));
}

/**
 * @brief 读取当前日期和时间
 * @param time 用于存储读取结果的日期时间结构体指针
 */
void PCF85063_Read_Time(datetime_t *time)
{
	uint8_t buf[7] = {0}; // 创建缓冲区用于存储读取的数据

	// 通过I2C从RTC的秒寄存器开始连续读取7个寄存器数据
	ESP_ERROR_CHECK(I2C_READ(PCF85063_ADDRESS, RTC_SECOND_ADDR, buf, 7));

	// 将读取的BCD码转换为十进制，并解析到日期时间结构体
	time->second = bcdToDec(buf[0] & 0x7F);		 // 秒（屏蔽最高位）
	time->minute = bcdToDec(buf[1] & 0x7F);		 // 分（屏蔽最高位）
	time->hour = bcdToDec(buf[2] & 0x3F);		 // 时（屏蔽高2位，12/24小时制标志）
	time->day = bcdToDec(buf[3] & 0x3F);		 // 日（屏蔽高2位）
	time->dotw = bcdToDec(buf[4] & 0x07);		 // 星期（保留低3位）
	time->month = bcdToDec(buf[5] & 0x1F);		 // 月（保留低5位）
	time->year = bcdToDec(buf[6]) + YEAR_OFFSET; // 年（转换为完整年份：1970+寄存器值）
}

/**
 * @brief 使能闹钟功能并清除闹钟标志
 */
void PCF85063_Enable_Alarm()
{
	// 设置控制寄存器2：默认值+使能闹钟中断
	uint8_t Value = RTC_CTRL_2_DEFAULT | RTC_CTRL_2_AIE;

	// 清除闹钟标志位（AF位清零）
	Value &= ~RTC_CTRL_2_AF;

	// 写入配置到控制寄存器2
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_CTRL_2_ADDR, &Value, 1));
}

/**
 * @brief 获取闹钟标志位状态
 * @return uint8_t 闹钟标志位状态
 */
uint8_t PCF85063_Get_Alarm_Flag()
{
	uint8_t Value = 0; // 用于存储读取的控制寄存器2值

	// 读取控制寄存器2的值
	ESP_ERROR_CHECK(I2C_READ(PCF85063_ADDRESS, RTC_CTRL_2_ADDR, &Value, 1));

	// 提取闹钟相关的标志位（闹钟标志AF和闹钟中断使能AIE）
	Value &= RTC_CTRL_2_AF | RTC_CTRL_2_AIE;

	return Value; // 返回闹钟相关标志位
}

/**
 * @brief 设置闹钟时间
 * @param time 包含闹钟时间的结构体
 * @note 当前实现只设置时、分、秒闹钟，日和星期闹钟被禁用
 */
void PCF85063_Set_Alarm(datetime_t time)
{
	// 创建闹钟数据缓冲区：秒、分、时、日、星期
	// 注意：日和星期闹钟被禁用（设置为RTC_ALARM）
	uint8_t buf[5] = {
		decToBcd(time.second) & (~RTC_ALARM), // 闹钟秒（清除闹钟禁用位）
		decToBcd(time.minute) & (~RTC_ALARM), // 闹钟分（清除闹钟禁用位）
		decToBcd(time.hour) & (~RTC_ALARM),	  // 闹钟时（清除闹钟禁用位）
		RTC_ALARM,							  // 禁用日闹钟（最高位置1）
		RTC_ALARM							  // 禁用星期闹钟（最高位置1）
	};

	// 通过I2C写入闹钟数据到RTC的闹钟寄存器
	// 注意：函数调用参数是6，但数组只有5个元素，这里需要修正为5
	ESP_ERROR_CHECK(I2C_WRITE(PCF85063_ADDRESS, RTC_SECOND_ALARM, buf, 5));
}

/**
 * @brief 读取当前设置的闹钟时间
 * @param time 用于存储读取结果的日期时间结构体指针
 */
void PCF85063_Read_Alarm(datetime_t *time)
{
	uint8_t bufss[6] = {0}; // 创建缓冲区用于存储读取的闹钟数据

	// 从RTC的闹钟秒寄存器开始连续读取6个闹钟寄存器数据
	ESP_ERROR_CHECK(I2C_READ(PCF85063_ADDRESS, RTC_SECOND_ALARM, bufss, 6));

	// 将读取的BCD码转换为十进制，并解析到日期时间结构体
	time->second = bcdToDec(bufss[0] & 0x7F); // 闹钟秒
	time->minute = bcdToDec(bufss[1] & 0x7F); // 闹钟分
	time->hour = bcdToDec(bufss[2] & 0x3F);	  // 闹钟时
	time->day = bcdToDec(bufss[3] & 0x3F);	  // 闹钟日
	time->dotw = bcdToDec(bufss[4] & 0x07);	  // 闹钟星期
}

/**
 * @brief 将十进制数转换为BCD码
 * @param val 十进制数值（0-99）
 * @return uint8_t 对应的BCD码
 * @note BCD码：十进制数12 -> BCD码0x12（0001 0010）
 */
static uint8_t decToBcd(int val)
{
	// 转换公式：十位部分×16 + 个位部分
	return (uint8_t)((val / 10 * 16) + (val % 10));
}

/**
 * @brief 将BCD码转换为十进制数
 * @param val BCD码值
 * @return int 对应的十进制数值
 * @note BCD码0x12（0001 0010）-> 十进制数12
 */
static int bcdToDec(uint8_t val)
{
	// 转换公式：高4位×10 + 低4位
	return (int)((val / 16 * 10) + (val % 16));
}

/**
 * @brief 将日期时间结构体转换为格式化字符串
 * @param datetime_str 用于存储转换结果的字符串缓冲区
 * @param time 要转换的日期时间结构体
 * @note 格式：YYYY.MM.DD WW HH:MM:SS
 */
void datetime_to_str(char *datetime_str, datetime_t time)
{
	// 使用sprintf格式化日期时间为字符串
	sprintf(datetime_str, " %d.%d.%d  %d %d:%d:%d ",
			time.year,	  // 年份
			time.month,	  // 月份
			time.day,	  // 日期
			time.dotw,	  // 星期（数字）
			time.hour,	  // 小时
			time.minute,  // 分钟
			time.second); // 秒

}