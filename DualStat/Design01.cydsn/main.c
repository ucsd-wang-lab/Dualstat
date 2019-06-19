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
#include <project.h>
#include "main.h"
#include "serial_ctrl.h"
#include "bleHandler.h"

#include "dac.h"
#include "adc.h"
#include "on.h"

/*Function declarations */
void StackEventHandler( uint32 eventCode, void *eventParam );
uint32_t getTick(void);
void resetTick(void);
void tickStart(void);
void tickStop(void);
void sysManagePower(void);

/* Global variables*/
volatile uint32_t tickTimer = 0;
uint8_t sysHibernate = FALSE;
/*******************************************************************************
* Function Name: Tick_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT tick mechanism.
*
*******************************************************************************/
CY_ISR(Tick_Interrupt) // MAIN_TICK_FREQ
{
    /* Indicate that timer is raised to the main loop */
    tickTimer++;
}

uint32_t getTick(void)
{
//    #if (DEBUG_UART_ENABLED == ENABLED)
//    if(tickTimer > 1)
//    {
//        //DBG_PRINTF("**********\tMissed tick! Tick Val: %d\t**********\r\n", tickTimer);
//    }
//    #endif
    return tickTimer;
}

void resetTick(void)
{
    tickTimer = 0;
    return;
}

void tickStart(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    // Register Tick_Interrupt() by the WDT COUNTER0 to generate an interrupt every tick
    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER0, Tick_Interrupt);
    // Enable the Counter0 ISR Handler
    CySysWdtEnableCounterIsr(CY_SYS_WDT_COUNTER0);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();   
}

void tickStop(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Disable the specified WDT counter */
    CySysWdtDisable(CY_SYS_WDT_COUNTER0);
    /* Locks out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}

#define TEMP_TOGGLE_CNT     4096
uint16_t temp_cnt = 0;

int main (void)
{
    CyGlobalIntEnable;   /* Enable global interrupts */
    /* Start peripherals */
    DBG_UART_Start();
    I2CM_Start();
    tickStart();
    Opamp_1_Start();
    
    INTERNAL_ADC_Start();
    
    //LED_1_Write(GPIO_HIGH);
    
    DBG_PRINTF("\r\n\r\nTEST> %s: FW Version %d.%d.%d, Built: %s, %s\n\r", PROJECT_NAME, FV_MAJOR, FV_MINOR, FV_POINT, COMPILE_TIME, FIRMWARE_DATE);
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    #if (SERIAL_CTRL == DISABLED)
        bleWake();
    #else
        startSerialCtrlInt();
    #endif
    
    
    DBG_PRINTF("TEST>  Reference ON\r\n");//echo command
    dacSetRef(REF_ON); //turn on the 1.25V reference
    CyDelay(10);
    INTERNAL_ADC_StartConvert();
    INTERNAL_ADC_IsEndConversion(INTERNAL_ADC_WAIT_FOR_RESULT);
    uint16_t mVoltRef = INTERNAL_ADC_CountsTo_mVolts(ADC_CH0, INTERNAL_ADC_GetResult16(ADC_CH0));
    DBG_PRINTF("TEST>  Ref : %d mV\r\n", mVoltRef);//echo command
    
    for(;;)
    {
        #if (SERIAL_CTRL == ENABLED)
            serialCtrl_ProcessEvents(); //handle serial events
        #else
            bleManagePower();
        #endif
        
        if(getTick())//MAIN_TICK_FREQ
        { 
            if (++temp_cnt == TEMP_TOGGLE_CNT)
            {
                temp_cnt = 0;
                (LED_1_Read() == GPIO_HIGH) ? LED_1_Write(GPIO_LOW) : LED_1_Write(GPIO_HIGH);
            }
            amperoExperiment(); //state machine for ampero
            resetTick();
        }
        sysManagePower();
    }
}

uint8_t getSysHibernateFlag(void)
{
    return sysHibernate;
}

void setSysHibernateFlag(uint8_t flag)
{
    sysHibernate = flag;
}

void sysManagePower(void)
{
    if (getSysHibernateFlag())
    {
        if (getMcuHibernateFlag())
        {
            CySysPmHibernate();
        }
    }
}

/*This function name cannot change.*/
void CyBoot_IntDefaultHandler_Exception_EntryCallback(void)
{
    //Turn on all LEDs, wait, then reset
    LED_1_Write(GPIO_HIGH);
    LED_2_Write(GPIO_HIGH);
    CyDelay(2000);
    CySoftwareReset();
}