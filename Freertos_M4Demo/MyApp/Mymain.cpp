#include "MyApplication.h"
#include "FreeRTOS.h"
#include "task.h"


#include <string.h>
#include <stdio.h>
#include <stdarg.h>       // 可变参数处理头文件
#include <stdlib.h>

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
    for( ;; );
}





uint8_t  myUsbRxData[64] = { 0 };   // 接收到的数据
uint16_t myUsbRxNum = 0;            // 接收到的字节数

void usbRecData(void) {
    // 判断接收
    if (myUsbRxNum)
    {
        static char myStr[100] = { 0 };
        sprintf(myStr, "\r\n收到 %d 个字节: \r内容是: %s\r\n", myUsbRxNum, (char *)myUsbRxData);
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

    // 1. 清空缓冲区（可选，防止脏数据）
    memset(usb_buf, 0, USB_PRINTF_BUF_SIZE);

    // 2. 初始化可变参数列表
    va_start(args, format);

    // 3. 将格式化内容写入缓冲区（vsnprintf自动处理格式化，返回实际需要的字节数）
    // 第三个参数：缓冲区大小-1，预留'\0'结束符
    len = vsnprintf((char *)usb_buf, USB_PRINTF_BUF_SIZE - 1, format, args);

    // 4. 结束可变参数列表
    va_end(args);

    // 5. 检查格式化结果是否溢出
    if (len < 0 || len >= USB_PRINTF_BUF_SIZE)
    {
        // 缓冲区不足，返回溢出错误
        return -1;
    }

    // 6. 调用CDC_Transmit_FS发送数据（len为实际有效字节数）
    send_status = CDC_Transmit_FS(usb_buf, len);

    // 7. 检查发送状态（CDC_Transmit_FS返回USBD_OK(0)表示成功）
    if (send_status != USBD_OK)
    {
        // 发送失败
        return -2;
    }

    // 8. 返回实际发送的字节数
    return len;
}

// ------------------- 使用示例 -------------------
void test_usbprintf(void)
{
    // 测试字符串输出
    usbprintf("Hello USB CDC!\r\n");

    // 测试数字、格式化输出
    int num = 123;
    float f_num = 3.14159f;
    usbprintf("整数：%d，十六进制：0x%04X，浮点数：%.2f\r\n", num, num, f_num);

    // 测试字符输出
    char ch = 'A';
    usbprintf("字符：%c，字符串长度：%d\r\n", ch, strlen("Test String"));
}



void Mymain() {



    for (;;)
    {
        HAL_Delay(500);
        test_usbprintf();

        usbRecData() ;





    }
}