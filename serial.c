#include "serial.h"


unsigned int ucPort;
unsigned char ucPin;


const char *TAG_UART = "UART";
const int BSIZE = 128;
uint32_t wait_ack;//2000
s_pctrl pctrl = {0, 0, 1, 1, 0};
bool ts_set = false;
static uint8_t allcmd = 0;
static uint32_t pknum_tx = 0;
static uint32_t pknum_rx = 0;
static bool mode = false;//at_command

s_lora_buf rxd;
static volatile unsigned long TBase = TIMERA0_BASE;

//******************************************************************************************
static void LoraHandler()
{
uint8_t bt;

    while (MAP_UARTCharsAvail(LORA)) {
    	bt = MAP_UARTCharGetNonBlocking(LORA);
    	if ( (bt >= 0x0a) && (bt <= 0x7f) ) {
    		rxd.buf[rxd.wr++] = bt;
    		if (rxd.wr >= lora_buf_len) rxd.wr = 0;
    		if (rxd.rd == rxd.wr) {
    			rxd.rdy = 0;
    			rxd.rd = rxd.wr = 0;
    		} else rxd.rdy = 1;
    	}
    }

    Timer_IF_InterruptClear(TBase);// Clear the timer interrupt.
}
//-----------------------------------------------------------------------------------------
void uart_lora_init()
{
    InitLora();

    while (MAP_UARTCharsAvail(LORA)) MAP_UARTCharGetNonBlocking(LORA);

    memset(&rxd, 0, sizeof(s_lora_buf));

    // Configuring the timer
    Timer_IF_Init(PRCM_TIMERA0, TBase, TIMER_CFG_PERIODIC, TIMER_A, 0);
    // Setup the interrupts for the timer timeout
    Timer_IF_IntSetup(TBase, TIMER_A, LoraHandler);
    // Start timer
    Timer_IF_Start(TBase, TIMER_A, 1);//timeout = 1ms

}
//-----------------------------------------------------------------------------------------
#ifndef ESP32
void gpio_set_level(int pin, bool val)
{
	GPIO_IF_GetPortNPin(pin, &ucPort, &ucPin);
    GPIO_IF_Set(pin, ucPort, ucPin, val);

}
//-----------------------------------------------------------------------------------------
bool gpio_get_level(int pin)
{
	GPIO_IF_GetPortNPin(pin, &ucPort, &ucPin);
	return GPIO_IF_Get(pin, ucPort, ucPin);
}
#endif
//-----------------------------------------------------------------------------------------
void lora_init()
{
    gpio_set_level(U2_CONFIG, (bool)pctrl.config);//0 //set configure mode - at_command
    gpio_set_level(U2_SLEEP, (bool)pctrl.sleep);//0 //set no sleep
    pctrl.status = gpio_get_level(U2_STATUS);
    gpio_set_level(U2_RESET, (bool)pctrl.reset);//1 //set no reset
}
//-----------------------------------------------------------------------------------------
void lora_reset()
{
    if (!(pctrl.status = gpio_get_level(U2_STATUS))) vTaskDelay(1000);//wait status=high (20 ms)

    pctrl.reset = 0; gpio_set_level(U2_RESET, pctrl.reset);//set reset
    vTaskDelay(1000);
    pctrl.reset = 1; gpio_set_level(U2_RESET, pctrl.reset);//set no reset
}
//-----------------------------------------------------------------------------------------
void lora_data_mode(bool cnf)//true - data mode, false - at_command mode
{
    if (cnf) pctrl.config = 1; else pctrl.config = 0;
    gpio_set_level(U2_CONFIG, pctrl.config);//0 - configure mode (at_command), 1 - data transfer mode
}
//-----------------------------------------------------------------------------------------
bool lora_at_mode()//true - at_command mode, false - data mode
{
    return (pctrl.config);
}
//-----------------------------------------------------------------------------------------
bool lora_sleep_mode(bool slp)//true - sleep mode, false - normal mode
{
    if (slp) {
    	if (pctrl.sleep) return (bool)pctrl.sleep;//already sleep mode
    	pctrl.sleep = 1;
    } else {
    	if (!pctrl.sleep) return (bool)pctrl.sleep;//already normal mode
    	pctrl.sleep = 0;
    }
    gpio_set_level(U2_SLEEP, pctrl.sleep);

    if (!pctrl.sleep) {
    	uint8_t sch = 12;
    	while ( sch-- && !(pctrl.status = gpio_get_level(U2_STATUS)) ) vTaskDelay(100);
    }

    return (bool)pctrl.sleep;
}
//-----------------------------------------------------------------------------------------
bool lora_check_sleep()
{
    return (bool)pctrl.sleep;
}
//-----------------------------------------------------------------------------------------
bool lora_check_status()
{
    pctrl.status = gpio_get_level(U2_STATUS);

    return (bool)pctrl.status;
}
//-----------------------------------------------------------------------------------------
inline void get_tsensor(t_sens_t *t_s)
{
    t_s->vcc = GetSampleADC(TheChan, false);//4095 * 0.8;//(uint32_t)(adc1_get_raw(ADC1_TEST_CHANNEL) * 0.8);
    t_s->faren = 78;//temprature_sens_read();// - 40;
    t_s->cels = (t_s->faren - 32) * 5/9;
}
//-----------------------------------------------------------------------------------------
void lora_task(void *arg)
{
char stx[256];
char *uks = NULL, *uke = NULL;


	wait_ack = 120000;

    sprintf(stx, "Start serial_task\r\n"); printik(TAG_UART, stx, CYAN_COLOR);


    char *data = (char *)calloc(1, BSIZE + 1);
    if (data) {
    	if (!pknum_tx) {
    		lora_init();
    		vTaskDelay(500);
    	}
    	lora_reset();
    	vTaskDelay(500);

    	char cmds[BSIZE], tmp[32] = {0};
    	uint32_t len = 0;
    	uint8_t lvl = 0;
    	bool needs = false;
    	TickType_t tms = 0, tmneeds = 0, wtt_start=0, wtt_stop=0, dur=0;
    	t_sens_t tchip;
    	s_evt evt;
    	memset(&lora_stat, 0, sizeof(s_lora_stat));
    	lora_stat.bandw=6;



    	while (true) {

    		while (allcmd < TotalCmd) {//at command loop //21

    			if (!allcmd) {
    				sprintf(stx, "%s[%s] Wait init lora module...%s\r\n", GREEN_COLOR, TAG_UART, STOP_COLOR);
    				pMessage(stx);
    			}

    			memset(cmds, 0, BSIZE);
    			sprintf(cmds, "%s", at_cmd[allcmd].cmd);

    			if (!strcmp(cmds, "AT+LRSF=")) {//Spreading Factor
    				lora_stat.sf = 11;//12
    				sprintf(cmds+strlen(cmds),"%X", lora_stat.sf);//7—SF=7, 8—SF=8, 9—SF=9, A—SF=10, B—SF=11, C—SF=12
    			} else if (!strcmp(cmds, "AT+LRPL=")) {// PackLen
    				lora_stat.plen = 80;
    				sprintf(cmds+strlen(cmds), "%d", lora_stat.plen);//1..127
    			} else if (!strcmp(cmds, "AT+CS=")) {// Channel
    				lora_stat.chan = 10;
    				sprintf(cmds+strlen(cmds), "%X", lora_stat.chan);//B//set Channel Select to 10 //0..F — 0..15 channel
    			} else if (strchr(cmds,'=')) sprintf(cmds+strlen(cmds),"?");

    			sprintf(cmds+strlen(cmds),"\r\n");
#ifdef PRINT_AT
    			sprintf(stx,"%s%s%s", BROWN_COLOR, cmds, STOP_COLOR);
    			pMessage(stx);
#endif
    			LoraTxBuf(cmds);
    			tms = get_tmr(at_cmd[allcmd].wait);
    			len = 0; memset(data, 0, BSIZE);
    			while (!check_tmr(tms)) {
    				if ((rxd.rd != rxd.wr) && rxd.rdy) {
    					if (rxd.rd >= lora_buf_len) rxd.rd = 0;
    					data[len++] = rxd.buf[rxd.rd++];
    					if ( (strstr(data, "\r\n")) || (len >= BSIZE - 2) ) {
    						if (strstr(data, "ERROR:")) {
    							sprintf(stx,"%s%s%s", RED_COLOR, data, STOP_COLOR);
    							pMessage(stx);
    						} else {
#ifdef PRINT_AT
    							sprintf(stx,"%s", data);
    							pMessage(stx);
#endif
    							if (data[0] == '+') put_at_value(allcmd, data);
    						}
    						break;
    					}
    				}

    			}
    			allcmd++;
    			vTaskDelay(25);//50//200
    		}//while (allcmd < TotalCmd)

    		if (!mode) {//at_command mode
    			if (lora_stat.plen) {
    				memset(stx,0,256);
    				sprintf(stx,"%s[%s] Freq=%s Mode=%s Hopping=%s Power=%s Channel=%u BandW=%s SF=%u PackLen=%u%s\r\n",
						GREEN_COLOR, TAG_UART,
						lora_freq[lora_stat.freq],
						lora_main_mode[lora_stat.mode],
						lora_hopping[lora_stat.hfss],
						lora_power[lora_stat.power],
						lora_stat.chan,
						lora_bandw[lora_stat.bandw - 6],
						lora_stat.sf,
						lora_stat.plen,
						STOP_COLOR);
    				pMessage(stx);
    			}
    			mode = true;
    			lora_data_mode(mode);//set data mode
    			vTaskDelay(150);
    			len = 0; memset(data, 0, BSIZE);
    			memset(stx,0,256);
    			sprintf(stx, "%s[%s] Device %X switch from at_command to data tx/rx mode%s\r\n", MAGENTA_COLOR, TAG_UART, cli_id, STOP_COLOR);
    			pMessage(stx);
    			needs = false;
    		} else {//data transfer mode
    			if (!needs) {
    				get_tsensor(&tchip);
    				memset(cmds, 0, BSIZE);
    				if (ts_set) {// time already set, send message without request timestamp and timezone
    					sprintf(cmds,"DevID %08X (%u): %.2fv %ddeg.C\n", cli_id, ++pknum_tx, (float)tchip.vcc/1000, (int)round(tchip.cels));
    				} else {// send message with request timestamp and timezone
    					sprintf(cmds,"DevID %08X (%u) TS: %.2fv %ddeg.C\n", cli_id, ++pknum_tx, (float)tchip.vcc/1000, (int)round(tchip.cels));
    				}
    				wtt_start = get_tmr(0);
    				LoraTxBuf(cmds);
    				sprintf(cmds+strlen(cmds),"\r");
    				printik(TAG_UART, cmds, MAGENTA_COLOR);
    				evt.type = 0; evt.num = pknum_tx;
    				if (xQueueSend(evtq, (void *)&evt, 0) != pdPASS) {
    					Report("[%s] Error while sending to evtq\r\n", TAG_UART);
    				}

    				needs = true;
    				tmneeds = get_tmr(wait_ack);
    			}
    		}

    		while ((rxd.rd != rxd.wr) && rxd.rdy) {
        		if (rxd.rd >= lora_buf_len) rxd.rd = 0;
        		data[len++] = rxd.buf[rxd.rd++];
    			if ( (strchr(data, '\n')) || (len >= BSIZE - 2) ) {
    				wtt_stop = get_tmr(0);
    				if (!strchr(data,'\n')) sprintf(data,"\n");
    				memset(stx,0,128);
    				sprintf(stx,"Recv (%u) : %s", ++pknum_rx, data);
    				printik(TAG_UART, stx, BROWN_COLOR);
    				//---------------
    				if (!ts_set) {
    					uks = strstr(data, "TS[");
    					if (uks) {
    						uks += 3;
    						uke = strchr(uks,':');
    						if (uke) {
    							memset(tmp, 0, 32);
    							memcpy(tmp, uks, (uint32_t)((uke-uks))&0x0F);//timestamp
    							time_t tt, stm = (time_t)atoi(tmp);
    							tt = wtt_stop - wtt_start;
    							dur = tt / 1000;
    							if ((tt % 1000) > 500) dur++;
    							stm += dur;
    							SetDevTime(&stm);// !!!!!!!!!!!   SET DATE_TIME   !!!!!!!!!!!!!!!!!!!!
    							uks = uke + 1;
    							uke = strchr(uks,']');
    							if (uke) {
    								*uke = '\0';
    								//setenv("TZ", uks, 1);
    								//tzset();
    								ts_set = true;
    								//sprintf(stx,"Time %u+%u (%u-%u|%u) with zone %s\n",
    								//			(uint32_t)(stm-dur), (uint32_t)dur, (uint32_t)wtt_start,
    								//			(uint32_t)wtt_stop, (uint32_t)(tt % 1000), uks);
    								//printik(TAG_UART, stx, BROWN_COLOR);
    							}
    						}
    					}
    				}
    				//---------------
    				len = 0; memset(data, 0, BSIZE);
    				evt.type = 1; evt.num = pknum_rx;
    				if (xQueueSend(evtq, (void *)&evt, 0) != pdPASS) {
    					Report("[%s] Error while sending to evtq\r\n", TAG_UART);
    				}
    			}
    		}//while


    		lvl = gpio_get_level(U2_SW2);
    		if (lvl) tmneeds = get_tmr(10);

    		if (needs) {
    			if (check_tmr(tmneeds)) needs = false;
    		}

    	}

    	free(data);

    }

    memset(stx, 0, 128); sprintf(stx, "Stop serial_task\r\n"); printik(TAG_UART, stx, CYAN_COLOR);

    vTaskDelete(NULL);
}

//******************************************************************************************

