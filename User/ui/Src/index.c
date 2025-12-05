/**
 * @file index.c
 * @brief 首页界面实现文件 - 基于main.c中的Page_Logic
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#include "index.h"
#include <string.h>

// ==================================
// 全局变量定义
// ==================================

index_state_t g_index_state = {0};

// ==================================
// 静态函数声明
// ==================================

static void index_display_time_info(void);
static void index_display_status_info(void);

// ==================================
// 首页实现
// ==================================

menu_item_t* index_init(void)
{
    // 初始化首页状态
    memset(&g_index_state, 0, sizeof(index_state_t));
    g_index_state.need_refresh = 1;
    g_index_state.last_update = xTaskGetTickCount();
    g_index_state.step_count = 0;
    g_index_state.step_active = 0;
    
    // 初始化RTC
    MyRTC_Init();
    
    // 创建首页菜单项
    menu_item_t* index_menu = MENU_ITEM_CUSTOM("Index", index_draw_function, &g_index_state);
    if (index_menu == NULL) {
        return NULL;
    }
    
    // 设置回调函数
    menu_item_set_callbacks(index_menu, index_on_enter, index_on_exit, NULL, index_key_handler);
    
    printf("Index page initialized successfully\r\n");
    return index_menu;
}

void index_draw_function(void* context)
{
    index_state_t* state = (index_state_t*)context;
    if (state == NULL) {
        return;
    }
    
    
    
    // 更新时间信息
    index_update_time();
    
    // 显示时间信息
    index_display_time_info();
    
    // 显示状态信息
    index_display_status_info();
    
    // 刷新显示
    OLED_Refresh_Dirty();
}

void index_key_handler(menu_item_t* item, uint8_t key_event)
{
    index_state_t* state = (index_state_t*)item->content.custom.draw_context;
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 可以用来切换某些状态或进入特定功能
            printf("Index: KEY0 pressed\r\n");
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 可以用来切换某些状态或进入特定功能
            printf("Index: KEY1 pressed\r\n");
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 可以返回上一级或特定功能
            printf("Index: KEY2 pressed\r\n");
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 进入功能菜单或执行特定操作
            printf("Index: KEY3 pressed - Enter menu\r\n");
            // 这里可以添加进入主菜单的逻辑
            break;
            
        case MENU_EVENT_REFRESH:
            // 刷新显示
            state->need_refresh = 1;
            break;
            
        default:
            break;
    }
    
    // 标记需要刷新
    state->need_refresh = 1;
}

void index_update_time(void)
{
    // 读取RTC时间
    MyRTC_ReadTime();
    
    // 更新状态
    g_index_state.hours = RTC_data.hours;
    g_index_state.minutes = RTC_data.minutes;
    g_index_state.seconds = RTC_data.seconds;
    g_index_state.day = RTC_data.day;
    g_index_state.month = RTC_data.mon;
    g_index_state.year = RTC_data.year;
    strncpy(g_index_state.weekday, RTC_data.weekday, sizeof(g_index_state.weekday) - 1);
    g_index_state.weekday[sizeof(g_index_state.weekday) - 1] = '\0';
}

index_state_t* index_get_state(void)
{
    return &g_index_state;
}

void index_refresh_display(void)
{
    g_index_state.need_refresh = 1;
    g_index_state.last_update = xTaskGetTickCount();
}

void index_on_enter(menu_item_t* item)
{
    printf("Enter index page\r\n");
    g_index_state.need_refresh = 1;
    
    // 重新初始化RTC
    MyRTC_Init();
}

void index_on_exit(menu_item_t* item)
{
    printf("Exit index page\r\n");
}

// ==================================
// 静态函数实现
// ==================================

static void index_display_time_info(void)
{
    // 显示日期和星期：第0行
    OLED_Printf_Line(0, "%02d/%02d/%02d %s",
                     g_index_state.year ,  
                     g_index_state.month,
                     g_index_state.day,
                     g_index_state.weekday);
    
    // 显示时间：第1行，使用32像素字体
    OLED_Printf_Line_32(1, " %02d:%02d:%02d", 
                       g_index_state.hours, 
                       g_index_state.minutes, 
                       g_index_state.seconds);
}

static void index_display_status_info(void)
{
    // 显示步数信息：第3行
    OLED_Printf_Line(3, "step : %lu", g_index_state.step_count);
    
    // 计算一天中的分钟数
    int time_of_day = (g_index_state.hours * 60 + g_index_state.minutes);
    
    // 绘制底部进度条：表示一天的时间进度
    OLED_DrawProgressBar(0, 44, 125, 2, time_of_day, 0, 24 * 60, 0, 1);
    
    // 绘制右侧进度条：表示秒数进度
    OLED_DrawProgressBar(125, 0, 2, 64, g_index_state.seconds, 0, 60, 0, 1);
}
