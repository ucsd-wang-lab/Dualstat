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

#define ADS122C04_ADDR  0x40    //7-bit 100 0000

typedef enum { SPS20, SPS45, SPS90, SPS175, SPS330, SPS600, SPS1000 } adcSampleRate_t;    
typedef enum { CH_A, CH_B } adcChannel_t;

#endif  //ADC_H
/* [] END OF FILE */
