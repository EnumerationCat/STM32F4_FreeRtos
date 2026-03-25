#include "FreeRtos_demo.h"
#include "MyApplication.h"
#include "key.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "tim.h"
#include <cstdint>
#include <sys/types.h>


// 记录上一次打印的时间戳（初始值为当前Tick）
uint32_t last_print_tick;

void RunningTimePrint(void)
{
	// 核心逻辑：判断当前Tick与上一次的差值是否≥1000ms（1秒）
		if (HAL_GetTick() - last_print_tick >= 1000)
		{
			// 执行每秒一次的打印操作
			usbprintf("当前累计Tick：%lu ms | 距离启动已%lu秒\r\n", 
			
			HAL_GetTick(), HAL_GetTick()/1000);
			
			// 更新上一次打印的时间戳，避免重复触发
			last_print_tick = HAL_GetTick();
		}
		
		// 核心逻辑：判断当前Tick与上一次的差值是否≥1000ms（1秒）
        if (HAL_GetTick() - last_print_tick >= 1000)
        {
            // 执行每秒一次的打印操作
            usbprintf("当前累计Tick：%lu ms | 距离启动已%lu秒\r\n", 
			
            HAL_GetTick(), HAL_GetTick()/1000);
			
            // 更新上一次打印的时间戳，避免重复触发
            last_print_tick = HAL_GetTick();
        }
}



void Mymain() {

	MyTIM_Init();
	freertos_start(); // 创建并启动FreeRTOS任务
	

    for (;;)
    {
		//Key_Scan(); // 扫描按键状态并处理事件
		RunningTimePrint(); // 打印运行时间
    }
}