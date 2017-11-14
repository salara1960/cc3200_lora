#include "func.h"

uint32_t cli_id = 0;
OsiMsgQ_t evtq = NULL;
uint32_t ThePin = PIN_58;
uint32_t TheChan = ADC_CH_1;
int i2c_err=0;

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
int len = 32;

	if (tag) len += strlen(tag);
	if (buf) len += strlen(buf);
	if (color) len += strlen(color);
	if (len > 32) {
		char *st = (char *)calloc(1, len);
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

}
//--------------------------------------------------------------------
void InitLora()
{
    MAP_PRCMPeripheralReset(PRCM_UARTA1);
    MAP_UARTConfigSetExpClk(LORA,
			    MAP_PRCMPeripheralClockGet(LORA_PERIPH),
			    UART_BAUD_RATE,
			    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    Message("Init UART1 (Lora) done.\r\n");
}
//--------------------------------------------------------------------
void LoraTxBuf(char *st)
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

    //UART_PRINT("\n\rTotal no of 32 bit ADC data printed :4096 \n\r");
    //UART_PRINT("\n\rADC data format:\n\r");
    //UART_PRINT("\n\rbits[13:2] : ADC sample\n\r");
    //UART_PRINT("\n\rbits[31:14]: Time stamp of ADC sample \n\r");
    //
    // Print out ADC samples
    if (prn) {
    	char st[128]={0};
    	sprintf(st,"Chan=%u Data=%u TimeStamp=%u Vcc=%.3fv\r\n", chan, ret, tsp, (float)ret/1000.0);//(float)(( ret*1.4)/4096) );
    	Message(st);
    }

    return ret;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
