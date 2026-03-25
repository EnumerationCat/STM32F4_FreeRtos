
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
#include "usbd_cdc_if.h"
#include "key.h"

#define SYS_SUPPORT_OS 1

void usbRecData(void);
int usbprintf(const char *format, ...);

void freertos_start(void);


#ifdef __cplusplus
}
/**********************C++*************************/

#endif

#endif 


