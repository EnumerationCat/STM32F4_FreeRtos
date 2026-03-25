#include "FreeRtos_demo.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

/*静态创建方式需要手动2个特殊任务的资源*/
// 如果使用静态内存分配（configSUPPORT_STATIC_ALLOCATION = 1）


/*空闲任务的配置*/
#define IDLE_TASK_STACK 128
StackType_t idle_TaskStack[IDLE_TASK_STACK];
StaticTask_t idle_TaskTCB;


/* 软件定时器任务配置 */
#define TIMER_TASK_STACK 128
StackType_t  timer_TaskStack[TIMER_TASK_STACK];
StaticTask_t timer_TaskTCB;



#if (configSUPPORT_STATIC_ALLOCATION == 1)
// 分配空闲任务内存
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    
    
    *ppxIdleTaskTCBBuffer = &idle_TaskTCB;
    *ppxIdleTaskStackBuffer = idle_TaskStack;
    *pulIdleTaskStackSize = IDLE_TASK_STACK;
}

// 分配定时器任务内存
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
    
	
    *ppxTimerTaskTCBBuffer = &timer_TaskTCB;
    *ppxTimerTaskStackBuffer = timer_TaskStack;
    *pulTimerTaskStackSize = TIMER_TASK_STACK;
}
#endif

// 内存分配失败钩子函数
#if (configUSE_MALLOC_FAILED_HOOK == 1)
void vApplicationMallocFailedHook( void )
{
    // 内存分配失败时的处理
    // 可以在这里添加日志、复位系统或进入错误状态
    taskDISABLE_INTERRUPTS();
    for( ;; );
}
#endif
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
		usbprintf("Stack overflow in task: %s\r\n", pcTaskName);

	}
}



uint8_t  myUsbRxData[64] = { 0 };   // 接收到的数据
uint16_t myUsbRxNum = 0;            // 接收到的字节数

void usbRecData(void) {
    // 判断接收
    if (myUsbRxNum)
    {
        static char myStr[200] = { 0 };
        sprintf(myStr, "\r\n收到 %d 个字节\r\n内容是: %s\r\n", myUsbRxNum, (char *)myUsbRxData);
        CDC_Transmit_FS((uint8_t *)myStr, strlen(myStr));
        myUsbRxNum = 0;
    }
}

// 定义缓冲区大小（可根据需求调整，建议256/512字节）
#define USB_PRINTF_BUF_SIZE 256

/**
 * @brief  自定义USB printf函数，支持格式化输出到USB CDC串口
 * @param  format: 格式化字符串（同printf）
 * @param  ...: 可变参数列表
 * @retval 发送的字节数（成功）/ -1（缓冲区溢出）/ -2（发送失败）
 */
int usbprintf(const char *format, ...)
{
    // 静态缓冲区：避免函数栈溢出，且仅初始化一次
    static uint8_t usb_buf[USB_PRINTF_BUF_SIZE];
    va_list args;
    int len = 0;
    uint8_t send_status = 0;

    //清空缓冲区（可选，防止脏数据）
    memset(usb_buf, 0, USB_PRINTF_BUF_SIZE);

    //初始化可变参数列表
    va_start(args, format);

    //将格式化内容写入缓冲区（vsnprintf自动处理格式化，返回实际需要的字节数）
    // 第三个参数：缓冲区大小-1，预留'\0'结束符
    len = vsnprintf((char *)usb_buf, USB_PRINTF_BUF_SIZE - 1, format, args);

    //结束可变参数列表
    va_end(args);

    // 检查格式化结果是否溢出
    if (len < 0 || len >= USB_PRINTF_BUF_SIZE)
    {
        // 缓冲区不足，返回溢出错误
        return -1;
    }

    //调用CDC_Transmit_FS发送数据（len为实际有效字节数）
    send_status = CDC_Transmit_FS(usb_buf, len);

    //检查发送状态（CDC_Transmit_FS返回USBD_OK(0)表示成功）
    if (send_status != USBD_OK)
    {
        // 发送失败
        return -2;
    }

    //返回实际发送的字节数
    return len;
}





