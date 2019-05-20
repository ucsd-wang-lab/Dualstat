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
#include "dac.h"

#define ADC_WREG_CMD    0x40 //Command byte: 0100 rr00 (rr-> reg addr)
adcConfig_t adcConfig;

uint8_t gainCode2Val(adcGainCode_t code);
uint16_t refCode2Val(adcRefCode_t code);

int32_t adcReadData(void)
{
    //N.B. Channel muust be configured using adcConfigChan before the start of the measurement
    // Measurement is started with adcConvStart 
    #define ADC_RDATA_CMD   0x10  //0001 0000
    uint8_t adcData[3];
    uint8_t *ptrData = (uint8_t*)&adcData; 
    I2CReadBytes(ADS122C04_ADDR, ADC_RDATA_CMD, 3, ptrData);
    //DBG_PRINTF("Get ADC Data: 0x%x|0x%x|0x%x|0x%x\r\n", ADC_RDATA_CMD, adcData[0], adcData[1], adcData[2]);
    int32_t extendedData = (int32_t)((((adcData[0] & 0x80) ? (0xFF) : (0x00)) << 24) | ((adcData[0] & 0xFF) << 16) | ((adcData[1] & 0xFF) << 8) | ((adcData[2]) & 0xFF) << 0);
    //DBG_PRINTF("Extended ADC Data: 0x%x\r\n", extendedData);
    return extendedData;
}

int32_t adcCode2Volts(int32_t code)
{
    /*  Convert 24 bit 2's complement output code to mVolts. 
        Dependant on current reference and gain selection.
        Max Code is 0x007FFFFF
        Zero is     0x00000000
        Min Code is 0x00FFFFFF
        Single ended measurements range from 0x000000 to 0x7FFFFF
            N.B. Can still get negative codes due to offset
    */
    #define ADC_LVLS   (16777216)    //2^24
    //TODO: Remove floats for speed
    float lsb = (float)((2*adcConfig.ref.val)/adcConfig.gain.val)/ADC_LVLS;
    float  mVolts = code*lsb;
    //printSignedMVolts(mVolts);
    return mVolts;
}

void printSignedMVolts(int32_t volts)
{
    (volts < 0) ? DBG_PRINTF("-%d", volts) : DBG_PRINTF("%d", volts);
}

uint8_t adcReadReg(adcReg_t reg)
{
    #define ADC_RREG_CMD     0x20  //0010 rr00
    uint8_t cmd_byte = ADC_RREG_CMD | reg;
    uint8_t regData = 0;
    uint8_t *ptrData = (uint8_t*)&regData; 
    I2CReadBytes(ADS122C04_ADDR, cmd_byte, 1, ptrData);
    DBG_PRINTF("Get ADC Reg: 0x%x|0x%x\r\n", cmd_byte, regData);
    return regData;
}

void adcConfigRateRef(adcSampleRateCode_t sps, adcTurboCode_t turbo, adcConvModeCode_t conv, adcRefCode_t ref)
{   
    #define SPS_IDX     5
    #define TRBO_IDX    4
    #define CONV_IDX    3
    #define VREF_IDX    1
    
    adcConfig.turbo = turbo;
    adcConfig.sps = sps;
    adcConfig.convMode = conv;
    adcConfig.ref.code = ref;
    adcConfig.ref.val = refCode2Val(adcConfig.ref.code);
    
    uint8_t cmd_byte = ADC_WREG_CMD | ADC_REG1;
    //Build reg values from options
    uint8_t reg_val = sps << SPS_IDX;
    (turbo == TRBO_EN) ? (reg_val |= 1 << TRBO_IDX) : (reg_val &= ~(1 << TRBO_IDX));   //set bit if turb is enabled, else clear
    reg_val |= (conv << CONV_IDX);
    reg_val |= (ref << VREF_IDX);
    DBG_PRINTF("Set ADC Rate, Turbo, Conv, Ref: 0x%x|0x%x\r\n", cmd_byte, reg_val);
    I2CWriteByte(ADS122C04_ADDR, cmd_byte, reg_val);
}

