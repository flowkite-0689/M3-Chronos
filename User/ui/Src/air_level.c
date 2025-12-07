#include "air_level.h"

// ==================================
// 本页面变量定义
// ==================================
static air_level_state_t s_air_level_state = {0};

// ==================================
// 静态函数声明
// ==================================
static void air_level_display_info_wrapper(void);

// ==================================
// 页面初始化
// ==================================

/**
 * @brief 初始化水平仪页面
 * @return 创建的水平仪页面指针
 */
menu_item_t *air_level_init(void)
{
    // 初始化状态结构体
    memset(&s_air_level_state, 0, sizeof(s_air_level_state));
    s_air_level_state.need_refresh = 1;
    s_air_level_state.last_update = xTaskGetTickCount();
    s_air_level_state.sensor_ready = 0;
    
    // 初始化传感器
//    if (MPU_Init() == 0) {
//        s_air_level_state.sensor_ready = 1;
//        printf("MPU6050 initialized successfully\r\n");
//    } else {
       printf("MPU6050 initialization failed\r\n");
//    }
    
    // 创建自定义菜单项
    menu_item_t *air_level_page = MENU_ITEM_CUSTOM("Air Level", air_level_draw_function, &s_air_level_state);
    if (air_level_page == NULL) {
        return NULL;
    }
    
    menu_item_set_callbacks(air_level_page, air_level_on_enter, air_level_on_exit, NULL, air_level_key_handler);
    
    printf("Air Level page initialized successfully\r\n");
    return air_level_page;
}

// ==================================
// 自定义绘制函数
// ==================================

/**
 * @brief 水平仪自定义绘制函数
 * @param context 绘制上下文
 */
void air_level_draw_function(void *context)
{
    air_level_state_t *state = (air_level_state_t *)context;
    if (state == NULL) {
        return;
    }
    
    // 更新传感器数据
    air_level_update_data(state);
    
    // 显示水平仪信息
    air_level_display_info_wrapper();
    
    OLED_Refresh_Dirty();
}

// ==================================
// 按键处理
// ==================================

/**
 * @brief 水平仪按键处理函数
 * @param item 菜单项
 * @param key_event 按键事件
 */
