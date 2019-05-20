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

void offsetCalibration(onChannel_t ch);

#endif /* ON_H */ 
/* [] END OF FILE */