void adcConfigChanGain(adcChannelCode_t ch, adcGainCode_t gain, adcPgaBypassCode_t pgaEnable)
{   
    #define MUX_IDX     4
    #define GAIN_IDX    1
    #define PGA_EN_IDX  0
    
    adcConfig.chan = ch;
    adcConfig.gain.code = gain;
    adcConfig.gain.val = gainCode2Val(adcConfig.gain.code);
    adcConfig.pgaEn = pgaEnable;
    
    uint8_t cmd_byte = ADC_WREG_CMD | ADC_REG0;
    //Build reg values from options
    uint8_t reg_val = ch << MUX_IDX;
    reg_val |= (gain << GAIN_IDX);
    (pgaEnable == PGA_EN) ? (reg_val &= ~(1 << PGA_EN_IDX)) : (reg_val |= 1 << PGA_EN_IDX);   //set bit if PGA is disbaled, clear to enable
    DBG_PRINTF("Set ADC Mux, Gain, PGA bypass: 0x%x|0x%x\r\n", cmd_byte, reg_val);
    I2CWriteByte(ADS122C04_ADDR, cmd_byte, reg_val);
}

void adcReset(void)
{
    #define ADC_RST_CMD ((uint8_t)0x06) //Command byte: 0000 0110
    I2CWriteCommand(ADS122C04_ADDR, ADC_RST_CMD);
    
    //Set config to reset values
    adcConfig.chan = 0;
    adcConfig.gain.code = ADC_G1;
    adcConfig.gain.val = gainCode2Val(adcConfig.gain.code);
    adcConfig.pgaEn = PGA_EN;
    adcConfig.sps = SPS20;
    adcConfig.turbo = TRBO_DIS;
    adcConfig.convMode = ADC_SINGLE_CONV;
    adcConfig.ref.code = ADC_REF_INTERN;
    adcConfig.ref.val = refCode2Val(adcConfig.ref.code);
}

void adcStartConv(void)
{
    #define ADC_START_CMD   ((uint8_t)0x08)  //Command byte: 0000 1000
    I2CWriteCommand(ADS122C04_ADDR, ADC_START_CMD);
}

void adcPwrDown(void)
{
    #define ADC_PWRDWN_CMD ((uint8_t)0x02) //Command byte: 0000 0010
    I2CWriteCommand(ADS122C04_ADDR, ADC_RST_CMD);
}

void adcPrintConfig(void)
{
    DBG_PRINTF("ADC Configuration Codes...\r\n");
    DBG_PRINTF("\tChannel:\t0x%x\r\n", adcConfig.chan);
    DBG_PRINTF("\tSPS:\t\t0x%x\r\n", adcConfig.sps);
    DBG_PRINTF("\tTurbo:\t\t0x%x\r\n", adcConfig.turbo);
    DBG_PRINTF("\tGain:\t\t0x%x\r\n", adcConfig.gain.code);
    DBG_PRINTF("\tPGA En:\t\t0x%x\r\n", adcConfig.pgaEn);
    DBG_PRINTF("\tRef:\t\t0x%x\r\n", adcConfig.ref.code);
    DBG_PRINTF("\tConv Mode:\t0x%x\r\n", adcConfig.convMode);
}

/* Private Functions*/
uint8_t gainCode2Val(adcGainCode_t code)
{
    // Maps the gain register code to the actual gain that is used. 
    // Ex. Code(7) -> Gain(128x)
    uint8_t val = 0;
    switch (code)
    {
        case ADC_G1:
            val = 1;
        break;
        case ADC_G2:
            val = 2;
        break;  
        case ADC_G4:
            val = 4;
        break;
        case ADC_G8:
            val = 8;
        break;  
        case ADC_G16:
            val = 16;
        break;     
        case ADC_G32:
            val = 32;
        break;  
        case ADC_G64:
            val = 64;
        break;   
        case ADC_G128:
            val = 128;
        break;        
    }
    return val;   
}


uint16_t refCode2Val(adcRefCode_t code)
{
    #define ADC_INTERN_REF  2048
    #define SYS_VDD         3300
    // Maps the refernce register code to the actual reference that is used. 
    // Ex. Code(0) -> Ref(2048mV)
    uint16_t val = 0;
    switch (code)
    {
        case ADC_REF_INTERN:
            val = ADC_INTERN_REF;
        break;
        case ADC_REF_EXTERN:
            val = DAC_REF;
        break;
        case ADC_REF_VDD:
            val = SYS_VDD;
        break;
    }
    return val;
}

/* [] END OF FILE */
