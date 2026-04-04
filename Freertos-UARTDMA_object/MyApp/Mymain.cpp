#include "FreeRtos_demo.h"
#include "MyApplication.h"
#include "key.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include <cstdint>
#include <sys/types.h>





void Mymain() {

	MyTIM_Init();
	freertos_start(); // 创建并启动FreeRTOS任务
	

    for (;;)
    {
		
    }
}