#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "func.h"
#include "at_cmd.h"
#include "pinmux.h"
#include "uart_if.h"

#define PRINT_AT

#define GPIO_0  0
#define GPIO_9  9
#define GPIO_30 30
#define GPIO_4  4
#define GPIO_22 22
#define GPIO_13 13

#define U2_CONFIG GPIO_0//PIN_50 //pin21 - out, pull down
#define U2_SLEEP  GPIO_9//PIN_64 //pin22 - out, pull down
#define U2_RESET  GPIO_30//PIN_53 //pin2 - out, pull up
#define U2_STATUS GPIO_4//PIN_59 //pin17 - in

#define U2_SW2 GPIO_22//PIN_15
#define U2_SW3 GPIO_13//PIN_4

#define mprintf Report

#define lora_buf_len 256

#pragma pack(push,1)
typedef struct
{
    unsigned config:1;
    unsigned sleep :1;
    unsigned status:1;
    unsigned reset :1;
    unsigned none  :4;
} s_pctrl;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct
{
    uint16_t rd;
    uint16_t wr;
    char buf[lora_buf_len];
} s_lora_buf;
#pragma pack(pop)

//extern SemaphoreHandle_t lora_mutex;

extern s_lora_buf rx_buf;

extern unsigned int ucPort;
extern unsigned char ucPin;
extern OsiMsgQ_t evtq;
extern uint32_t cli_id;
extern s_lora_stat lora_stat;
extern const char *TAG_UART;

extern void uart_lora_init();
extern void lora_task(void *arg);

#endif
