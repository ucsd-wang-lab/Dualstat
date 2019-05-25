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
    #define MIN_CAL_MV     1200
    #define MAX_CAL_MV     1100
    #define CAL_STEP    10
    #define CAL_SAMPLES 5
    
    DBG_PRINTF("Starting calibration\r\n\tPotential Range: -%d - %d\r\n\tStep Size: %d\r\n\tSample Count: %d\r\n", MIN_CAL_MV, MAX_CAL_MV, CAL_STEP, CAL_SAMPLES);
    //Sweep from min to zero
    for(uint16_t pot=MIN_CAL_MV; pot>0; pot-=CAL_STEP)
    {
        dacSet(pot, dacChan, FALSE);
        DBG_PRINTF("-%d, ", pot);    //potential in mV
        for(uint8_t i=0; i<CAL_SAMPLES; i++)
        {
            adcStartConv();
            CyDelay(75);
            printSignedMVolts(adcCode2Volts(adcReadData()));
            DBG_PRINTF(", ");
        }
        DBG_PRINTF("\r\n");
    }
    //Sweep from zero to max
    for(uint16_t pot=0; pot<=MAX_CAL_MV; pot+=CAL_STEP)
    {
        dacSet(pot, dacChan, TRUE);
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
