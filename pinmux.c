//******************************************************************************
// This file was automatically generated by the CC31xx PinMux Utility
// Version: 1.0.2
//******************************************************************************

#include "pinmux.h"

#include "hdr.h"

//*****************************************************************************
void PinMuxConfig(void)
{

    // Enable Peripheral Clocks
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA0, PRCM_RUN_MODE_CLK);// (GPIO0  - GPIO7)
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);// (GPIO8  - GPIO15)
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA2, PRCM_RUN_MODE_CLK);// (GPIO16 - GPIO23)
    MAP_PRCMPeripheralClkEnable(PRCM_GPIOA3, PRCM_RUN_MODE_CLK);// (GPIO24 - GPIO31)

    MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);

#ifdef DISPLAY
    MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
    // Configure PIN_05 (GPIO14) for I2C0 I2C_SCL - P5
    MAP_PinTypeI2C(PIN_05, PIN_MODE_5);
    // Configure PIN_06 (GPIO15) for I2C0 I2C_SDA - P6
    MAP_PinTypeI2C(PIN_06, PIN_MODE_5);
#endif


    // Configure PIN_64 for GPIOOutput (GPIO9 RED)
    MAP_PinTypeGPIO(PIN_64, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x2, GPIO_DIR_MODE_OUT);

    // Configure PIN_01 for GPIOOutput (GPI010 ORANGE) // PIN_61 - GPIO6
    MAP_PinTypeGPIO(PIN_01, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x4, GPIO_DIR_MODE_OUT);//old PIN_01 GPIO10 0x4

    // Configure PIN_02 for GPIOOutput (GPI011 GREEN) // PIN_62 - GPIO7
    MAP_PinTypeGPIO(PIN_02, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x8, GPIO_DIR_MODE_OUT);//old PIN_02 GPIO11 0x8

    // Configure PIN_04 for GPIOInput (GPIO13 SW3)
    MAP_PinTypeGPIO(PIN_04, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA1_BASE, 0x20, GPIO_DIR_MODE_IN);

    // Configure PIN_15 for GPIOInput (GPIO22 SW2)
    MAP_PinTypeGPIO(PIN_15, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA2_BASE, 0x40, GPIO_DIR_MODE_IN);

    //Console
    MAP_PinTypeUART(PIN_55, PIN_MODE_3);// Configure PIN_55 for UART0 UART0_TX
    MAP_PinTypeUART(PIN_57, PIN_MODE_3);// Configure PIN_57 for UART0 UART0_RX

    /*========================  Lora pins  =================================*/

    // Configure PIN_59 for GPIOInput (GPIO4) : status
    MAP_PinTypeGPIO(PIN_59, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, 0x10, GPIO_DIR_MODE_IN);

    // Configure PIN_50 for GPIOOutput (GPIO0) : config
    MAP_PinTypeGPIO(PIN_50, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA0_BASE, 0x1, GPIO_DIR_MODE_OUT);// ????????????

    // Configure PIN_53 for GPIOOutput (GPIO30) : reset
    MAP_PinTypeGPIO(PIN_53, PIN_MODE_0, false);
    MAP_GPIODirModeSet(GPIOA3_BASE, 0x40, GPIO_DIR_MODE_OUT);// ????????????

    //Lora
    MAP_PinTypeUART(PIN_07, PIN_MODE_5);// Configure PIN_07 for UART1 UART1_TX (GPIO16)
    MAP_PinTypeUART(PIN_08, PIN_MODE_5);// Configure PIN_08 for UART1 UART1_RX (GPIO17)


}
