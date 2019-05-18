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
#ifndef DAC_H
#define DAC_H

#include "main.h"
    
#define AD5647R_ADDR    0x0F    //7 bit, 8 bit 0x1E

typedef enum { CH_A, CH_B, CH_BOTH } dacChannel_t;
    
void setDAC(uint16_t val, dacChannel_t ch);
void enableDAC_ref(void);

#endif /* DAC_H */ 
/* [] END OF FILE */
