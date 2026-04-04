#ifndef _UART_DEVICE_H
#define _UART_DEVICE_H


#ifdef __cplusplus
extern "C" {
#endif
    /*****************C*****************/
#include <stdint.h>

struct UART_Device {
    char *name;
    int (*Init)(struct UART_Device *pDev, int baud, int datas, char parity, int stop);
    int (*Send)(struct UART_Device *pDev, uint8_t *datas, int len, int timeout_ms);
    int (*Recv)(struct UART_Device *pDev, uint8_t *data, int timeout_ms);
    void *priv_data;
};

struct UART_Device *GetUARTDevice(char *name);



#ifdef __cplusplus
}
/**********************C++*************************/

#endif

#endif 
