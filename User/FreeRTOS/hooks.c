#include <FreeRTOS.h>
#include <task.h>


void vApplicationIdleHook( void )
{
}
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
void vApplicationTickHook( void )
{
   
}
void vApplicationMallocFailedHook( void )
{
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
