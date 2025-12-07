#include "stm32f10x.h" // Device header
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
#include "queue.h"
#include "unified_menu.h"
#include "index.h"
#include "MPU6050_hardware_i2c.h"


// 创建队列来存储按键事件
QueueHandle_t keyQueue;     // 按键队列



static TaskHandle_t Menu_handle = NULL;
static TaskHandle_t Key_handle = NULL;

/* 任务函数声明 */
static void Menu_Main_Task(void *pvParameters);
static void Key_Main_Task(void *pvParameters);

int main(void)
{
    // 系统初始化开始
    TIM2_Delay_Init();
    debug_init();
    OLED_Init();
    OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
    OLED_Refresh();
    Key_Init();
    Beep_Init();
    // RTC_SetTime_Manual(15,22,0);

    	MPU_Init();

	// ??MPU6050??ID
	u8 device_id;
	MPU_Read_Byte(MPU_ADDR, MPU_DEVICE_ID_REG, &device_id);
	printf("MPU6050 Device ID: 0x%02X (Expected: 0x68)\r\n", device_id);

	if (device_id != MPU_ADDR)
	{
		printf("MPU6050 Device ID Mismatch!\r\n");
		OLED_Printf_Line(1, "MPU ID Error!");
		OLED_Refresh();
		Delay_ms(2000);
	}
	else
	{
		printf("MPU6050 Device ID OK\r\n");
	}
    // 系统初始化成功
    printf("wait for sys OK...\n");
    OLED_Clear();
    OLED_Printf_Line(3, "wait for sys OK...");
    OLED_Refresh();

    // 初始化菜单系统
    if (menu_system_init() != 0) {
        printf("Menu system initialization failed\r\n");
        return -1;
    }
    
    // 创建并初始化首页
    menu_item_t* index_menu = index_init();
    if (index_menu == NULL) {
        printf("Index page initialization failed\r\n");
        return -1;
    }
    
    // 设置首页为根菜单
    g_menu_sys.root_menu = index_menu;
    g_menu_sys.current_menu = index_menu;

    printf("root_menu init OK\n");
    /* 创建菜单任务 */
    xTaskCreate((TaskFunction_t)Menu_Main_Task,          /* 任务函数 */
                (const char *)"Menu_Main",               /* 任务名称 */
                (uint16_t)2024,                          /* 任务堆栈大小 */
                (void *)NULL,                           /* 任务函数参数 */
                (UBaseType_t)3,                         /* 任务优先级 */
                (TaskHandle_t *)&Menu_handle);           /* 任务控制句柄 */
    xTaskCreate(Key_Main_Task, "KeyMain", 128, NULL, 4, &Key_handle);
    
    printf("creat task OK\n");
    
    // 添加调试信息，确认调度器启动
    printf("Starting scheduler...\n");
    
    /* 启动调度器 */
    vTaskStartScheduler();
    
    // 如果调度器启动失败，会执行到这里
    printf("ERROR: Scheduler failed to start!\n");
    LED2_ON();
}

static void Menu_Main_Task(void *pvParameters)
{
    printf("Menu_Main_Task start ->\n");
    // 直接调用统一菜单框架的任务
    menu_task(pvParameters);
}

static void Key_Main_Task(void *pvParameters)
{
    // 直接调用统一菜单框架的按键任务
    menu_key_task(pvParameters);
}
