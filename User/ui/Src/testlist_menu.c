#include "testlist_menu.h"


#include "air_level.h"
// 引用全局菜单系统变量
extern menu_system_t g_menu_sys;

const char *testlist_menu_text[] = {
    "SPI_test",
    "2048_oled",
    "frid_test",
    "iwdg_test",
    "air_level"};

menu_item_t *testlist_menu_init(void)
{
  menu_item_t *testlist_menu = menu_item_create(
      "TestList Menu",
      MENU_TYPE_VERTICAL_LIST,
      (menu_content_t){0} // 主菜单不需要内容
  );

  if (testlist_menu == NULL)
  {
    printf("testlist init fail\r\n");
    return NULL;
  }

  for (uint8_t i = 0; i < TESTLIST_MENU_COUNT; i++)
  {
    menu_item_t *menu_item = MENU_ITEM_TEXT(
        testlist_menu_text[i],
        testlist_menu_text[i],
        15);
    if (menu_item != NULL)
    {
      menu_item_set_callbacks(menu_item,
                              testlist_menu_on_enter,
                              testlist_menu_on_exit,
                              NULL,  // on_select 暂时设为NULL，因为没有对应的子页面
                              NULL); // on_key 使用默认的按键处理
                menu_add_child(testlist_menu, menu_item);  
                   if (i == TESTLIST_MENU_AIR_LEVEL) {
          menu_item_t* air_level_page = air_level_init();
          if (air_level_page != NULL) {
              menu_add_child(menu_item, air_level_page);
          }
      }             
    }
  }
  
  // 设置第一个子项为选中状态
  if (testlist_menu->child_count > 0) {
      testlist_menu->selected_child = 0;
  }
  
 return testlist_menu;
}

void testlist_menu_on_enter(menu_item_t *item)
{
  printf("Enter testlist menu\r\n");
  OLED_Clear();
}

void testlist_menu_on_exit(menu_item_t *item)
{
   printf("Exit testlist menu\r\n");
}
