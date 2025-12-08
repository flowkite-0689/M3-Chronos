/**
 * @file main_menu.c
 * @brief 横向主菜单实现文件
 * @author flowkite-0689
 * @version v1.0
 * @date 2025.12.05
 */

#include "main_menu.h"
//+++++++++++++++++++++++=
//页面
//++++++++++++++++++++++++=
#include "TandH.h"
#include "setting_menu.h"
#include "Stopwatch.h"

#include "alarm_menu.h"
#include "testlist_menu.h"

// ==================================
// 绑定状态标记结构体
// ==================================

typedef struct {
    uint8_t is_bound;          // 子菜单是否已绑定
} menu_binding_status_t;
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
    "TandH_page",
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
    
    // 为主菜单设置进入回调，用于在进入主菜单时绑定所有子菜单
    menu_item_set_callbacks(main_menu, 
                           main_menu_on_enter,  // 进入回调
                           main_menu_on_exit,   // 退出回调
                           NULL,               // 选中回调（不需要特殊处理）
                           NULL); // 按键处理
    
    // 创建各个子菜单项
    for (uint8_t i = 0; i < MAIN_MENU_COUNT; i++) {
        menu_item_t* menu_item = MENU_ITEM_ICON(
            main_menu_names[i],
            main_menu_icons[i],
            32,  // 图标宽度
            32   // 图标高度
        );

        
        if (menu_item != NULL) {
            // 子菜单项不需要设置进入回调，因为主菜单会处理绑定
            menu_item_set_callbacks(menu_item, 
                                   NULL,        // 进入回调（由主菜单处理）
                                   NULL,        // 退出回调（由主菜单处理）
                                   NULL,        // 选中回调（不需要特殊处理）
                                   NULL);      // 按键处理
            
            // 为每个菜单项分配绑定状态上下文
            menu_binding_status_t *binding_status = (menu_binding_status_t*)pvPortMalloc(sizeof(menu_binding_status_t));
            if (binding_status != NULL) {
                binding_status->is_bound = 0; // 初始状态为未绑定
                menu_item->context = binding_status;
            }
            
            // 添加到主菜单
            menu_add_child(main_menu, menu_item);
            
        }
    }
    
    return main_menu;
}

// ==================================
// 辅助函数：动态绑定子菜单
// ==================================

static void main_menu_bind_child_menu(menu_item_t *parent_item, int menu_index)
{
    if (parent_item == NULL || parent_item->context == NULL) {
        return;
    }
    printf("parent_item:%s",parent_item->name);
    menu_binding_status_t *binding_status = (menu_binding_status_t*)parent_item->context;
    
    // 检查是否已经绑定过
    if (binding_status->is_bound) {
        return;
    }
    
    // 根据菜单索引动态创建并绑定子菜单
    menu_item_t *child_menu = NULL;
    
    switch (menu_index) {
        case MAIN_MENU_STOPWATCH: // 秒表
            child_menu = Stopwatch_init();
            break;
            
        case MAIN_MENU_SETTINGS: // 设置
            child_menu = setting_menu_init();
            break;
            
        case MAIN_MENU_TEMPHUMI: // 温湿度
            child_menu = TandH_init();
            break;
            
        case MAIN_MENU_ALARM: // 闹钟
            child_menu = alarm_menu_init();
            break;
            
        case MAIN_MENU_TEST: // 测试
            printf("testlist_menu_init start init->\r\n");
            child_menu = testlist_menu_init();
            break;
            
        // 手电筒和步数计数器暂时不需要子菜单
        case MAIN_MENU_FLASHLIGHT:
        case MAIN_MENU_STEP:
        default:
            break;
    }
    
    // 如果成功创建子菜单，则添加到父菜单
    if (child_menu != NULL) {
        menu_add_child(parent_item, child_menu);
        binding_status->is_bound = 1; // 标记为已绑定
        printf("Dynamic binding: %s -> %s\r\n", parent_item->name, child_menu->name);
    }
}

void main_menu_on_enter(menu_item_t* item)
{
    printf("Enter main menu: %s\r\n", item ? item->name : "NULL");
    OLED_Clear();
    
    // 安全检查：确保当前是主菜单
    if (item == NULL || strcmp(item->name, "Main Menu") != 0) {
        return;
    }
    
    // 动态绑定所有需要子菜单的菜单项
    for (uint8_t i = 0; i < item->child_count; i++) {
        menu_item_t *menu_item = item->children[i];
        
        // 通过菜单名称匹配来确定索引
        int menu_index = -1;
        for (uint8_t j = 0; j < MAIN_MENU_COUNT; j++) {
            if (strcmp(menu_item->name, main_menu_names[j]) == 0) {
                menu_index = j;
                break;
            }
        }
        
        // 如果找到匹配的菜单项，并且是需要子菜单的类型，则进行绑定
        if (menu_index != -1) {
            if (menu_index == MAIN_MENU_STOPWATCH || menu_index == MAIN_MENU_SETTINGS || 
                menu_index == MAIN_MENU_TEMPHUMI || menu_index == MAIN_MENU_ALARM || 
                menu_index == MAIN_MENU_TEST) {
                main_menu_bind_child_menu(menu_item, menu_index);
            }
        }
    }
    
    printf("Main menu binding completed, child_count: %d\r\n", item->child_count);
}


void main_menu_on_exit(menu_item_t* item)
{
    printf("Exit main menu: %s\r\n", item ? item->name : "NULL");
    
    // 安全检查：确保当前是主菜单
    if (item == NULL || strcmp(item->name, "Main Menu") != 0) {
        return;
    }
    
    // 获取当前选中的子菜单索引
    uint8_t selected_index = item->selected_child;
    
    // 清理除当前选中项外的其他需要子菜单的菜单项
    for (uint8_t i = 0; i < item->child_count; i++) {
        // 跳过当前选中的菜单项
        if (i == selected_index) {
            printf("Skipping cleanup for selected menu: %s\r\n", item->children[i]->name);
            continue;
        }
        
        menu_item_t *menu_item = item->children[i];
        
        // 通过菜单名称匹配来确定索引
        int menu_index = -1;
        for (uint8_t j = 0; j < MAIN_MENU_COUNT; j++) {
            if (strcmp(menu_item->name, main_menu_names[j]) == 0) {
                menu_index = j;
                break;
            }
        }
        
        // 如果找到匹配的菜单项，并且是需要子菜单的类型，则进行清理
        if (menu_index != -1) {
            if (menu_index == MAIN_MENU_STOPWATCH || menu_index == MAIN_MENU_SETTINGS || 
                menu_index == MAIN_MENU_TEMPHUMI || menu_index == MAIN_MENU_ALARM || 
                menu_index == MAIN_MENU_TEST) {
                    
                printf("-----------------------------------------\n\n%s will be delate\n",menu_item->children[0]->name);
            printf("menu_item->name:%s\n",menu_item->name);
					// menu_item_delete(&(menu_item->children[0]));				
            
            }
        }
    }
    
    printf("Main menu cleanup completed, kept selected menu: %s\r\n", 
           item->children[selected_index]->name);
}
