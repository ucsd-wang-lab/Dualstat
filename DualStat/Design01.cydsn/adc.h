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

typedef enum { SPS20, SPS45, SPS90, SPS175, SPS330, SPS600, SPS1000 } adcSampleRate_t;
typedef enum { ADC_CH_A = 8, ADC_CH_B = 10 } adcChannel_t;   //CH_A -> MUX: 1000, CH_B -> MUX: 1010
typedef enum { NORM, TRBO } adcTurbo_t; //Turbo is 2X SPS

/* Functions*/

void adcStartConv(void);
void adcReset(void);
void adcConfigChan(adcChannel_t ch);
void adcConfigRate(adcSampleRate_t sps, adcTurbo_t turbo);
int32_t adcRead(void);

#endif  //ADC_H
/* [] END OF FILE */
