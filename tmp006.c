#include "func.h"

#ifdef TMP006
#include "tmp006.h"

// from TI CC3200 SDK-1.3.0 (i2c_if.c)
//int I2C_IF_Write(unsigned char ucDevAddr, unsigned char *pucData, unsigned char ucLen, unsigned char ucStop)
//int I2C_IF_Read(unsigned char ucDevAddr, unsigned char *pucData, unsigned char ucLen)
//int I2C_IF_ReadFrom(unsigned char ucDevAddr,unsigned char *pucWrDataBuf,unsigned char ucWrLen,unsigned char *pucRdDataBuf,unsigned char ucRdLen)

//*****************************************************************************************

// Read 16 bit from I2C address addr and register reg
uint16_t i2c_read16(uint8_t addr, uint8_t reg)
{
uint16_t data = 0;
uint8_t wr_reg = reg;
uint8_t rd_dat[sizeof(uint16_t)] = {0};

    if (!I2C_IF_ReadFrom(addr, &wr_reg, 1, rd_dat, sizeof(rd_dat))) {
    	data = rd_dat[0];
    	data <<= 8;
    	data |= rd_dat[1];
    } else GPIO_IF_LedOn(MCU_RED_LED_GPIO);//Set error led (RED)

    return data;
}
//-----------------------------------------------------------------------------------------
// Write data to I2C address addr, register reg
void i2c_write16(uint8_t addr, uint8_t reg, uint16_t data)
{
uint8_t dat[2];

    dat[0] = data >> 8;
    dat[1] = data;
    if (I2C_IF_Write(addr, dat, sizeof(dat), true)) GPIO_IF_LedOn(MCU_RED_LED_GPIO);

}
//-----------------------------------------------------------------------------------------
// Configures sensor, use before reading from it
void config_TMP006(uint8_t addr, uint16_t samples)
{
    i2c_write16(addr, TMP006_CONFIG, samples | TMP006_CFG_MODEON | TMP006_CFG_DRDYEN);
}
//-----------------------------------------------------------------------------------------
// Read raw sensor temperature
int16_t readRawDieTemperature(uint8_t addr)
{
  return ((i2c_read16(addr, TMP006_TAMB)) >> 2);
}
//-----------------------------------------------------------------------------------------
// Read raw thermopile voltage
int16_t readRawVoltage(uint8_t addr)
{
  return (i2c_read16(addr, TMP006_VOBJ));
}
//-----------------------------------------------------------------------------------------
// Calculate object temperature based on raw sensor temp and thermopile voltage
double readObjTempC(uint8_t addr)
{
    double Tdie = (double)readRawDieTemperature(addr);
    double Vobj = (double)readRawVoltage(addr);
    Vobj *= 156.25;  // 156.25 nV per LSB
    Vobj /= 1000000000; // nV -> V
    Tdie *= 0.03125; // convert to celsius
    Tdie += 273.15; // convert to kelvin

    // Equations for calculating temperature found in section 5.1 in the user guide
    double tdie_tref = Tdie - TMP006_TREF;
    double S = (1 + TMP006_A1 * tdie_tref + TMP006_A2 * tdie_tref * tdie_tref);
    S *= TMP006_S0;
    S /= 10000000;
    S /= 10000000;

    double Vos = TMP006_B0 + TMP006_B1 * tdie_tref + TMP006_B2 * tdie_tref * tdie_tref;

    double fVobj = (Vobj - Vos) + TMP006_C2 * (Vobj-Vos) * (Vobj-Vos);

    double Tobj = sqrt(sqrt(Tdie * Tdie * Tdie * Tdie + fVobj/S));

    //Tobj -= 273.15; // Kelvin -> *C

    return Tobj;
}
//-----------------------------------------------------------------------------------------
// Caculate sensor temperature based on raw reading
double readDieTempC(uint8_t addr)
{
    return ((double)readRawDieTemperature(addr) * 0.03125);// convert to celsius
}
//*****************************************************************************************

#endif


