#include "func.h"
#ifdef TMP006
	#include "tmp006.h"
#endif

uint32_t cli_id = 0;
OsiMsgQ_t evtq = NULL;
uint32_t ThePin = PIN_58;
uint32_t TheChan = ADC_CH_1;
int i2c_err = 0;

//--------------------------------------------------------------------
void pMessage(const char *st)
{
    if (st) {
    	const char *uk = st;
        while (*uk != '\0') MAP_UARTCharPut(CONSOLE, *uk++);
    }
}
//--------------------------------------------------------------------
void SetDevTime(time_t *tim)
{
SlDateTime_t dt = {0};
struct tm *ct = localtime(tim);

	dt.sl_tm_day  = ct->tm_mday;
	dt.sl_tm_mon  = ct->tm_mon;
	dt.sl_tm_year = ct->tm_year;
	dt.sl_tm_hour = ct->tm_hour;
	dt.sl_tm_min  = ct->tm_min;
	dt.sl_tm_sec  = ct->tm_sec;

	sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
			  SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
			  sizeof(SlDateTime_t),
			  (_u8 *)&dt);

}
//--------------------------------------------------------------------
inline void printik(const char *tag, const char *buf, const char *color)
{

	char *st = (char *)calloc(1, strlen(tag) + strlen(buf) + strlen(color) + 32);
	if (st) {
		time_t ct = time(NULL);
		struct tm *ctimka = localtime(&ct);
		sprintf(st,"%02d:%02d:%02d | %s[%s] : %s%s",
					//ctimka->tm_mday, ctimka->tm_mon+1, (ctimka->tm_year+1900) - 2000,
					ctimka->tm_hour,
					ctimka->tm_min,
					ctimka->tm_sec,
					color,
					tag,
					buf,
					STOP_COLOR);
		pMessage(st);
		free(st);
	}

}
//--------------------------------------------------------------------
void InitLora()
{
    MAP_PRCMPeripheralReset(PRCM_UARTA1);
    MAP_UARTFIFOLevelSet(LORA, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    MAP_UARTConfigSetExpClk(LORA,
                            MAP_PRCMPeripheralClockGet(LORA_PERIPH),
                            UART_BAUD_RATE,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    Message("Init UART1 (Lora) done.\r\n");
}
//--------------------------------------------------------------------
void LoraTxBuf(const char *st)
{
    if (st) {
    	int i = 0, len = strlen(st);
        while (i < len) {
        	//MAP_UARTCharPutNonBlocking(LORA, *(st + i));
        	MAP_UARTCharPut(LORA, *(st + i));
        	i++;
        }
    }

}
//-----------------------------------------------------------------------
unsigned char LoraRxByte(unsigned char *byte)
{

    if (MAP_UARTCharsAvail(LORA) != false) {
    	*byte = MAP_UARTCharGetNonBlocking(LORA);
    	return 1;
    }

    return 0;
}
//--------------------------------------------------------------------
void init_adc(uint32_t pin)//ThePin
{

    MAP_PinTypeADC(pin, PIN_MODE_255);//ThePin = PIN_58;

    // Configure ADC timer which is used to timestamp the ADC data samples
    MAP_ADCTimerConfig(ADC_BASE, 2^17);
    // Enable ADC timer which is used to timestamp the ADC data samples
    MAP_ADCTimerEnable(ADC_BASE);

    // Enable ADC module
    MAP_ADCEnable(ADC_BASE);


}
//--------------------------------------------------------------------
uint32_t GetSampleADC(uint32_t chan, bool prn)//TheChan,
{
uint32_t ret = 0, tsp = 0;
uint8_t cnt = 4;

    // Enable ADC channel
    MAP_ADCChannelEnable(ADC_BASE, chan);

    while (cnt) {
    	if (ADCFIFOLvlGet(ADC_BASE, chan)) {
    		ret = ADCFIFORead(ADC_BASE, chan);
    		tsp = ret >> 14;
    		ret >>= 2; ret &= 0xFFF;
    		break;
    	}
    	cnt--;
    }
    // Disable ADC channel
    MAP_ADCChannelDisable(ADC_BASE, chan);

    // Print out ADC samples
    if (prn) {
    	char st[128];
    	sprintf(st,"Chan=%u Data=%u TimeStamp=%u Vcc=%.3fv\r\n", chan, ret, tsp, (float)ret/1000.0);
    	Message(st);
    }

    return ret;
}
//--------------------------------------------------------------------
inline void get_tsensor(t_sens_t *t_s)
{
    t_s->vcc = GetSampleADC(TheChan, false);

	#ifdef TMP006
    	t_s->kelv = readObjTempC(TMP006_ADDR);
    	t_s->cels = t_s->kelv - 273.15;
    	t_s->faren = 1.8 * (t_s->kelv - 273) + 32;
	#else
    	t_s->faren = 78.0;
    	t_s->cels = (t_s->faren - 32) * 5/9;
	#endif
}
//--------------------------------------------------------------------
#ifdef TMP006
	char *str_sensor(t_sens_t *t_s)
	{
		char *st = (char *)calloc(1, 80);
		if (st) {
			sprintf(st," Temperature :\n   %.2f%cC\n   %.2f%cF\n   %.2f%cK\n  ADC : %u mv",
					    t_s->cels, 0x1f,
						t_s->faren,0x1f,
						t_s->kelv, 0x1f,
						t_s->vcc);
		}
		return st;
	}
#endif
//--------------------------------------------------------------------

