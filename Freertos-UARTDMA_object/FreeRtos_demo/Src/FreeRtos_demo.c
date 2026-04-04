
#include "FreeRtos_demo.h"
#include "FreeRTOS.h"
#include "key.h"
#include "portable.h"
#include "portmacro.h"
#include "projdefs.h"
#include "stm32f4xx_hal.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"





/**

 * @brief  重定向标准输出到串口1（适用于GCC）
 */
int _write(int file, char *ptr, int len)
{
    if (file == STDOUT_FILENO || file == STDERR_FILENO)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
        return len;
    }

    errno = EBADF;
    return -1;
}



/**
 * @brief 栈溢出钩子函数（configCHECK_FOR_STACK_OVERFLOW>0 时必须实现）
 * @param pxTask 发生栈溢出的任务句柄
 * @param pcTaskName 发生栈溢出的任务名
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    /* 栈溢出处理逻辑，比如打印任务名、触发硬件看门狗、记录日志等 */
    ( void ) pxTask;
    ( void ) pcTaskName;
    /* 死循环（可选，便于调试定位问题） */
    for( ;; )
	{
		printf("StackOverflow!");
		vTaskDelay(1000); // 延时1秒，避免打印过快导致串口堵塞
	}
}








/* 进入低功耗前所需要执行的操作 */
void PRE_SLEEP_PROCESSING(void)
{
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOF_CLK_DISABLE();
    __HAL_RCC_GPIOG_CLK_DISABLE();
}
/* 退出低功耗后所需要执行的操作 */
void POST_SLEEP_PROCESSING(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
}





/*启动任务的配置*/
#define START_TASK_STACK 130
#define START_TASK_PRIORITY 1
TaskHandle_t start_TaskHandle;
void start_Task(void * pvParameters);


/*任务1的配置*/
#define TASK1_STACK 1024
#define TASK1_PRIORITY 4
TaskHandle_t task1_TaskHandle;
void task1(void * pvParameters);

/*任务2的配置*/
#define TASK2_STACK 512
#define TASK2_PRIORITY 3
TaskHandle_t task2_TaskHandle;
void task2(void * pvParameters);

/*任务3的配置*/
#define TASK3_STACK 512
#define TASK3_PRIORITY 2
TaskHandle_t task3_TaskHandle;
void task3(void * pvParameters);



void timer1_callback(TimerHandle_t xTimer);
TimerHandle_t timer1Handle;



void timer2_callback(TimerHandle_t xTimer);
TimerHandle_t timer2Handle;



void freertos_start(void)
{


	

	/*创建软件定时器*/
	timer1Handle = xTimerCreate((char *) "Timer1", //定时器名称
		(TickType_t) 500, //定时器周期（单位：tick）
		(BaseType_t) pdFALSE, //是否自动重载
		(void *) 1, //定时器ID
		(TimerCallbackFunction_t )timer1_callback //定时器回调函数
	);

	if(timer1Handle != NULL)
	{
		printf("单次timer1创建成功!\r\n");
	}

	/*创建软件定时器*/
	timer2Handle = xTimerCreate((char *) "Timer2", //定时器名称
		(TickType_t) 1000, //定时器周期（单位：tick）
		(BaseType_t) pdTRUE, //是否自动重载
		(void *) 2, //定时器ID
		(TimerCallbackFunction_t )timer2_callback //定时器回调函数
	);

	if(timer2Handle != NULL)
	{
		printf("重复timer2创建成功!\r\n");
	}


	
	/*创建启动任务*/
	xTaskCreate((TaskFunction_t) start_Task,//任务函数的地址
		 (char *) "start_Task",//任务名字
		 (configSTACK_DEPTH_TYPE) START_TASK_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) START_TASK_PRIORITY,//任务优先级
		 (TaskHandle_t *) &start_TaskHandle//任务句柄的地址
		);

	/*启动调度器*/
	vTaskStartScheduler();

}


/**
 * @brief  启动任务函数，创建其他任务并删除自身
 * @param  pvParameters: 传递给任务函数的参数（此处未使用）
 */

void start_Task(void * pvParameters)
{

	taskENTER_CRITICAL(); // 进入临界区，防止任务切换干扰任务创建过程
	
	/*创建启动任务*/
	xTaskCreate((TaskFunction_t) task1,//任务函数的地址
		 (char *) "task1",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK1_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK1_PRIORITY,//任务优先级
		 (TaskHandle_t *) &task1_TaskHandle//任务句柄的地址

		);

	xTaskCreate((TaskFunction_t) task2,//任务函数的地址
		 (char *) "task2",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK2_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK2_PRIORITY,//任务优先级
		 (TaskHandle_t *) &task2_TaskHandle//任务句柄的地址

		);
	xTaskCreate((TaskFunction_t) task3,//任务函数的地址
		 (char *) "task3",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK3_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK3_PRIORITY,//任务优先级
		 (TaskHandle_t *) &task3_TaskHandle//任务句柄的地址

		);
	
	/*启动任务执行一次，删除启动任务*/
	vTaskDelete(start_TaskHandle);

	taskEXIT_CRITICAL(); // 退出临界区，允许任务切换


}



void task1(void * pvParameters)
{

	char c[100];
	struct UART_Device *pUARTDev = GetUARTDevice("stm32_uart1");
    pUARTDev->Init(pUARTDev, 115200, 8, 'N', 1);



    

	while (1)
	{

		// char *send_str = "Hello";
		// pUARTDev->Send(pUARTDev, (uint8_t *)send_str, strlen(send_str), 100);

	
		while (0 != pUARTDev->Recv(pUARTDev, (uint8_t *)c, 100));
		
		
		pUARTDev->Send(pUARTDev, (uint8_t *)c,1, 1);
		vTaskDelay(10);

		
			
	}
}


void task2(void * pvParameters)
{
	// struct UART_Device *pUARTDev = GetUARTDevice("stm32_uart1");
	// pUARTDev->Init(pUARTDev, 115200, 8, 'N', 1);

	
	while (1)
	{
		// char *send_str = "World!\r\n";
		// pUARTDev->Send(pUARTDev, (uint8_t *)send_str, strlen(send_str), 100);

		//printf("任务2:执行...\r\n");
		vTaskDelay(10);

	}
}


void task3(void * pvParameters)
{


	while(1)
	{

		//printf("任务3:执行按键检测申请内存...\r\n");

		for(uint8_t i=0;i<KEY_COUNT;i++)
		{
			if(Key_Check(i,KEY_SINGLE))
			{
				if(i==0)
				{
					printf("按键%d被单击\r\n",i);

					

					

				}
				
			}

			if(Key_Check(i,KEY_DOUBLE))
			{
				if(i==0)
				{
					printf("按键%d被双击\r\n",i);
					
					
				}
				
			}
		}

		vTaskDelay(500);

	}

}


void timer1_callback(TimerHandle_t xTimer)
{
	static uint16_t count = 0;
	count++;
	printf("单次timer1回调执行！count=%d\r\n",count);
}

void timer2_callback(TimerHandle_t xTimer)
{
	static uint16_t count = 0;
	count++;
	printf("重复timer2回调执行！count=%d\r\n",count);
}
