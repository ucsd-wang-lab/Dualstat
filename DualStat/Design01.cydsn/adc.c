/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "main.h"
#include "adc.h"
#include "i2c.h"

#define ADC_WREG_CMD    4 //Command byte: 0100 rr00 (rr-> reg addr)
#define ADC_REG_0       0   //mux, pga, pga bypass
#define ADC_REG_1       1   //dr, mode, cm, vref, ts
#define ADC_REG_2       2   //drdy, dcnt,crc,bcs,idac
#define ADC_REG_3       3   //IMUX

int32_t adcRead(void)
{
    //N.B. Channel muust be configured using adcConfigChan before the start of the measurement
    // Measurement is started with adcConvStart 
    #define ADC_CMD_RDATA   16  //0001 0000
    int32_t adcData = 0;
    uint8_t *ptrData = (uint8_t*)&adcData; 
    I2CReadBytes(ADS122C04_ADDR, ADC_CMD_RDATA, 3, ptrData);
    //DBG_PRINTF("ADC Data: 0x%x\r\n", adcData);
    return adcData;
}

void adcConfigRate(adcSampleRate_t sps, adcTurbo_t turbo)
{   
    #define SPS_IDX   5
    #define TRBO_IDX  4
    uint8_t cmd_byte = ADC_WREG_CMD & ADC_REG_1;
    //Build reg values from options
    uint8_t reg_val = sps << SPS_IDX;
    (turbo == TRBO) ? (reg_val |= 1 << TRBO_IDX) : (reg_val &= ~(1 << TRBO_IDX));   //set bit if turb is enabled, else clear
    DBG_PRINTF("Set ADC Rate: 0x%x|0x%x\r\n", cmd_byte, reg_val);
    I2CWriteByte(ADS122C04_ADDR, cmd_byte, reg_val);
}

void adcConfigChan(adcChannel_t ch)
{   
    //TODO: Add PGA control
    #define MUX_IDX   4
    uint8_t cmd_byte = ADC_WREG_CMD & ADC_REG_0;
    //Build reg values from options
    uint8_t reg_val = ch << MUX_IDX;
    DBG_PRINTF("Set ADC Mux: 0x%x|0x%x\r\n", cmd_byte, reg_val);
    I2CWriteByte(ADS122C04_ADDR, cmd_byte, reg_val);
}

void adcReset(void)
{
    #define ADC_RST_CMD ((uint8_t)6) //Command byte: 0000 0110
    I2CWriteCommand(ADS122C04_ADDR, ADC_RST_CMD);
}

void adcStartConv(void)
{
    #define ADC_START_CMD   ((uint8_t)8)  //Command byte: 0000 1000
    I2CWriteCommand(ADS122C04_ADDR, ADC_START_CMD);
}

/* [] END OF FILE */
