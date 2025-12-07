/**
 * @file air_level.h
 * @brief 水平仪页面头文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.06
 */

#ifndef __AIR_LEVEL_H
#define __AIR_LEVEL_H

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unified_menu.h"
#include "oled_print.h"
#include "Key.h"
#include "MPU6050_hardware_i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ==================================
// 水平仪状态结构体
// ==================================

typedef struct {
    // 传感器数据
    float angle_x;           // X轴角度（前后倾斜）
    float angle_y;           // Y轴角度（左右倾斜）
    float last_angle_x;      // 上次X轴角度（用于平滑显示）
    float last_angle_y;      // 上次Y轴角度（用于平滑显示）
    
    // 显示控制
    uint8_t need_refresh;    // 需要刷新标志
    uint32_t last_update;    // 上次更新时间
    
    // 信息显示
    char direction_text[16]; // 方向文本
    char angle_text[16];     // 角度文本
    
    // 传感器状态
    uint8_t sensor_ready;    // 传感器就绪状态
} air_level_state_t;

// ==================================
// 函数声明
// ==================================

/**
 * @brief 初始化水平仪页面
 * @return 创建的水平仪页面指针
 */
menu_item_t* air_level_init(void);

/**
 * @brief 水平仪自定义绘制函数
 * @param context 绘制上下文
 */
void air_level_draw_function(void *context);

/**
 * @brief 水平仪按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void air_level_key_handler(menu_item_t *item, uint8_t key_event);

/**
 * @brief 水平仪页面进入回调
 * @param item 菜单项
 */
void air_level_on_enter(menu_item_t *item);

/**
 * @brief 水平仪页面退出回调
 * @param item 菜单项
 */
void air_level_on_exit(menu_item_t *item);

/**
 * @brief 计算倾斜角度
 * @param ax X轴加速度
 * @param ay Y轴加速度
 * @param az Z轴加速度
 * @param angle_x 输出X轴角度
 * @param angle_y 输出Y轴角度
 */
void calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y);

/**
 * @brief 获取当前倾斜方向和角度
 * @param state 水平仪状态
 */
void air_level_update_data(air_level_state_t *state);

/**
 * @brief 显示水平仪信息
 * @param state 水平仪状态
 */
void air_level_display_info(air_level_state_t *state);

#endif // __AIR_LEVEL_H
