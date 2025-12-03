#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include <FreeRTOS.h>
#include <task.h>
#include "OLED.h"
static TaskHandle_t app_task1_handle = NULL;
static TaskHandle_t app_task2_handle = NULL;
/* 任务1 */
static void app_task1(void *pvParameters);
static void app_task2(void *pvParameters);

int main(void)
{
  OLED_Init();	
	LED_Init();
  LED1_ON();
	OLED_ShowString(1, 1, "RxData:");
	
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

		LED1_Turn();
		vTaskDelay(1000);
	}
}
static void app_task2(void *pvParameters)
{
	for (;;)
	{
    
		LED2_Turn();
		vTaskDelay(1000);
	}
}

