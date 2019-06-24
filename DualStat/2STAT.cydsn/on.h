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
#ifndef ON_H
#define ON_H

#include "main.h"
    
typedef enum {  ON_CH_A, ON_CH_B, ON_CH_BOTH} onChannel_t;    

typedef struct
{
    onChannel_t channel;
    uint8_t posNum;
    uint16_t potential; //in mV
    uint16_t sampleCnt;
    uint16_t period;
    uint16_t delay;
} onAmperoCfg_t;

void offsetCalibration(onChannel_t ch);
void amperoExperiment(void);
void stopAll(void);
void amperoExperimentStart(onAmperoCfg_t cfg);

#endif /* ON_H */ 
/* [] END OF FILE */