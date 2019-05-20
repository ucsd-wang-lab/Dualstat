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
#include "on.h"
#include "adc.h"
#include "dac.h"



void offsetCalibration(onChannel_t ch)
{
    //Sample 10x every 10mV across the entire range
    dacChannel_t dacChan;
    adcChannelCode_t adcChan;
    if (ch == ON_CH_A)
    {
        dacChan = DAC_CH_A;
        adcChan = ADC_CH_A;
    }
    else if (ch == ON_CH_B)
    {
        dacChan = DAC_CH_B;
        adcChan = ADC_CH_B;
    }
    else
    {
        DBG_PRINTF("Error! Bad calibration channel.\r\n");
        return;
    }
    //Reset ADC and turn on reference
    dacSetRef(REF_ON);
    adcReset();
    adcConfigRateRef(SPS20,TRBO_DIS, ADC_SINGLE_CONV, ADC_REF_INTERN);
    adcConfigChanGain(adcChan, ADC_G1, PGA_DIS);
    #define MAX_CAL     2000
    #define CAL_STEP    10
    #define CAL_SAMPLES 5
    
    DBG_PRINTF("Starting calibration\r\n\tPotential Range: 0-%d\r\n\tStep Size: %d\r\n\tSample Count: %d\r\n", MAX_CAL, CAL_STEP, CAL_SAMPLES);
    for(uint16_t pot=0; pot<=MAX_CAL; pot+=CAL_STEP)
    {
        dacSet(pot, dacChan);
        DBG_PRINTF("%d, ", pot);    //potential in mV
        for(uint8_t i=0; i<CAL_SAMPLES; i++)
        {
            adcStartConv();
            CyDelay(75);
            printSignedMVolts(adcCode2Volts(adcReadData()));
            DBG_PRINTF(", ");
        }
        DBG_PRINTF("\r\n");
    }
    DBG_PRINTF("Calibration Finished!\r\n");
}

/* [] END OF FILE */
