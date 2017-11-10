//*****************************************************************************

#include "func.h"

#include "pinmux.h"

#include "sl_mqtt_client.h"

#include "serial.h"

#include "netcfg.h"


#define mac_len 6
#define TASK_PRIORITY           3
#define OSI_STACK_SIZE          2048

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
/*
void LedTimerConfigNStart()
{
    // Configure Timer for blinking the LED for IP acquisition
    Timer_IF_Init(PRCM_TIMERA0, TIMERA0_BASE, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_IntSetup(TIMERA0_BASE, TIMER_A, TimerPeriodicIntHandler);
    Timer_IF_Start(TIMERA0_BASE, TIMER_A, 100);
}
*/
//****************************************************************************
/*
void LedTimerDeinitStop()
{
    // Disable the LED blinking Timer as Device is connected to AP
    Timer_IF_Stop(TIMERA0_BASE, TIMER_A);
    Timer_IF_DeInit(TIMERA0_BASE, TIMER_A);
}
*/
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
//const char *TAG_LOOP = "LOOP";
s_evt evt;
//char stk[128];

	//GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);

    while (true) {

    	if (xQueueReceive(evtq, &evt, 0) == pdTRUE) {
    		/*
    		memset(stk, 0, 128);
    		if (!evt.type)
    			sprintf(stk,"Send");
    		else
    			sprintf(stk,"Recv");
    		sprintf(stk+strlen(stk)," pack #%u\r\n", evt.num);
    		pMessage(stk);
    		//printik(TAG_LOOP, stk, WHITE_COLOR);
    		 */
    	}
    	//vTaskDelay(50);

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
    	GPIO_IF_LedOn(MCU_RED_LED_GPIO);
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


/*
    osi_MsgQCreate(&g_PBQueue, "PBQueue", sizeof(event_msg), 5);
    lRetVal = osi_TaskCreate(MqttClient, (const signed char *)"Mqtt Client", OSI_STACK_SIZE, NULL, (1), NULL);// Start the MQTT Client task
    if (lRetVal < 0) {
    	Message("Error start mqtt_client task !!!\r\n");
        LOOP_FOREVER();
    } else {
    	Message("Start mqtt_client task OK\r\n");
    }
*/

    //****************    UART1 (LORA)    **************************
    osi_MsgQCreate(&evtq, "evtq", sizeof(s_evt), 5);//create a queue to handle uart event
//    s_evt evt;
    serial_init();
    lRetVal = osi_TaskCreate(serial_task, (const signed char *)"LoRa", OSI_STACK_SIZE, NULL, (2), NULL);// Start the LoRa task
    if (lRetVal < 0) {
    	Message("Error start serial task !!!\r\n");
        LOOP_FOREVER();
    } else {
    	Message("Start serial task OK\r\n");
    	GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);
    }
    //**************************************************************


    lRetVal = osi_TaskCreate(loop_task, (const signed char *)"Loop", OSI_STACK_SIZE, NULL, (2), NULL);// Start the MQTT Client task
    if (lRetVal < 0) {
    	Message("Error start loop task !!!\r\n");
        LOOP_FOREVER();
    } else {
    	Message("Start loop task OK\r\n");
    	GPIO_IF_LedOn(MCU_ORANGE_LED_GPIO);
    }

    osi_start();// Start the task scheduler

    while (1) { vTaskDelay(1000); };

}