void air_level_key_handler(menu_item_t *item, uint8_t key_event)
{
    air_level_state_t *state = (air_level_state_t *)item->content.custom.draw_context;
    
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // KEY0 - 可以用来切换显示模式或校准
            printf("Air Level: KEY0 pressed\r\n");
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // KEY1 - 可以用来切换显示模式或校准
            printf("Air Level: KEY1 pressed\r\n");
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // KEY2 - 返回上一级
            printf("Air Level: KEY2 pressed - Return\r\n");
            menu_back_to_parent();
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // KEY3 - 可以用于校准或特殊功能
            printf("Air Level: KEY3 pressed - Calibrate\r\n");
            // 可以添加校准功能
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

// ==================================
// 回调函数
// ==================================

/**
 * @brief 水平仪页面进入回调
 * @param item 菜单项
 */
void air_level_on_enter(menu_item_t *item)
{
    printf("Enter Air Level page\r\n");
    OLED_Clear();
    s_air_level_state.need_refresh = 1;
}

/**
 * @brief 水平仪页面退出回调
 * @param item 菜单项
 */
void air_level_on_exit(menu_item_t *item)
{
    printf("Exit Air Level page\r\n");
    OLED_Clear();
}

// ==================================
// 传感器数据处理
// ==================================

/**
 * @brief 计算倾斜角度
 * @param ax X轴加速度
 * @param ay Y轴加速度
 * @param az Z轴加速度
 * @param angle_x 输出X轴角度
 * @param angle_y 输出Y轴角度
 */
void calculate_tilt_angles(short ax, short ay, short az, float *angle_x, float *angle_y)
{
    // 使用双精度浮点数计算以提高精度
    double ax_d = (double)ax;
    double ay_d = (double)ay;
    double az_d = (double)az;
    
    *angle_x = atan2(ax_d, sqrt(ay_d * ay_d + az_d * az_d)) * 180.0 / 3.14159265;
    *angle_y = atan2(ay_d, sqrt(ax_d * ax_d + az_d * az_d)) * 180.0 / 3.14159265;
}

/**
 * @brief 获取当前倾斜方向和角度
 * @param state 水平仪状态
 */
void air_level_update_data(air_level_state_t *state)
{
    if (!state->sensor_ready) {
        return;
    }
    
    short ax, ay, az;
    
//    if (MPU_Get_Accelerometer(&ax, &ay, &az) == 0) {
//        // 计算原始角度
//        calculate_tilt_angles(ax, ay, az, &state->angle_x, &state->angle_y);
//        
//        // 平滑处理角度数据（避免跳动）
//        const float smoothing_factor = 0.3f;
//        state->last_angle_x = state->last_angle_x * (1 - smoothing_factor) + state->angle_x * smoothing_factor;
//        state->last_angle_y = state->last_angle_y * (1 - smoothing_factor) + state->angle_y * smoothing_factor;
//        
//        // 确定方向文本
//        const float trigger_threshold = 20.0f;
//        const float trend_threshold = 5.0f;
//        
//        if (fabsf(state->last_angle_x) > fabsf(state->last_angle_y)) {
//            // X轴为主要倾斜方向
//            if (state->last_angle_x < -trigger_threshold) {
//                strcpy(state->direction_text, "up   ");
//            } else if (state->last_angle_x > trigger_threshold) {
//                strcpy(state->direction_text, "down ");
//            } else if (state->last_angle_x < -trend_threshold) {
//                strcpy(state->direction_text, "up~   ");
//            } else if (state->last_angle_x > trend_threshold) {
//                strcpy(state->direction_text, "down~");
//            } else {
//                strcpy(state->direction_text, "flat ");
//            }
//            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_x);
//        } else {
//            // Y轴为主要倾斜方向
//            if (state->last_angle_y > trigger_threshold) {
//                strcpy(state->direction_text, "right");
//            } else if (state->last_angle_y < -trigger_threshold) {
//                strcpy(state->direction_text, "left  ");
//            } else if (state->last_angle_y > trend_threshold) {
//                strcpy(state->direction_text, "right~");
//            } else if (state->last_angle_y < -trend_threshold) {
//                strcpy(state->direction_text, "left~  ");
//            } else {
//                strcpy(state->direction_text, "flat   ");
//            }
//            snprintf(state->angle_text, 16, " %.1f^     ", state->last_angle_y);
//        }
//    }
}

// ==================================
// 显示函数
// ==================================

/**
 * @brief 显示水平仪信息
 * @param state 水平仪状态
 */
void air_level_display_info(air_level_state_t *state)
{
    if (!state->sensor_ready) {
        OLED_Printf_Line(0, "MPU6050 Error!");
        OLED_Printf_Line(1, "Check Sensor");
        return;
    }
    
    // 显示方向信息
    OLED_Printf_Line(0, "Direction: %s", state->direction_text);
    OLED_Printf_Line(1, "Angle: %s", state->angle_text);
    
    // 显示X轴角度（前后倾斜）
    OLED_Printf_Line(2, "X: %0.1f^", state->last_angle_x);
    
    // 显示Y轴角度（左右倾斜）
    OLED_Printf_Line(3, "Y: %0.1f^", state->last_angle_y);
    
    // 绘制水平指示器
    // 垂直进度条显示Y轴倾斜（左右倾斜）
    int y_angle_scaled = (int)(state->last_angle_y * 10);
    OLED_DrawProgressBar(96, 0, 2, 64, y_angle_scaled, -900, 900, 0, 1, 0);
    
    // 水平进度条显示X轴倾斜（前后倾斜）
    int x_angle_scaled = (int)(state->last_angle_x * 10);
    OLED_DrawProgressBar(64, 32, 64, 2, x_angle_scaled, -900, 900, 0, 1, 0);
}

/**
 * @brief 静态显示函数包装器（供内部使用）
 */
static void air_level_display_info_wrapper(void)
{
    air_level_display_info(&s_air_level_state);
}

