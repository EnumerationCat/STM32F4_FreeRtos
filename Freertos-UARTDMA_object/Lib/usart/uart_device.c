#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_usart.h"
#include "usart.h"
#include "dma.h"
#include "uart_device.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "string.h"
#include <stdint.h>
#include <string.h>


/*要把串口中断的优先级设在FreeRtos的终端管理范围内*/


#define UART_DMA_MODE 1
#define UART_INTERRUPT_MODE 0
#define UART_DELAY_MODE 0

#if ( (UART_INTERRUPT_MODE + UART_DMA_MODE + UART_DELAY_MODE) != 1 )
    #error "just one mode can be selected"
#endif


#define UART_BUFFER_SIZE 512
#define UART_RX_QUEUE_LEN 512




//extern UART_HandleTypeDef huart1;


static struct UART_Device g_stm32_uart1; 


struct UART_Data {

    UART_HandleTypeDef *handle;
    SemaphoreHandle_t xTxSem; // 发送完成信号量
	SemaphoreHandle_t xTxMutex;//发送互斥锁
    QueueHandle_t xRxQueue;
    uint8_t rxdata[UART_BUFFER_SIZE];
	int read_index;

}; 










#if UART_INTERRUPT_MODE||UART_DMA_MODE

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	struct UART_Data *uart_data = g_stm32_uart1.priv_data;
    if (huart == &huart1)
    {
		#if UART_INTERRUPT_MODE
        HAL_UART_Receive_IT(uart_data->handle, uart_data->rxdata, 1); // 重启接收
		#endif

		#if UART_DMA_MODE
		HAL_UARTEx_ReceiveToIdle_DMA(uart_data->handle, uart_data->rxdata, UART_BUFFER_SIZE); // 重启DMA接收
		#endif

    }
}


/*发送完成回调函数*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct UART_Data *uart_data;

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;//上下文切换标志
    
    if (huart == &huart1)
    {
        uart_data = g_stm32_uart1.priv_data;
        
        /* 释放信号量 */
        xSemaphoreGiveFromISR(uart_data->xTxSem, &xHigherPriorityTaskWoken);

		/* 触发上下文切换（确保高优先级任务及时运行） */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 触发切换
    }
}


/*接收完成回调函数*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct UART_Data *uart_data;

	int rxdata_len = huart->RxXferSize - huart->RxXferCount; // 计算接收数据长度

	BaseType_t xHigherPriorityTaskWoken = pdFALSE; //上下文切换标志
    
    if (huart == &huart1)
    {
        uart_data = g_stm32_uart1.priv_data;

		
        
		for(int i=0; i<rxdata_len; i++)
		{
			/* 写队列 */
         	xQueueSendFromISR(uart_data->xRxQueue, &uart_data->rxdata[i], &xHigherPriorityTaskWoken);

		}
       

		 #if UART_INTERRUPT_MODE
        /* 再次启动数据的接收 */
        HAL_UART_Receive_IT(uart_data->handle, uart_data->rxdata, huart->RxXferSize);
		#endif

		#if UART_DMA_MODE
		/* 再次启动DMA接收 */
		HAL_UARTEx_ReceiveToIdle_DMA(uart_data->handle, uart_data->rxdata, huart->RxXferSize);
		#endif

		/* 触发上下文切换（确保高优先级任务及时运行） */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
#endif


