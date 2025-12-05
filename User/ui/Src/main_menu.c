/**
 * @file main_menu.c
 * @brief 横向主菜单实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#include "main_menu.h"
// ==================================
// 图标数组
// ==================================

const unsigned char *main_menu_icons[] = {
    gImage_stopwatch,    // 秒表
    gImage_setting,      // 设置
    gImage_TandH,        // 温湿度
    gImage_flashlight,   // 手电筒
    gImage_bell,         // 闹钟
    gImage_step,         // 步数
    gImage_test          // 测试
};

const char *main_menu_names[] = {
    "Stopwatch",
    "Settings",
    "Temp&Humid",
    "Flashlight",
    "Alarm",
    "Step Counter",
    "Test"
};

// ==================================
// 函数实现
// ==================================

menu_item_t* main_menu_init(void)
{

    // 创建主菜单（横向图标菜单）
    menu_item_t* main_menu = menu_item_create(
        "Main Menu",
        MENU_TYPE_HORIZONTAL_ICON,
        (menu_content_t){0}  // 主菜单不需要内容
    );
    
    if (main_menu == NULL) {
        return NULL;
    }
    
    // 创建各个子菜单项
    for (uint8_t i = 0; i < MAIN_MENU_COUNT; i++) {
        menu_item_t* menu_item = MENU_ITEM_ICON(
            main_menu_names[i],
            main_menu_icons[i],
            32,  // 图标宽度
            32   // 图标高度
        );
        
        if (menu_item != NULL) {
            // 设置回调函数（可选，主要用于按键处理等）
            menu_item_set_callbacks(menu_item, 
                                   main_menu_on_enter,  // 进入回调
                                   main_menu_on_exit,   // 退出回调
                                   NULL,               // 选中回调（不需要特殊处理）
                                   main_menu_key_handler); // 按键处理
            
            // 添加到主菜单
            menu_add_child(main_menu, menu_item);
            
        }
    }
    
    return main_menu;
}

void main_menu_on_enter(menu_item_t* item)
{
    printf("Enter main menu\r\n");
    OLED_Clear();
    // 主菜单进入时的初始化操作
}

void main_menu_on_exit(menu_item_t* item)
{
    printf("Exit main menu\r\n");
    // 主菜单退出时的清理操作
}

void main_menu_key_handler(menu_item_t* item, uint8_t key_event)
{
    // 主菜单的按键处理
    switch (key_event) {
        case MENU_EVENT_KEY_UP:
            // 向左导航
            printf("Main menu: Navigate Left\r\n");
            break;
            
        case MENU_EVENT_KEY_DOWN:
            // 向右导航
            printf("Main menu: Navigate Right\r\n");
            break;
            
        case MENU_EVENT_KEY_ENTER:
            // 进入选中功能
            printf("Main menu: Enter selected function: %s\r\n", 
                   item->children[item->selected_child].name);
            // 注意：统一框架会自动处理子菜单进入
            break;
            
        case MENU_EVENT_KEY_SELECT:
            // 返回上一级（回到index页面）
            printf("Main menu: Back to index\r\n");
            menu_back_to_parent();
            break;
            
        default:
            break;
    }
}
