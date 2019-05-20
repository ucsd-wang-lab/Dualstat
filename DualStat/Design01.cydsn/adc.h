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
#ifndef ADC_H
#define ADC_H
#include "main.h"
    
#define ADS122C04_ADDR  0x40    //7-bit 100 0000

/* Register codes i.e. what is written to the registers*/
//Registers
typedef enum { ADC_REG0, ADC_REG1, ADC_REG2, ADC_REG3} adcReg_t;    //register addresses
//Codes
typedef enum { SPS20, SPS45, SPS90, SPS175, SPS330, SPS600, SPS1000 } adcSampleRateCode_t;
typedef enum { ADC_CH_A = 8, ADC_CH_B = 10 } adcChannelCode_t;   //CH_A -> MUX: 1000, CH_B -> MUX: 1010
typedef enum { TRBO_DIS, TRBO_EN } adcTurboCode_t; //Turbo is 2X SPS
typedef enum { ADC_G1, ADC_G2, ADC_G4, ADC_G8, ADC_G16, ADC_G32, ADC_G64, ADC_G128} adcGainCode_t;
typedef enum { PGA_EN, PGA_DIS} adcPgaBypassCode_t;
typedef enum { ADC_REF_INTERN, ADC_REF_EXTERN, ADC_REF_VDD} adcRefCode_t;
typedef enum { ADC_SINGLE_CONV, ADC_CONT_CONV} adcConvModeCode_t;

/* Register values, used when the value is different from the code and is needed in the application*/
typedef struct
{
    uint8_t code;
    uint8_t val;
} adcGain_t;

typedef struct
{
    uint8_t code;
    uint16_t val;
} adcRef_t;


//Struct to hold ADC configuration
typedef struct 
{
    adcChannelCode_t chan;
    adcSampleRateCode_t sps;
    adcTurboCode_t turbo;
    adcGain_t gain;
    adcPgaBypassCode_t pgaEn;
    adcRef_t ref;
    adcConvModeCode_t convMode;
} adcConfig_t;


/* Functions*/
void adcStartConv(void);
void adcReset(void);
void adcConfigChanGain(adcChannelCode_t ch, adcGainCode_t gain, adcPgaBypassCode_t pgaEnable);
void adcConfigRateRef(adcSampleRateCode_t sps, adcTurboCode_t turbo, adcConvModeCode_t conv, adcRefCode_t ref);
int32_t adcReadData(void);
void adcPwrDown(void);
uint8_t adcReadReg(adcReg_t reg);
int32_t adcCode2Volts(int32_t code);
void adcPrintConfig(void);
void printSignedMVolts(int32_t volts);

#endif  //ADC_H
/* [] END OF FILE */
