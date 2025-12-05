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
#include <string.h>
#include <stdarg.h> // 加在文件顶部（已有 <string.h>，补上这个）
// 创建队列来存储按键事件
QueueHandle_t keyQueue;     // 按键队列
QueueHandle_t displayQueue; // 显示队列

// 显示命令类型定义
typedef enum
{
    DISPLAY_CMD_CLEAR,      // 清屏
    DISPLAY_CMD_CLEAR_LINE, // 清除指定行
    DISPLAY_CMD_PRINT_LINE, // 行打印
    DISPLAY_CMD_PRINT,      // 指定位置打印
    DISPLAY_CMD_PRINT_32,   // 32px字体行打印
    DISPLAY_CMD_REFRESH,    // 刷新显示
    DISPLAY_CMD_PICTURE,    // 显示图片
    DISPLAY_CMD_PROGRESS_BAR
} display_cmd_type_t;

// 显示消息结构体
typedef struct
{
    display_cmd_type_t cmd;
    uint8_t x;     // 左上角 X
    uint8_t y;     // 左上角 Y
    char text[32]; // 文本缓冲（PRINT 类命令用）

    union
    {
        struct
        {
            uint8_t width;
            uint8_t height;
            const unsigned char *data;
        } pic;

        struct
        {
            uint8_t width;
            uint8_t height;
            int32_t value;
            int32_t min_val;
            int32_t max_val;
            uint8_t show_border;
            uint8_t fill_mode;
        } progress;
    } extra;
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
        .x = line};
    xQueueSend(displayQueue, &msg, 0);
}

void display_print_line(uint8_t line, const char *format, ...)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT_LINE,
        .x = line};

    va_list args;
    va_start(args, format);
    vsnprintf(msg.text, sizeof(msg.text), format, args);
    va_end(args);

    // 确保结尾为 \0（vsnprintf 会自动加，但防 buffer 满时截断无 \0）
    msg.text[sizeof(msg.text) - 1] = '\0';

    xQueueSend(displayQueue, &msg, 0);
}

void display_print_line_32(uint8_t line, const char *format, ...)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT_32,
        .x = line};

    va_list args;
    va_start(args, format);
    vsnprintf(msg.text, sizeof(msg.text), format, args);
    va_end(args);

    // 确保结尾为 \0（vsnprintf 会自动加，但防 buffer 满时截断无 \0）
    msg.text[sizeof(msg.text) - 1] = '\0';

    xQueueSend(displayQueue, &msg, 0);
}

void display_print(uint8_t x, uint8_t y, const char *text)
{
    display_msg_t msg = {
        .cmd = DISPLAY_CMD_PRINT,
        .x = x,
        .y = y};
    strncpy(msg.text, text, sizeof(msg.text) - 1);
    msg.text[sizeof(msg.text) - 1] = '\0';
    xQueueSend(displayQueue, &msg, 0);
}

void display_DrawProgressBar(
    uint8_t x, uint8_t y,
    uint8_t width, uint8_t height,
    int32_t value,
    int32_t min_val, int32_t max_val,
    uint8_t show_border,
    uint8_t fill_mode)
{
    display_msg_t msg; // 初始化为0（避免联合体垃圾数据）
  memset(&msg, 0, sizeof(msg));  
	msg.cmd = DISPLAY_CMD_PROGRESS_BAR;
    msg.x = x;
    msg.y = y;

    msg.extra.progress.width = width;
    msg.extra.progress.height = height;
    msg.extra.progress.value = value;
    msg.extra.progress.min_val = min_val;
    msg.extra.progress.max_val = max_val;
    msg.extra.progress.show_border = show_border;
    msg.extra.progress.fill_mode = fill_mode;

    xQueueSend(displayQueue, &msg, 0);
}

void display_refresh(void)
{
    display_msg_t msg = {.cmd = DISPLAY_CMD_REFRESH};
    xQueueSend(displayQueue, &msg, 0);
}

int main(void)
{
    // 系统开始初始化
    TIM2_Delay_Init();
    debug_init();
    OLED_Init();
    OLED_ShowPicture(32, 0, 64, 64, gImage_bg, 1);
    OLED_Refresh();
    Key_Init();
    Beep_Init();
    // RTC_SetTime_Manual(15,22,0);

    // 系统初始化成功
    printf("init OK\n");
    OLED_Clear();
    OLED_Printf_Line(3, "sys OK...");
    OLED_Refresh();

    keyQueue = xQueueCreate(10, sizeof(int));
    displayQueue = xQueueCreate(20, sizeof(display_msg_t));

    if (keyQueue == NULL)
    {
        printf("Failed to create key queue\r\n");
        return -1;
    }
    if (displayQueue == NULL)
    {
        printf("Failed to create display queue\r\n");
        return -1;
    }

    /* 创建app_task1任务 */
    xTaskCreate((TaskFunction_t)Key_scan,          /* 任务入口函数 */
                (const char *)"Key_scan",          /* 任务名字 */
                (uint16_t)512,                     /* 任务栈大小 */
                (void *)NULL,                      /* 任务入口函数参数 */
                (UBaseType_t)4,                    /* 任务的优先级 */
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
    MyRTC_Init();
    for (;;)
    {
        MyRTC_ReadTime();
        // 获取rtc时间
        display_print_line(0, "%02d/%02d/%02d %s",
                           RTC_data.year,
                           RTC_data.mon,
                           RTC_data.day,
                           RTC_data.weekday);

        display_print_line_32(1, " %02d:%02d:%02d", RTC_data.hours, RTC_data.minutes, RTC_data.seconds);
        
        
        vTaskDelay(60);
        display_print_line(3,"step : 0");//预留

        int timeofdaybeuse = (RTC_data.hours * 60 + RTC_data.minutes);
		display_DrawProgressBar(0, 44, 125, 2, timeofdaybeuse, 0, 24 * 60, 0, 1);
		display_DrawProgressBar(125, 0, 2, 64, RTC_data.seconds, 0, 60, 0, 1); // ????
		display_refresh();
        // 等待按键事件
        if (xQueueReceive(keyQueue, &received_key, 0) == pdPASS)
        {
            // 处理按键事件
            switch (received_key)
            {
            case 1:
              

                break;
            case 2:
              

         
                break;
            case 3:
              

               
                break;
            case 4:
                
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
        KeyNum = Key_GetNum(); // 获取按键键码

        if (KeyNum != 0) // 有按键按下
        {
            BEEP_Buzz(5); // 蜂鸣器反馈

            // 将按键值发送到队列
            if (xQueueSend(keyQueue, &KeyNum, 0) != pdPASS)
            {
                // 队列满，可以选择忽略或处理错误
                printf("Key queue full\r\n");
            }
        }

        vTaskDelay(60); // 60ms去抖延迟
    }
}

static void OLED_Display_Task(void *pvParameters)
{
    display_msg_t msg;

    for (;;)
    {
        // 等待显示消息
        if (xQueueReceive(displayQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            switch (msg.cmd)
            {
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
                if (msg.extra.pic.data != NULL)
                {
                    OLED_ShowPicture(msg.x, msg.y, msg.extra.pic.width, msg.extra.pic.height, msg.extra.pic.data, 1);
                }
                break;
            case DISPLAY_CMD_PROGRESS_BAR:
                OLED_DrawProgressBar(
                    msg.x, msg.y,
                    msg.extra.progress.width, msg.extra.progress.height,
                    msg.extra.progress.value,
                    msg.extra.progress.min_val, msg.extra.progress.max_val,
                    msg.extra.progress.show_border,
                    msg.extra.progress.fill_mode);
                break;
            }
        }
    }
}
