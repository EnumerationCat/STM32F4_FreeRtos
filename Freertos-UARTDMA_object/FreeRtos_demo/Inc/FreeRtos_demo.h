
#ifndef _FREERTOS_DEMO_H
#define _FREERTOS_DEMO_H


#ifdef __cplusplus
extern "C" {
#endif
    /*****************C*****************/
#include "stm32f4xx_hal.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>       // 可变参数处理头文件
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "key.h"
#include "uart_device.h"

void usbRecData(void);
int usbprintf(const char *format, ...);


void PRE_SLEEP_PROCESSING(void);
void POST_SLEEP_PROCESSING(void);

void freertos_start(void);


#ifdef __cplusplus
}
/**********************C++*************************/

#endif

#endif 


