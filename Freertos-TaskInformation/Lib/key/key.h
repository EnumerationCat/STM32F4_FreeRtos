
#ifndef _Key_H
#define _Key_H


#ifdef __cplusplus
extern "C" {
#endif
    /*****************C*****************/
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "tim.h"
#include <stdint.h>
#include "FreeRtos_demo.h"


#define KEY_COUNT 1 // 定义按键数量，根据实际情况调整


#define KEY_PRESSED 1
#define KEY_NOPRESSED 0
#define KEY_TIME_DOUBLE 200 // 双击时间阈值，单位为ms
#define KEY_TIME_LONG 1000 // 长按时间阈值，单位为ms
#define KEY_TIME_REPEAT 100 // 重复按键时间阈值，单位为ms

#define KEY_HOLD      0x01
#define KEY_DOWN      0x02
#define KEY_UP        0x04
#define KEY_SINGLE    0x08
#define KEY_DOUBLE    0x10
#define KEY_LONG      0x20
#define KEY_REPEAT    0x40




#define KEY_IDLE 0
#define KEY_HasBeenPressed 1
#define KEY_HasBeenLoosened 2
#define KEY_HasBeenDoubleClicked 3
#define KEY_HasBeenLongPressed 4


uint8_t getKeyState(uint8_t keyIndex);
void Key_Tick(void);
uint8_t Key_Check(uint8_t keyIndex, uint8_t Flag);
void Key_Scanf(void);

#ifdef __cplusplus
}
/**********************C++*************************/

#endif

#endif 
