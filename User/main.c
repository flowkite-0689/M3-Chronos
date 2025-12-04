#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include <FreeRTOS.h>
#include <task.h>
#include "hardware_def.h"
#include "Key.h"
#include "debug.h"
#include "beep.h"
#include "oled_print.h"

static TaskHandle_t app_task1_handle = NULL;
static TaskHandle_t app_task2_handle = NULL;
/* 任务1 */
static void app_task1(void *pvParameters);
static void app_task2(void *pvParameters);

int main(void)
{
	
	 TIM2_Delay_Init();
	debug_init();
	OLED_Init();

	LED_Init();
  LED1_ON();
	Key_Init();
	
	Beep_Init();
  printf("1111");
	OLED_Printf_Line(0,"hello");
	OLED_Refresh_Dirty();
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
				(UBaseType_t)3,						/* 任务的优先级 */
				(TaskHandle_t *)&app_task2_handle); /* 任务控制块指针 */
				
// LED1_OFF();
	/* 开启任务调度 */
	vTaskStartScheduler();
	LED2_ON();
}

static void app_task1(void *pvParameters)
{
	for (;;)
	{

		// LED3=!LED3;
		vTaskDelay(1000);
	}
}
static void app_task2(void *pvParameters)
{
	int KeyNum;
	for (;;)
	{
    
		KeyNum = Key_GetNum();		//获取按键键码
		
		if (KeyNum == 1)			//按键1按下
		{
			BEEP_Buzz(10);
			printf("key1 被按下\n");
			LED1_Turn();			//LED1翻转
		}
		
		if (KeyNum == 2)			//按键2按下
		{
			BEEP_Buzz(10);
			printf("key2 被按下\n");
			LED2_Turn();			//LED2翻转
		}

		if (KeyNum == 3)			//按键2按下
		{
			BEEP_Buzz(10);
			printf("key3 被按下\n");
						//LED2翻转
		}
		if (KeyNum == 4)			//按键2按下
		{
			BEEP_Buzz(10);
			printf("key4 被按下\n");
			LED2_Turn();			//LED2翻转
		}
		
	}
}

void vApplicationIdleHook( void )
{
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
void vApplicationTickHook( void )
{
   
}
void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
