/**
 * @file index.h
 * @brief 自定义首页界面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#ifndef __INDEX_H
#define __INDEX_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "unified_menu.h"
#include "oled_print.h"

// ==================================
// 首页状态数据结构
// ==================================

typedef struct {
    // 时间信息
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    
    // 系统状态
    uint8_t wifi_connected;      // WiFi连接状态
    uint8_t alarm_active;       // 闹钟状态
    uint8_t battery_level;      // 电池电量
    float temperature;          // 温度
    
    // 交互状态
    uint8_t current_selection;   // 当前选中项
    uint8_t edit_mode;          // 编辑模式
    uint8_t blink_state;        // 闪烁状态
    
    // 刷新标志
    uint8_t need_refresh;       // 需要刷新
    uint32_t last_update;       // 上次更新时间
} index_state_t;

// ==================================
// 首页选项枚举
// ==================================

typedef enum {
    INDEX_MENU_CLOCK = 0,       // 时钟功能
    INDEX_MENU_ALARM,           // 闹钟功能
    INDEX_MENU_SETTINGS,        // 设置功能
    INDEX_MENU_TEST,            // 测试功能
    INDEX_MENU_COUNT            // 选项总数
} index_menu_option_t;

// ==================================
// 函数声明
// ==================================

/**
 * @brief 初始化首页
 * @return 创建的首页菜单项指针
 */
menu_item_t* index_init(void);

/**
 * @brief 首页自定义绘制函数
 * @param context 绘制上下文（首页状态指针）
 */
void index_draw_function(void* context);

/**
 * @brief 首页按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void index_key_handler(menu_item_t* item, uint8_t key_event);

/**
 * @brief 更新首页状态信息
 */
void index_update_state(void);

/**
 * @brief 获取首页状态
 * @return 首页状态指针
 */
index_state_t* index_get_state(void);

/**
 * @brief 进入选中的功能
 * @param selection 选中的功能索引
 */
void index_enter_function(uint8_t selection);

/**
 * @brief 刷新首页显示
 */
void index_refresh_display(void);

#endif // __INDEX_H