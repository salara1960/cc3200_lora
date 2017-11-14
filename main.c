#include "func.h"
#include "pinmux.h"
#include "sl_mqtt_client.h"
#include "serial.h"
#include "netcfg.h"


#define mac_len 6
#define TASK_PRIORITY           7
#define OSI_STACK_SIZE          2048

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
//****************************************************************************
//****************************************************************************
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
void loop_task(void *arg)
{
char stx[128];
uint8_t fl=0, i;
#ifdef DISPLAY
uint8_t col = 0, row = 0, cnt = 0xff;

    if (!i2c_err) ssd1306_on(true); //display on
    vTaskDelay(10);
    if (!i2c_err) ssd1306_contrast(cnt);
    vTaskDelay(10);
    if (!i2c_err) ssd1306_clear();
    vTaskDelay(10);

#endif

    GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    vTaskDelay(1000);
    GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    while (1) {
    	if (xQueueReceive(evtq, &evt, 0) == pdTRUE) {
    		sprintf(stx,"[%s] ", TAG_LOOP);
    		if (!evt.type) {
    			fl=1;
    			sprintf(stx,"Send");
#ifdef DISPLAY
    			row = 7;
#endif
    		} else {
    			fl=2;
    			sprintf(stx,"Recv");
#ifdef DISPLAY
    			row = 8;
#endif
    		}
    		sprintf(stx+strlen(stx)," pack #%u", evt.num);
#ifdef DISPLAY
    		col = calcx(strlen(stx));
    		if (!i2c_err) ssd1306_text_xy(stx, col, row);
#endif
    		sprintf(stx+strlen(stx),"\r\n");
    		pMessage(stx);
    	}
    	switch (fl) {
    		case 1:
    			for (i=0; i<8; i++) {
    			    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
    			    vTaskDelay(100);
    			    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    			    vTaskDelay(100);
    			    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);
    			}
    			fl=0;
    		break;
    		case 2:
    			for (i=0; i<8; i++) {
    				GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    				vTaskDelay(100);
    				GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    				vTaskDelay(100);
    				GPIO_IF_LedOff(MCU_ORANGE_LED_GPIO);
    			}
    			fl=0;
    		break;
    	}
    }

}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void main()
{
long lRetVal = -1;
char stk[64]={0}, stx[128]={0};
uint8_t i = 0, mac_addr_len = mac_len;
uint8_t mac_addr[mac_len];


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
    } else {
    	Message("Starting wifi task Ok\r\n");
    	//GPIO_IF_LedOn(MCU_RED_LED_GPIO);
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

    i2c_err = i2c_ssd1306_init();
    if (i2c_err) Report("I2C port open failure\r\n");
    else {
    	//vTaskDelay(10);
    	ssd1306_on(false);
    	if (!i2c_err) {
    		//vTaskDelay(10);
    		ssd1306_init();
    		if (!i2c_err) {
    			//vTaskDelay(10);
    	    	ssd1306_pattern();
    		}
    	}
    }
    //**************************************************************
#endif

    //**************************************************************
    lRetVal = osi_TaskCreate(loop_task, (const signed char *)"Loop", OSI_STACK_SIZE, NULL, TASK_PRIORITY, NULL);// Start the MQTT Client task
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
    serial_init();
    lRetVal = osi_TaskCreate(serial_task, (const signed char *)"LoRa", OSI_STACK_SIZE, NULL, TASK_PRIORITY, NULL);// Start the LoRa task
    if (lRetVal < 0) {
    	Message("Error start serial task !!!\r\n");
        LOOP_FOREVER();
    } else {
    	Message("Start serial task OK\r\n");
    	//GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    }
    //**************************************************************


    osi_start();// Start the task scheduler

    LOOP_FOREVER();

//    while (1) { vTaskDelay(100); }


}

