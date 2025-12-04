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
#include "queue.h"
#include <string.h>

//创建队列来存储按键事件
QueueHandle_t keyQueue;      // 按键队列
QueueHandle_t displayQueue;  // 显示队列

// 显示命令类型定义
typedef enum {
    DISPLAY_CMD_CLEAR,           // 清屏
    DISPLAY_CMD_CLEAR_LINE,      // 清除指定行
    DISPLAY_CMD_PRINT_LINE,      // 行打印
    DISPLAY_CMD_PRINT,           // 指定位置打印
    DISPLAY_CMD_PRINT_32,        // 32px字体行打印
    DISPLAY_CMD_REFRESH,         // 刷新显示
    DISPLAY_CMD_PICTURE          // 显示图片
} display_cmd_type_t;

// 显示消息结构体
typedef struct {
    display_cmd_type_t cmd;      // 命令类型
    uint8_t x;                   // X坐标或行号
    uint8_t y;                   // Y坐标
    char text[32];               // 要显示的文本
    uint8_t param1;              // 扩展参数1（图片宽度等）
    uint8_t param2;              // 扩展参数2（图片高度等）
    const unsigned char *picture_data; // 图片数据
} display_msg_t;


static TaskHandle_t Page_Logic_handle = NULL;
static TaskHandle_t Key_scan_handle = NULL;
static TaskHandle_t OLED_Display_handle = NULL;
/* 任务1 */
static void Page_Logic(void *pvParameters);
static void Key_scan(void *pvParameters);
static void OLED_Display_Task(void *pvParameters);

// 显示辅助函数
void display_clear(void)
{
    display_msg_t msg = {.cmd = DISPLAY_CMD_CLEAR};
    xQueueSend(displayQueue, &msg, 0);
}

void display_clear_line(uint8_t line)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_CLEAR_LINE,
        .x = line
    };
    xQueueSend(displayQueue, &msg, 0);
}

void display_print_line(uint8_t line, const char* text)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT_LINE,
        .x = line
    };
    strncpy(msg.text, text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';
    xQueueSend(displayQueue, &msg, 0);
}

void display_print(uint8_t x, uint8_t y, const char* text)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT,
        .x = x,
        .y = y
    };
    strncpy(msg.text, text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';
    xQueueSend(displayQueue, &msg, 0);
}

void display_refresh(void)
{
    display_msg_t msg = {.cmd = DISPLAY_CMD_REFRESH};
    xQueueSend(displayQueue, &msg, 0);
}

int main(void)
{
	//系统开始初始化
	TIM2_Delay_Init();
	debug_init();
	OLED_Init();
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


	keyQueue = xQueueCreate(10,sizeof(int));
	displayQueue = xQueueCreate(20,sizeof(display_msg_t));

	if (keyQueue == NULL) {
          printf("Failed to create key queue\r\n");
          return -1;
      }
	if (displayQueue == NULL) {
          printf("Failed to create display queue\r\n");
          return -1;
      }

	/* 创建app_task1任务 */
	xTaskCreate((TaskFunction_t)Key_scan,			/* 任务入口函数 */
				(const char *)"Key_scan",			/* 任务名字 */
				(uint16_t)512,						/* 任务栈大小 */
				(void *)NULL,						/* 任务入口函数参数 */
				(UBaseType_t)4,						/* 任务的优先级 */
				(TaskHandle_t *)&Key_scan_handle); /* 任务控制块指针 */
	xTaskCreate(Page_Logic, "PageLogic", 256, NULL, 2, &Page_Logic_handle);
	xTaskCreate(OLED_Display_Task, "OLED_Display", 256, NULL, 3, &OLED_Display_handle);	
// LED1_OFF();
	/* 开启任务调度 */
	vTaskStartScheduler();
	LED2_ON();
}

  static void Page_Logic(void *pvParameters)
  {
      int received_key;

      for (;;)
      {
				vTaskDelay(60);
          // 等待按键事件
          if (xQueueReceive(keyQueue, &received_key, portMAX_DELAY)
  == pdPASS) {
              // 处理按键事件
              switch (received_key) {
                  case 1:
                      // 清屏并显示按键1信息
                     
                      display_print_line(1, "key 1 p");
                      break;
                  case 2:
                      // 清屏并显示按键2信息
                   
                      display_print_line(1, "key 2 p");
                      break;
                  case 3:
                      // 清屏并显示按键3信息
                  
                      display_print_line(1, "key 3 p");
                      break;
                  case 4:
                     // 清屏并显示按键4信息
                     
                     display_print_line(1, "key 4 p");
                      break;
              }
              // 发送刷新命令
              display_refresh();
          }
      }
  }

static void Key_scan(void *pvParameters)
{
	// MyRTC_Init();
	int KeyNum;
 for (;;)
      {
          KeyNum = Key_GetNum();        // 获取按键键码

          if (KeyNum != 0)             // 有按键按下
          {
              BEEP_Buzz(5);            // 蜂鸣器反馈

              // 将按键值发送到队列
              if (xQueueSend(keyQueue, &KeyNum, 0) != pdPASS) {
                  // 队列满，可以选择忽略或处理错误
                  printf("Key queue full\r\n");
              }
          }

          vTaskDelay(60);              // 60ms去抖延迟
      }
  }

static void OLED_Display_Task(void *pvParameters)
{
    display_msg_t msg;
    
    for (;;)
    {
        // 等待显示消息
        if (xQueueReceive(displayQueue, &msg, portMAX_DELAY) == pdPASS) {
            switch (msg.cmd) {
                case DISPLAY_CMD_CLEAR:
                    OLED_Clear();
                    break;
                    
                case DISPLAY_CMD_CLEAR_LINE:
                    OLED_Clear_Line(msg.x);
                    break;
                    
                case DISPLAY_CMD_PRINT_LINE:
                    OLED_Printf_Line(msg.x, "%s", msg.text);
                    break;
                    
                case DISPLAY_CMD_PRINT:
                    OLED_Printf(msg.x, msg.y, "%s", msg.text);
                    break;
                    
                case DISPLAY_CMD_PRINT_32:
                    OLED_Printf_Line_32(msg.x, "%s", msg.text);
                    break;
                    
                case DISPLAY_CMD_REFRESH:
                    OLED_Refresh_Dirty();
                    break;
                    
                case DISPLAY_CMD_PICTURE:
                    if (msg.picture_data != NULL) {
                        OLED_ShowPicture(msg.x, msg.y, msg.param1, msg.param2, msg.picture_data, 1);
                    }
                    break;
            }
        }
    }
}