/*启动任务的配置*/
#define START_TASK_STACK 128
#define START_TASK_PRIORITY 1
TaskHandle_t start_TaskHandle;
StackType_t start_TaskStack[START_TASK_STACK];
StaticTask_t start_TaskTCB;
void start_Task(void * pvParameters);



/*任务1的配置*/
#define TASK1_STACK 128
#define TASK1_PRIORITY 2
TaskHandle_t task1_TaskHandle;
StackType_t task1_TaskStack[TASK1_STACK];
StaticTask_t task1_TaskTCB;
void task1(void * pvParameters);

/*任务2的配置*/
#define TASK2_STACK 128
#define TASK2_PRIORITY 3
TaskHandle_t task2_TaskHandle;
StackType_t task2_TaskStack[TASK2_STACK];
StaticTask_t task2_TaskTCB;
void task2(void * pvParameters);

/*任务3的配置*/
#define TASK3_STACK 128
#define TASK3_PRIORITY 4
TaskHandle_t task3_TaskHandle;
StackType_t task3_TaskStack[TASK3_STACK];
StaticTask_t task3_TaskTCB;
void task3(void * pvParameters);








/**
 * @brief  FreeRTOS启动函数，创建启动任务并启动调度器

 */

void freertos_start(void)
{
	/*创建启动任务（使用静态分配）*/
	start_TaskHandle = xTaskCreateStatic((TaskFunction_t) start_Task,//任务函数的地址
		(char *) "start_Task",//任务名字
		(configSTACK_DEPTH_TYPE) START_TASK_STACK,//任务堆栈大小
		(void *) NULL,//传递给任务函数的参数
		(UBaseType_t) START_TASK_PRIORITY,//任务优先级
		(StackType_t*) start_TaskStack,    //任务地址
		(StaticTask_t *)&start_TaskTCB);//静态创建任务TCB地址

	

	/*启动调度器:自动创建空闲任务和软件定时器*/
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
	task1_TaskHandle = xTaskCreateStatic((TaskFunction_t) task1,//任务函数的地址
		 (char *) "task1",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK1_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK1_PRIORITY,//任务优先级
		 (StackType_t*) task1_TaskStack,    //任务地址
		 (StaticTask_t *)&task1_TaskTCB//静态创建任务TCB地址
		);

	task2_TaskHandle = xTaskCreateStatic((TaskFunction_t) task2,//任务函数的地址
		 (char *) "task2",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK2_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK2_PRIORITY,//任务优先级
		 (StackType_t*) task2_TaskStack,    //任务地址
		 (StaticTask_t *)&task2_TaskTCB//静态创建任务TCB地址

		);

	task3_TaskHandle = xTaskCreateStatic((TaskFunction_t) task3,//任务函数的地址
		 (char *) "task3",//任务名字
		 (configSTACK_DEPTH_TYPE) TASK3_STACK,//任务堆栈大小
		 (void *) NULL,//传递给任务函数的参数
		 (UBaseType_t) TASK3_PRIORITY,//任务优先级
		 (StackType_t*) task3_TaskStack,    //任务地址
		 (StaticTask_t *)&task3_TaskTCB//静态创建任务TCB地址
		);

	/*启动任务执行一次，删除启动任务*/
	vTaskDelete(start_TaskHandle);

	taskEXIT_CRITICAL(); // 退出临界区，允许任务切换


}


void task1(void * pvParameters)
{
	while (1)
	{
		usbprintf("任务1:按键扫描\r\n");
		Key_Scanf();
		vTaskDelay(500);
	}
}

void task2(void * pvParameters)
{
	while (1)
	{
		usbprintf("任务2\r\n");
		
		vTaskDelay(500 );
	}
}

void task3(void * pvParameters)
{
	while (1)
	{
		usbprintf("任务3:删除任务3\r\n");
		for(uint8_t i=0;i<KEY_COUNT;i++)
			{
				if(Key_Check(i,KEY_SINGLE))
				{
					if(i==0)
					{
						if(task3_TaskHandle!=NULL)
						{
							vTaskDelete(task3_TaskHandle);
							task3_TaskHandle = NULL; // 删除任务后将句柄置空，避免误操作	
							usbprintf("删除任务3\r\n");

						}
						
						
					}
					
				}
			}
		vTaskDelay(500);
	}
}