#if UART_DMA_MODE

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    struct UART_Data *uart_data;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE; //上下文切换标志
    int rxdata_len = Size;

    int read_index;
    if (huart == &huart1)
    {
        uart_data = g_stm32_uart1.priv_data;
        
        /* 写队列 */
        for (  read_index = uart_data->read_index; read_index< rxdata_len; read_index++)
        {
            xQueueSendFromISR(uart_data->xRxQueue, &uart_data->rxdata[read_index],&xHigherPriorityTaskWoken);
        }

		uart_data->read_index = read_index;



		/*下面两种方式哪个好用用哪个*/

		/*循环模式DMA，硬件开启*/
		// if(huart->RxEventType == HAL_UART_RXEVENT_TC)
		// {
		// 	uart_data->read_index = 0; // 重置读索引

		// }

		/*正常模式DMA，软件开启*/
		if(huart->RxEventType == HAL_UART_RXEVENT_IDLE  || huart->RxEventType == HAL_UART_RXEVENT_TC)
		{
			 /* 再次启动数据的接收 */
        	HAL_UARTEx_ReceiveToIdle_DMA(uart_data->handle, uart_data->rxdata, huart->RxXferSize);
			uart_data->read_index = 0; // 重置读索引

		}
       
		

		/* 触发上下文切换（确保高优先级任务及时运行） */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#endif





static int stm32_uart_init(struct UART_Device *pDev, int baud, int datas, char parity, int stop)
{

	/******************UserInit************************/


	/**************************************************/

	
	struct UART_Data *uart_data = pDev->priv_data;
   
	uart_data->xTxMutex = xSemaphoreCreateMutex();

	#if UART_INTERRUPT_MODE||UART_DMA_MODE
	uart_data->xTxSem = xSemaphoreCreateBinary();
    uart_data->xRxQueue = xQueueCreate(UART_RX_QUEUE_LEN, 1);
	#if UART_INTERRUPT_MODE

    /* 启动第1次数据的接收 */
    HAL_UART_Receive_IT(uart_data->handle, uart_data->rxdata, 1);
	#endif

	#if UART_DMA_MODE
	/* 启动DMA接收 */	
	HAL_UARTEx_ReceiveToIdle_DMA(uart_data->handle, uart_data->rxdata, UART_BUFFER_SIZE);
	#endif

	#endif


    return 0;
}

static int stm32_uart_send(struct UART_Device *pDev, uint8_t *datas, int len, int timeout_ms)
{
    struct UART_Data *uart_data = pDev->priv_data;


	//#if UART_INTERRUPT_MODE||UART_DMA_MODE
	
    // 加互斥锁，避免多任务同时发送
    if (pdTRUE != xSemaphoreTake(uart_data->xTxMutex, timeout_ms)) {
        return -1;
    }
	//#endif


    #if UART_INTERRUPT_MODE
    /* 仅仅是触发中断而已 */
    HAL_UART_Transmit_IT(uart_data->handle, datas, len);
	#endif

	#if UART_DMA_MODE
	/* 触发DMA发送 */
	HAL_UART_Transmit_DMA(uart_data->handle, datas, len);
	#endif


	#if UART_DELAY_MODE
	if(HAL_UART_Transmit(uart_data->handle, datas, len, timeout_ms) == HAL_OK)
	{
		xSemaphoreGive(uart_data->xTxMutex); // 释放互斥锁
		return 0;
	}else{
		xSemaphoreGive(uart_data->xTxMutex); // 释放互斥锁
		return -1;
	}
	#endif





	#if UART_INTERRUPT_MODE||UART_DMA_MODE
    /* 等待发送完毕:等待信号量 */
    if (pdTRUE == xSemaphoreTake(uart_data->xTxSem, timeout_ms))
	{
		xSemaphoreGive(uart_data->xTxMutex); // 释放互斥锁
		return 0;
	}else{
		xSemaphoreGive(uart_data->xTxMutex); // 释放互斥锁
		return -1;
	}
	#endif
        
}

static int stm32_uart_recv(struct UART_Device *pDev, uint8_t *data, int timeout_ms)
{

 	struct UART_Data *uart_data = pDev->priv_data;
	#if UART_INTERRUPT_MODE||UART_DMA_MODE
   
    /* 读取队列得到数据, 问题:谁写队列?中断:写队列 */
    if (pdPASS == xQueueReceive(uart_data->xRxQueue, data,timeout_ms))
	{
		return 0;
	}else{
		return -1;
	}
	#endif

	#if UART_DELAY_MODE
	 if(HAL_UART_Receive(uart_data->handle, data, 1, timeout_ms) == HAL_OK)
	 {
		 return 0;
	 }else{
		 return -1;
	 }
	#endif
}



static struct UART_Data g_stm32_uart1_data = {
    &huart1,
	NULL,
	NULL,
	NULL,
	{0},
	0
	
};

static struct UART_Device g_stm32_uart1 = {
    "stm32_uart1",
    stm32_uart_init,
    stm32_uart_send,
    stm32_uart_recv,
    &g_stm32_uart1_data
};



static struct UART_Device *g_uart_devs[] = {
	&g_stm32_uart1

};


struct UART_Device *GetUARTDevice(char *name)
{
    
    for (uint8_t i = 0; i < sizeof(g_uart_devs)/sizeof(g_uart_devs[0]); i++)
    {
        if (0 == strcmp(name, g_uart_devs[i]->name))
            return g_uart_devs[i];
    }

    return NULL;
}


