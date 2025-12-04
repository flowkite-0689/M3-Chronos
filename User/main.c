#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include <FreeRTOS.h>
#include <task.h>
#include "hardware_def.h"
#include "Key.h"
#include "debug.h"
#include "beep.h"
#include "oled_print.h"
#include "rtc_date.h"
#include "dht11.h"

static TaskHandle_t app_task1_handle = NULL;
static TaskHandle_t app_task2_handle = NULL;
/* 任务1 */
static void app_task1(void *pvParameters);
static void app_task2(void *pvParameters);

int main(void)
{
	//系统开始初始化
	TIM2_Delay_Init();
	debug_init();
	OLED_Init();
  OLED_Printf_Line(0,"init");
	OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
	OLED_Refresh();
	LED_Init();
  LED1_ON();
	Key_Init();
	
	Beep_Init();
	
	// RTC_SetTime_Manual(15,22,0);
  





	//系统初始化成功
  printf("init OK\n");
	OLED_Clear();
	OLED_Printf_Line(3,"sys OK...");
	OLED_Refresh();


	/* 创建app_task1任务 */
	xTaskCreate((TaskFunction_t)app_task1,			/* 任务入口函数 */
				(const char *)"app_task1",			/* 任务名字 */
				(uint16_t)512,						/* 任务栈大小 */
				(void *)NULL,						/* 任务入口函数参数 */
				(UBaseType_t)3,						/* 任务的优先级 */
				(TaskHandle_t *)&app_task1_handle); /* 任务控制块指针 */
	/* 创建app_task1任务 */
	xTaskCreate((TaskFunction_t)app_task2,			/* 任务入口函数 */
				(const char *)"app_task2",			/* 任务名字 */
				(uint16_t)512,						/* 任务栈大小 */
				(void *)NULL,						/* 任务入口函数参数 */
				(UBaseType_t)4,						/* 任务的优先级 */
				(TaskHandle_t *)&app_task2_handle); /* 任务控制块指针 */
				
// LED1_OFF();
	/* 开启任务调度 */
	vTaskStartScheduler();
	LED2_ON();
}

static void app_task1(void *pvParameters)
{
  
		DHT11_Data_TypeDef dhtdata;
	  DHT11_Init();
int result = 1;
	for (;;)
	{
		// MyRTC_ReadTime();	
    // OLED_Printf_Line_32(1,"%02d:%02d:%02d",
		// 	RTC_data.hours,RTC_data.minutes,RTC_data.seconds);
    //  printf("%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
		// OLED_Printf_Line(0,"%d.%d.%d %s",RTC_data.year,RTC_data.mon,RTC_data.day,RTC_data.weekday);
 result = Read_DHT11(&dhtdata);
    if (result == 0)
    {
      OLED_Clear_Line(3);
      OLED_Printf_Line(0, "Temperature:%d.%dC ",
                       dhtdata.temp_int, dhtdata.temp_deci);
      OLED_Printf_Line(2, "Humidity:  %d.%d%%",
                       dhtdata.humi_int, dhtdata.humi_deci);
                       // 横向温度计（支持小数：25.5°C → 255）
    

    
    }
		// printf("Temperature:%d.%dC ",
    //                    dhtdata.temp_int, dhtdata.temp_deci);
		OLED_Refresh_Dirty();
// printf("1");
		// LED3=!LED3;
		Delay_ms(1000);
	}	
}
static void app_task2(void *pvParameters)
{
	// MyRTC_Init();
	int KeyNum;
	for (;;)
	{
    
		KeyNum = Key_GetNum();		//获取按键键码
		
		if (KeyNum == 1)			//按键1按下
		{
			BEEP_Buzz(1);
		}
		
		if (KeyNum == 2)			//按键2按下
		{
			BEEP_Buzz(1);
		}

		if (KeyNum == 3)			//按键2按下
		{
			BEEP_Buzz(1);
		}
		if (KeyNum == 4)			//按键2按下
		{
			BEEP_Buzz(1);
			//LED2翻转
		}
		vTaskDelay(60);
	}
}

