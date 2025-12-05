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
#include "unified_menu.h"
#include "index.h"
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

static TaskHandle_t Menu_handle = NULL;
static TaskHandle_t Key_handle = NULL;

/* 任务函数声明 */
static void Menu_Main_Task(void *pvParameters);
static void Key_Main_Task(void *pvParameters);

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
    // 系统初始化开始
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

    // 初始化菜单系统
    if (menu_system_init() != 0) {
        printf("Menu system initialization failed\r\n");
        return -1;
    }
    
    // 可选：启用显示队列模式
    // 如果想使用队列模式，取消下面的注释
    if (menu_display_queue_init() != 0) {
        printf("Display queue initialization failed, using direct mode\r\n");
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

    /* 创建菜单任务 */
    xTaskCreate((TaskFunction_t)Menu_Main_Task,          /* 任务函数 */
                (const char *)"Menu_Main",               /* 任务名称 */
                (uint16_t)512,                          /* 任务堆栈大小 */
                (void *)NULL,                           /* 任务函数参数 */
                (UBaseType_t)3,                         /* 任务优先级 */
                (TaskHandle_t *)&Menu_handle);           /* 任务控制句柄 */
    xTaskCreate(Key_Main_Task, "KeyMain", 128, NULL, 4, &Key_handle);
    
    // LED1_OFF();
    /* 启动调度器 */
    vTaskStartScheduler();
    LED2_ON();
}

static void Menu_Main_Task(void *pvParameters)
{
    // 直接调用统一菜单框架的任务
    menu_task(pvParameters);
}

static void Key_Main_Task(void *pvParameters)
{
    // 直接调用统一菜单框架的按键任务
    menu_key_task(pvParameters);
}
