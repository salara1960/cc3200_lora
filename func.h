#ifndef __FUNC_H__
#define __FUNC_H__

#include <stdlib.h>
#include <hw_types.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "osi.h"

// simplelink includes
#include "simplelink.h"

// driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_adc.h"
#include "interrupt.h"
#include "rom_map.h"
#include "prcm.h"
#include "uart.h"
#include "timer.h"
#include "pin.h"
#include "adc.h"

// common interface includes
#include "network_if.h"
#include "uart_if.h"
#include "i2c_if.h"

#include "gpio_if.h"
#include "timer_if.h"
#include "common.h"
#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


#include "hdr.h"

#ifdef DISPLAY
	#include "ssd1306.h"
#endif

#define UART_BAUD_RATE  115200
#define SYSCLK          80000000

#define MS_TO_TICK(ms)   ((SYSCLK/1000) * (ms))
#define TICK_TO_MS(ts)   (((double) 1000/(double) SYSCLK) * (double) (ts))

#define LORA             	UARTA1_BASE
#define LORA_PERIPH      	PRCM_UARTA1
#define LORAUartGetChar()	MAP_UARTCharGet(LORA)
#define LORAUartPutChar(c)	MAP_UARTCharPut(LORA,c)

#define UART_IF_BUFFER           256 //64

#define BLACK_COLOR  "\x1B[30m"
#define RED_COLOR  "\x1B[31m"
#define GREEN_COLOR  "\x1B[32m"
#define BROWN_COLOR  "\x1B[33m"
#define BLUE_COLOR  "\x1B[34m"
#define MAGENTA_COLOR  "\x1B[35m"
#define CYAN_COLOR  "\x1B[36m"
#define WHITE_COLOR "\x1B[0m"
#define START_COLOR CYAN_COLOR
#define STOP_COLOR  WHITE_COLOR

#define UART_PRINT Report

#define get_tmr(tm) (xTaskGetTickCount() + tm)
#define check_tmr(tm) (xTaskGetTickCount() >= tm ? true : false)

#pragma pack(push,1)
typedef struct {
    uint32_t num;
    uint8_t type;
} s_evt;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    double faren;
	double kelv;
    double cels;
    uint32_t vcc;
} t_sens_t;
#pragma pack(pop)


//typedef unsigned char uint8_t;



//--------------------------------------------------------------------
extern uint32_t cli_id;
extern OsiMsgQ_t evtq;
extern uint32_t ThePin, TheChan;
extern int i2c_err;
//--------------------------------------------------------------------
extern void pMessage(const char *st);
extern void SetDevTime(time_t *tim);
extern void printik(const char *tag, const char *buf, const char *color);

extern void InitLora();
extern void LoraTxBuf(char *st);
extern unsigned char LoraRxByte(unsigned char *byte);

extern void init_adc(uint32_t pin);
extern uint32_t GetSampleADC(uint32_t chan, bool prn);

extern void get_tsensor(t_sens_t *t_s);
#ifdef TMP006
	extern char *str_sensor(t_sens_t *t_s);
#endif

#endif
