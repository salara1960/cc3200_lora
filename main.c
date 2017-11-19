#include "func.h"
#include "pinmux.h"
#include "serial.h"
#include "netcfg.h"
#include "tmp006.h"


#define mac_len 6
#define TASK_PRIORITY           5
#define MAX_OSI_STACK_SIZE          4096
#define MIN_OSI_STACK_SIZE          2048

const char *TAG_LOOP = "LOOP";
s_evt evt;

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void BoardInit(void);
//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#ifdef USE_FREERTOS
    #if defined(ewarm)
	extern uVectorEntry __vector_table;
    #endif
    #if defined(ccs)
	extern void (* const g_pfnVectors[])(void);
    #endif
#endif
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
    #if defined(ccs)
        IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
    #endif
    #if defined(ewarm)
        IntVTableBaseSet((unsigned long)&__vector_table);
    #endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//*****************************************************************************
void i2c_task(void *arg)
{
char stx[128];
#ifdef DISPLAY
	#ifdef TMP006
		t_sens_t ts;
		char *st=NULL;
	#endif
	uint8_t col = 0, row = 0;
	char *uk=NULL;

    ssd1306_contrast(0xff);
    osi_Sleep(10);
    ssd1306_clear();
#endif


    while (1) {
    	if (xQueueReceive(evtq, &evt, 10) == pdTRUE) {
    		memset(stx,0,128);
    		sprintf(stx,"[%s] ", TAG_LOOP);
#ifdef DISPLAY
    		uk = stx + strlen(stx);
#endif
    		if (!evt.type) {
    			GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    			sprintf(stx+strlen(stx),"Send");
#ifdef DISPLAY
    			row = 7;
#endif
    		} else {
    			GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    			sprintf(stx+strlen(stx),"Recv");
#ifdef DISPLAY
    			row = 8;
#endif
    		}
    		sprintf(stx+strlen(stx)," msg #%u", evt.num);
#ifdef DISPLAY
    		if (!i2c_err) {
#ifdef TMP006
    			get_tsensor(&ts);
    			st = str_sensor(&ts);
    			if (st) {
    				ssd1306_text(st);
    				free(st);
    			}
#endif
    			col = calcx(strlen(uk));
    			ssd1306_text_xy(uk, col, row);
    		}
#endif
    		sprintf(stx+strlen(stx),"\r\n");
    		pMessage(stx);
    		//printik(TAG_LOOP, stx, CYAN_COLOR);
    	}
    }//while(1)

}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void main()
{
long lRetVal = -1;
char stk[64]={0}, stx[128]={0};
uint8_t i = 0, mac_addr_len = mac_len, mac_addr[mac_len];


    BoardInit();// Initialize the board configurations

    PinMuxConfig();// Pinmux for UART

    InitTerm();// Configuring UART

    GPIO_IF_LedConfigure(LED1 | LED2 | LED3);// Configure LED
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);


    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);// Start the SimpleLink Host
    if (lRetVal < 0) {
    	Message("Error starting wifi task !!!\r\n");
        LOOP_FOREVER();
    }

    //memset(mac_addr, 0, mac_addr_len);
    //sl_NetCfgGet(SL_MAC_ADDRESS_GET, NULL, &mac_addr_len, mac_addr);
    for (i = 0; i < mac_addr_len; i++) {
    	mac_addr[i] = i + 1;
    	if (i<5) sprintf(stk+strlen(stk),"%02X:", mac_addr[i]);
	    	else sprintf(stk+strlen(stk),"%02X", mac_addr[i]);
    }
    memcpy(&cli_id, &mac_addr[2], 4);
    cli_id = ntohl(cli_id);
    sprintf(stx,"Mac address %s DevID=%08X\r\n", stk, cli_id);
    Message(stx);

    init_adc(ThePin);

#ifdef DISPLAY
    //*********************    SSD1306    **************************

    i2c_err = i2c_port_master_init();
    if (i2c_err) Report("I2C port open failure\r\n");
    else {
    	ssd1306_on(false);
    	if (!i2c_err) {
    		ssd1306_init();
    		if (!i2c_err) ssd1306_pattern();
    	}
    }

#ifdef TMP006
    config_TMP006(TMP006_ADDR, TMP006_CFG_8SAMPLE);
#endif

    //**************************************************************
#endif

    //**************************************************************
    lRetVal = osi_TaskCreate(i2c_task, (const signed char *)"i2c_task", MIN_OSI_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    if (lRetVal < 0) {
    	Message("Error start loop task !!!\r\n");
        LOOP_FOREVER();
    } else {
        Message("Start loop task OK\r\n");
        //GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    }
    //**************************************************************



    //****************    UART1 (LORA)    **************************
    osi_MsgQCreate(&evtq, "evtq", sizeof(s_evt), 5);//create a queue to handle uart event
//    lora_mutex = xSemaphoreCreateMutex();
    uart_lora_init();
    lRetVal = osi_TaskCreate(lora_task, (const signed char *)"lora_task", MAX_OSI_STACK_SIZE, NULL, TASK_PRIORITY, NULL);
    if (lRetVal < 0) {
    	Message("Error start serial task !!!\r\n");
        LOOP_FOREVER();
    } else {
    	Message("Start serial task OK\r\n");
    	GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    }
    //**************************************************************


    osi_start();// Start the task scheduler

    LOOP_FOREVER();

//    while (1) { vTaskDelay(100); }


}

