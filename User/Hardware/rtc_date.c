#include "rtc_date.h"

uint16_t MyRTC_Time[] = {2025, 12, 4, 14, 59, 55, 0}; // 定义全局的时间数组，数组内容分别为年、月、日、时、分、秒
myRTC_data RTC_data;
void MyRTC_SetTime(void); // 函数声明
// 先定义星期字符串常量（避免重复创建字符串，推荐做法）
static const char *weekday_str[] = {"   Sunday",
                                    "   Monday",
                                    "  Tuesday",
                                    "Wednesday",
                                    " Thursday",
                                    "   Friday",
                                    " Saturday"};

/**
 * 函    数：RTC初始化
 * 参    数：无
 * 返 回 值：无
 */

void MyRTC_Init(void)
{
    printf("Myrtc init...\n");
    /*开启时钟*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // 开启PWR的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE); // 开启BKP的时钟

    /*备份寄存器访问使能*/
    PWR_BackupAccessCmd(ENABLE); // 使用PWR开启对备份寄存器的访问
    printf("PWR_BackupAccessCmd(ENABLE)\n");
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5) // 通过写入备份寄存器的标志位，判断RTC是否是第一次配置
                                                   // if成立则执行第一次的RTC配置
    {
        RCC_LSEConfig(RCC_LSE_ON);
        printf("wait for LSE\n"); // 开启LSE时钟
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET)
            ; // 等待LSE准备就绪

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE); // 选择RTCCLK来源为LSE
        RCC_RTCCLKCmd(ENABLE);                  // RTCCLK使能

        RTC_WaitForSynchro();  // 等待同步
        RTC_WaitForLastTask(); // 等待上一次操作完成

        RTC_SetPrescaler(32768 - 1); // 设置RTC预分频器，预分频后的计数频率为1Hz
        RTC_WaitForLastTask();       // 等待上一次操作完成

        MyRTC_SetTime(); // 设置时间，调用此函数，全局数组里时间值刷新到RTC硬件电路

        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5); // 在备份寄存器写入自己规定的标志位，用于判断RTC是不是第一次执行配置
    }
    else // RTC不是第一次配置
    {
        printf("--waitforSynchro--\n");
        RTC_WaitForSynchro(); // 等待同步
        printf("RTC_WaitForLastTask\n");
        RTC_WaitForLastTask(); // 等待上一次操作完成
    }
    printf("RTC init OK!\n");
}

// 如果LSE无法起振导致程序卡死在初始化函数中
// 可将初始化函数替换为下述代码，使用LSI当作RTCCLK
// LSI无法由备用电源供电，故主电源掉电时，RTC走时会暂停

// void MyRTC_Init(void)
// {
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);

// 	PWR_BackupAccessCmd(ENABLE);

// 	if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
// 	{
// 		RCC_LSICmd(ENABLE);
// 		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);

// 		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
// 		RCC_RTCCLKCmd(ENABLE);

// 		RTC_WaitForSynchro();
// 		RTC_WaitForLastTask();

// 		RTC_SetPrescaler(40000 - 1);
// 		RTC_WaitForLastTask();

// 		MyRTC_SetTime();

// 		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
// 	}
// 	else
// 	{
// 		RCC_LSICmd(ENABLE);				//即使不是第一次配置，也需要再次开启LSI时钟
// 		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);

// 		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
// 		RCC_RTCCLKCmd(ENABLE);

// 		RTC_WaitForSynchro();
// 		RTC_WaitForLastTask();
// 	}
// }

/**
 * 函    数：RTC设置时间
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，全局数组里时间值将刷新到RTC硬件电路
 */
void MyRTC_SetTime(void)
{
    time_t time_cnt;     // 定义秒计数器数据类型
    struct tm time_date; // 定义日期时间数据类型

    time_date.tm_year = MyRTC_Time[0] - 1900; // 将数组的时间赋值给日期时间结构体
    time_date.tm_mon = MyRTC_Time[1] - 1;
    time_date.tm_mday = MyRTC_Time[2];
    time_date.tm_hour = MyRTC_Time[3];
    time_date.tm_min = MyRTC_Time[4];
    time_date.tm_sec = MyRTC_Time[5];
    time_date.tm_wday = MyRTC_Time[6] - 1;

    time_cnt = mktime(&time_date) - 8 * 60 * 60; // 调用mktime函数，将日期时间转换为秒计数器格式
                                                 //- 8 * 60 * 60为东八区的时区调整

    RTC_SetCounter(time_cnt); // 将秒计数器写入到RTC的CNT中
    RTC_WaitForLastTask();    // 等待上一次操作完成
}

/**
 * 函    数：RTC读取时间
 * 参    数：无
 * 返 回 值：无
 * 说    明：调用此函数后，RTC硬件电路里时间值将刷新到全局数组
 */
void MyRTC_ReadTime(void)
{
    time_t time_cnt;     // 定义秒计数器数据类型
    struct tm time_date; // 定义日期时间数据类型

    time_cnt = RTC_GetCounter() + 8 * 60 * 60; // 读取RTC的CNT，获取当前的秒计数器
                                               //+ 8 * 60 * 60为东八区的时区调整

    time_date = *localtime(&time_cnt); // 使用localtime函数，将秒计数器转换为日期时间格式

    MyRTC_Time[0] = time_date.tm_year + 1900; // 将日期时间结构体赋值给数组的时间
    MyRTC_Time[1] = time_date.tm_mon + 1;
    MyRTC_Time[2] = time_date.tm_mday;
    MyRTC_Time[3] = time_date.tm_hour;
    MyRTC_Time[4] = time_date.tm_min;
    MyRTC_Time[5] = time_date.tm_sec;
    MyRTC_Time[6] = time_date.tm_wday + 1;

    RTC_data.year = time_date.tm_year + 1900; // 将日期时间结构体赋值给数组的时间
    RTC_data.mon = time_date.tm_mon + 1;
    RTC_data.day = time_date.tm_mday;
    RTC_data.hours = time_date.tm_hour;
    RTC_data.minutes = time_date.tm_min;
    RTC_data.seconds = time_date.tm_sec;
    // 复用上面定义的weekday_str数组
    if (time_date.tm_wday >= 0 && time_date.tm_wday <= 6)
    {
        RTC_data.weekday = weekday_str[time_date.tm_wday];
    }
    else
    {
        RTC_data.weekday = "Unknown";
    }
}

// 手动设置RTC时间
void RTC_SetTime_Manual(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    // 参数检查
    if (hours > 23)
        hours = 23;
    if (minutes > 59)
        minutes = 59;
    if (seconds > 59)
        seconds = 59;

    MyRTC_Time[3] = hours;
    MyRTC_Time[4] = minutes;
    MyRTC_Time[5] = seconds;
    MyRTC_SetTime();
}
void RTC_SetDate_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday)
{
    // 参数检查
    if (year > 99)
        year = 99;
    if (month < 1)
        month = 1;
    if (month > 12)
        month = 12;
    if (day < 1)
        day = 1;
    if (day > 31)
        day = 31;
    if (weekday < 1)
        weekday = 1;
    if (weekday > 7)
        weekday = 7;

    MyRTC_Time[0] = year;
    MyRTC_Time[1] = month;
    MyRTC_Time[2] = day;
    MyRTC_Time[6] = weekday;
    MyRTC_SetTime();
}
void RTC_SetDateTime_Manual(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday,
                            uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_SetDate_Manual(year, month, day, weekday);

    // 再设置时间
    RTC_SetTime_Manual(hours, minutes, seconds);

    printf("RTC DateTime set complete!\n");
}
