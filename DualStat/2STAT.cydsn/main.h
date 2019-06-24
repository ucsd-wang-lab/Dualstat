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
#ifndef MAIN_H
#define MAIN_H

#define ENABLED     1
#define DISABLED    0

#define TRUE            (1u)
#define FALSE           (0u)

#define GPIO_LOW        (0u)
#define GPIO_HIGH       (1u)

//Project Info
#define PROJECT_NAME        "2STAT_XA"
#define NAME_LENGTH         8  // without null character
    
#define FV_MAJOR            0
#define FV_MINOR            0
#define FV_POINT            2
#define FIRMWARE_DATE       __DATE__
#define COMPILE_TIME        __TIME__

    

/* Options */    
#define SERIAL_CTRL         DISABLED 
#define DEBUG_UART_ENABLED  ENABLED
    
/* UART Functions*/    
#if (DEBUG_UART_ENABLED == ENABLED)
    #include "iprintf.h"
    #define DBG_PRINTF(...)          (iprintf(__VA_ARGS__))
    #define DBG_WAIT                 while((DBG_UART_SpiUartGetTxBufferSize() + DBG_UART_GET_TX_FIFO_SR_VALID) != 0)
#else
    #define DBG_PRINTF(...)
    #define DBG_WAIT    
#endif
    
void setSysHibernateFlag(uint8_t flag);
uint8_t getSysHibernateFlag(void);

#endif
/* [] END OF FILE */
