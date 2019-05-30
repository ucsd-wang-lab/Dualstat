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




typedef enum {  AMP_INIT_S, AMP_TRIGGER_S, AMP_WAIT_FOR_DATA_S, AMP_WAIT_FOR_TIME_S, AMP_STOP_S} amperoState_t;    
uint8_t adcDRDY = FALSE;
amperoState_t aState = AMP_STOP_S;
onAmperoCfg_t amperoCfg;
uint32_t ampTickCnt = 0;
uint32_t ampTicksPerSample = 0;

CY_ISR_PROTO(adcISR); // use to process Rx interrupt

CY_ISR(adcISR) //interrupt on Rx byte received
{   
    ADC_DRDY_ClearInterrupt();
    adcDRDY = TRUE;
}

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

void amperoExperimentStart(onAmperoCfg_t cfg)
{
    amperoCfg.channel = cfg.channel;
    amperoCfg.posNum = cfg.posNum;
    amperoCfg.potential = cfg.potential;
    amperoCfg.sampleCnt = cfg.sampleCnt;
    amperoCfg.sampleRate = cfg.sampleRate;
    adc_int_StartEx(adcISR);    //start interrupt
}

void amperoExperiment(onAmperoCfg_t cfg)
{
    dacChannel_t dacChan;
    adcChannelCode_t adcChan;
    if (cfg.channel == ON_CH_A)
    {
        dacChan = DAC_CH_A;
        adcChan = ADC_CH_A;
    }
    else if (cfg.channel == ON_CH_B)
    {
        dacChan = DAC_CH_B;
        adcChan = ADC_CH_B;
    }
    else
    {
        DBG_PRINTF("Error! Bad ampero channel.\r\n");
        return;
    }
    //Reset ADC and turn on reference
    dacSetRef(REF_ON);
    adcReset();
    adcConfigRateRef(SPS20,TRBO_DIS, ADC_SINGLE_CONV, ADC_REF_INTERN);
    adcConfigChanGain(adcChan, ADC_G1, PGA_DIS);
    dacSet(cfg.potential, DAC_CH_A, cfg.posNum);
    
    for (uint8_t i=0;i<100;i++)
    {
        adcStartConv();
        INTERNAL_ADC_StartConvert();
        CyDelay(100);
        
        printSignedMVolts(adcCode2Volts(adcReadData()));
        INTERNAL_ADC_IsEndConversion(INTERNAL_ADC_WAIT_FOR_RESULT);
        uint16_t mVoltRef = INTERNAL_ADC_CountsTo_mVolts(ADC_CH0, INTERNAL_ADC_GetResult16(ADC_CH0));
        DBG_PRINTF("\t%d\r\n", mVoltRef);          
    }
    
    switch (aState)
    {
        case AMP_INIT_S:
        {
            ampTickCnt = 0;
            ampTicksPerSample = 100;
        }
        case AMP_TRIGGER_S:
        {
            adcStartConv(); //start conversion on pot signal
            INTERNAL_ADC_StartConvert();    //start conversion on reference
            aState = AMP_WAIT_FOR_DATA_S;
            break;
        }
        case AMP_WAIT_FOR_DATA_S:
        {
            ampTickCnt++;
            if (adcDRDY == TRUE)
            {
                adcDRDY = FALSE;
                printSignedMVolts(adcCode2Volts(adcReadData()));
                INTERNAL_ADC_IsEndConversion(INTERNAL_ADC_WAIT_FOR_RESULT); //TODO: Make this non-blocking
                uint16_t mVoltRef = INTERNAL_ADC_CountsTo_mVolts(ADC_CH0, INTERNAL_ADC_GetResult16(ADC_CH0));
                DBG_PRINTF("\t%d\r\n", mVoltRef);   
                (ampTickCnt >= ampTicksPerSample) ? (aState = AMP_TRIGGER_S) : (aState = AMP_WAIT_FOR_TIME_S);
            }
            break;
        }
        case AMP_WAIT_FOR_TIME_S:
        {
            if (++ampTickCnt >= ampTicksPerSample)
            {
                aState = AMP_TRIGGER_S;
            }
            break;
        }        
        case AMP_STOP_S:
        {
            break;
        }
    }
    
    DBG_PRINTF("TEST>  Ampero test complete!\r\n"); 
}


/* [] END OF FILE */
