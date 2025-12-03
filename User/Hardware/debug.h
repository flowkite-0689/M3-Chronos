#ifndef DEBUG_H
#define DEBUG_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>

void debug_init(void);
void Usart1_Send_Sring(char *string);

#endif
