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
#include "serial_ctrl.h"
#include "dac.h"
#include "adc.h"
#include "on.h"

/* Functions */
uint8_t isCharReady(void);
uint8_t getRxStr(void);
void processCommandMsg(void);
CY_ISR_PROTO(MfgRxInt); // use to process Rx interrupt
char HexToAscii(uint8_t value, uint8_t nibble);
unsigned char atoh (unsigned char data);

/* Global variables */
uint8_t	RxBuffer[RxBufferSize];     // Rx circular buffer to hold all incoming command
uint8_t  *RxReadIndex	 = RxBuffer;    // pointer to position in RxBuffer to write incoming Rx bytes
uint8_t  *RxWriteIndex = RxBuffer;    // pointer to position in RxBuffer to read and process bytes
char   *RxStrIndex = RB.RxStr;      // pointer to command string buffer (processed messages)
                                    // each Rx command consists of: <byte command><string value><CR>
//buffer to hold application settings
typedef struct TParamBuffer{
    uint8 L, R;
} ParamBuffer; //settings
volatile ParamBuffer PB;  


void startSerialCtrlInt(void)
{
    Rx_Int_StartEx(MfgRxInt);//start Rx interrupt  
}

void serialCtrl_ProcessEvents(void)
{
    if(isCharReady()) //Rx buffer not empty
    {
        if (getRxStr())   //extract first command message (non-blocking)
        {
            processCommandMsg(); 
        }
    }   
}

CY_ISR(MfgRxInt) //interrupt on Rx byte received
{   
    DBG_UART_ClearRxInterruptSource(DBG_UART_INTR_RX_NOT_EMPTY ); //clear interrupt
    
    //move all available characters from Rx queue to RxBuffer
    char byte;
    while((byte = DBG_UART_UartGetChar()) !=0 )
    {
        *RxWriteIndex++ = byte; 
		if (RxWriteIndex >= RxBuffer + RxBufferSize) RxWriteIndex = RxBuffer;      
	}   
}

//===========================================================================
uint8_t isCharReady(void) 
{
	return !(RxWriteIndex == RxReadIndex);
}

//===========================================================================

uint8_t getRxStr(void)
{
    uint8_t RxStr_flag = 0;
    static uint8_t Ch;//static?
   
	Ch = *RxReadIndex++;       //read next char in buffer
    if (RxReadIndex >= RxBuffer + RxBufferSize) RxReadIndex = RxBuffer;
            
    //if (Ch == EOM_char)
    if ( (Ch == EOM_CR) || (Ch == EOM_LF) ) //any standard terminator
    {
        *RxStrIndex = 0;        //terminate string excluding EOM_char
        RxStrIndex = RB.RxStr;  //reset pointer
        if (strlen(RB.RxStr) > 0)//non-empty message received
        {
            RxStr_flag  = 1;    //set flag to process message
        }   
    }
    else                        //string body char received
    {
        *RxStrIndex++ = Ch;     //build command message   
        //todo: problem if first char is empty space
    }   

    return RxStr_flag;        
}

//===========================================================================
//===========================================================================
// Process UART Receive Buffer content: RB.RxStr = RB.cms + RB.valstr
// 
//===========================================================================

void processCommandMsg(void)
{    
    // check received message for any valid command and execute it if necessary or report old value
    // if command not recognized, then report error (!)
    if  (RB.cmd == cmd_V)//command 'V' received..
    {
        DBG_PRINTF("V\n\r");//echo command  
        DBG_PRINTF("TEST> %s: FW Version %d.%d.%d, Built: %s, %s\n\r", PROJECT_NAME, FV_MAJOR, FV_MINOR, FV_POINT, COMPILE_TIME, FIRMWARE_DATE);
    }
    else if (RB.cmd == cmd_L)//command received..
    {
        PB.L = RB.valstr[0];
        if (PB.L == ASCII_0)
        {
            DBG_PRINTF("L0\n\rTEST>  Red LED OFF\r\n");//echo command   
            LED_2_Write(GPIO_LOW);
        }
        else if (PB.L == ASCII_1)
        {
            DBG_PRINTF("L1\n\rTEST>  Red LED ON\r\n");//echo command
            LED_2_Write(GPIO_HIGH);
        }
        else 
        {
            DBG_PRINTF("TEST> Red LED command value '%c' not recognized. Valid values are 0 and 1.\r\n", RB.valstr[0]);//echo command and value       
        }
    } 
    else if (RB.cmd == cmd_R)//command 'R' received..
    {
        PB.R = RB.valstr[0];
        adcReset();
        if (PB.R == ASCII_0)
        {
            DBG_PRINTF("R0\n\rTEST>  Reference OFF\r\n");//echo command   
            dacSetRef(REF_OFF); //turn off the 1.25V reference
        }
        else if (PB.R == ASCII_1)
        {
            DBG_PRINTF("R1\n\rTEST>  Reference ON\r\n");//echo command
            dacSetRef(REF_ON); //turn on the 1.25V reference
            CyDelay(10);
            INTERNAL_ADC_StartConvert();
            INTERNAL_ADC_IsEndConversion(INTERNAL_ADC_WAIT_FOR_RESULT);
            uint16_t mVoltRef = INTERNAL_ADC_CountsTo_mVolts(ADC_CH0, INTERNAL_ADC_GetResult16(ADC_CH0));
            DBG_PRINTF("TEST>  Ref : %d mV\r\n", mVoltRef);//echo command
        }
        else 
        {
            DBG_PRINTF("TEST> Reference command value '%c' not recognized. Valid values are 0 and 1.\r\n", RB.valstr[0]);//echo command and value       
        }
    }
    else if (RB.cmd == cmd_S)
    {
        //Echo
        DBG_PRINTF("S");
        uint8_t len = 6;
        for(uint8_t i=0; i < len; i++)
        {
            if (RB.valstr[i] == 0)
            {
                len--;
                //RB.valstr[i] = '*';
            }
            DBG_PRINTF("%c", RB.valstr[i]);
        }
        DBG_PRINTF("\r\n");
        /* Parse DAC command data First nibble is channel, next 2 bytes are the value (voltage)*/       
        char ch = (RB.valstr[0]);  //channel
        //DBG_PRINTF("DAC CH: %c\r\n", ch);
        uint16_t val = 0;
        uint8_t posFlag = TRUE;
        for (uint8_t i=1; i < len; i++)    //skip first nibble since that is the channel, N.B. i+=2
        { 
            if (isdigit(RB.valstr[i]))
            {
                val = 10 * val + RB.valstr[i] - '0';
            }
            else if (RB.valstr[i] == ASCII_MINUS)
            {
                posFlag = FALSE;
            }
        }
        (posFlag == TRUE) ? (DBG_PRINTF("TEST> Set DAC[%c]: %d mV\r\n", ch, val)) : (DBG_PRINTF("TEST> Set DAC[%c]: -%d mV\r\n", ch, val));
        switch (ch)
        {
            case ASCII_A:
                dacSet(val, DAC_CH_A, posFlag);
            break;
            case ASCII_B:
                dacSet(val, DAC_CH_B, posFlag);
            break;
            case ASCII_F:
                dacSet(val, DAC_CH_BOTH, posFlag);
            break;
            default:
            DBG_PRINTF("TEST>  Error! Bad DAC channel.\r\n"); 
            break;
        }
    }
    else if (RB.cmd == cmd_G)
    {
        adcReset();
        adcConfigRateRef(SPS20,TRBO_DIS, ADC_SINGLE_CONV, ADC_REF_INTERN); //set sample rate
        //DBG_PRINTF("TEST> ADC REG1: 0x%x\r\n", adcReadReg(ADC_REG1));
        int32_t data = 0;
        switch (RB.valstr[0])
        {
            case ASCII_A:
                DBG_PRINTF("GA\r\n");    //Echo
                adcConfigChanGain(ADC_CH_A, ADC_G1, PGA_DIS);
                adcStartConv();
                CyDelay(100);
                //DBG_PRINTF("TEST> ADC REG0: 0x%x\r\n", adcReadReg(ADC_REG0));
                data = adcReadData();
                DBG_PRINTF("TEST> ADC_CHA\r\n\tCode: 0x%x\r\n\tmVolts: ", data);
                printSignedMVolts(adcCode2Volts(data));
                DBG_PRINTF(" mV\r\n");
                
            break;
            case ASCII_B:
                adcConfigChanGain(ADC_CH_B, ADC_G1, PGA_DIS);
                DBG_PRINTF("GB\r\n");    //Echo
                adcStartConv();
                CyDelay(100);
                //DBG_PRINTF("TEST> ADC REG0: 0x%x\r\n", adcReadReg(ADC_REG0));
                data = adcReadData();
                DBG_PRINTF("TEST> ADC_CHB\r\n\tCode: 0x%x\r\n\tmVolts: ", data);
                printSignedMVolts(adcCode2Volts(data));
                DBG_PRINTF(" mV\r\n");
            break;
            case ASCII_0:
                adcPwrDown();
                DBG_PRINTF("G0\r\n");    //Echo
                DBG_PRINTF("TEST> ADC Off\r\n"); 
            break;                
            default:
            DBG_PRINTF("TEST>  Error! Bad ADC channel.\r\n"); 
            break;
        }
    }
    else if (RB.cmd == cmd_A)
    {
        DBG_PRINTF("A");
        uint8_t len = 5;
        for(uint8_t i=0; i < len; i++)
        {
            if (RB.valstr[i] == 0)
            {
                len--;
                //RB.valstr[i] = '*';
            }
            DBG_PRINTF("%c", RB.valstr[i]);
        }
        DBG_PRINTF("\r\n");
        /* Parse DAC command data First nibble is channel, next 2 bytes are the value (voltage)*/       
        char ch = (RB.valstr[0]);  //channel
        //DBG_PRINTF("DAC CH: %c\r\n", ch);
        uint16_t val = 0;
        uint8_t posFlag = TRUE;
        for (uint8_t i=1; i < len; i++)    //skip first nibble since that is the channel, N.B. i+=2
        { 
            if (isdigit(RB.valstr[i]))
            {
                val = 10 * val + RB.valstr[i] - '0';
            }
            else if (RB.valstr[i] == ASCII_MINUS)
            {
                posFlag = FALSE;
            }
            
        }
        DBG_PRINTF("TEST> AMPERO - POT[%c]: %d mV\r\n", ch, val);
        POT_A_SHDN_Write(GPIO_LOW); //disable potentiostat
        dacSetRef(REF_ON); //turn on the 1.25V reference
        switch (ch)
        {
            case ASCII_A:
                dacSet(val, DAC_CH_A, posFlag);
                adcConfigChanGain(ADC_CH_A, ADC_G1, PGA_DIS);
            break;
            case ASCII_B:
                dacSet(val, DAC_CH_B, posFlag);
                adcConfigChanGain(ADC_CH_B, ADC_G1, PGA_DIS);
            break;
            case ASCII_F:
                dacSet(val, DAC_CH_BOTH, posFlag);
            break;
            default:
            DBG_PRINTF("TEST>  Error! Bad channel.\r\n"); 
            break;
        }
        CyDelay(1000);  //delay for voltage to stabilize
        POT_A_SHDN_Write(GPIO_HIGH); //enable potentiostat
        for (uint16_t i=0;i<1200;i++)
        {
            adcStartConv();
            INTERNAL_ADC_StartConvert();
            CyDelay(100);
            printSignedMVolts(adcCode2Volts(adcReadData()));
            INTERNAL_ADC_IsEndConversion(INTERNAL_ADC_WAIT_FOR_RESULT);
            uint16_t mVoltRef = INTERNAL_ADC_CountsTo_mVolts(ADC_CH0, INTERNAL_ADC_GetResult16(ADC_CH0));
            DBG_PRINTF("\t%d\r\n", mVoltRef);          
        }
        DBG_PRINTF("TEST>  Ampero test complete!\r\n"); 
        switch (ch) //RETURN TO 0V, 100 accounts for offset
        {
            case ASCII_A:
                dacSet(150, DAC_CH_A, FALSE);
                adcConfigChanGain(ADC_CH_A, ADC_G1, PGA_DIS);
            break;
            case ASCII_B:
                dacSet(150, DAC_CH_B, FALSE);
                adcConfigChanGain(ADC_CH_B, ADC_G1, PGA_DIS);
            break;
            case ASCII_F:
                dacSet(val, DAC_CH_BOTH, posFlag);
            break;
            default:
            DBG_PRINTF("TEST>  Error! Bad channel.\r\n"); 
            break;
        }
    }
    else if (RB.cmd == cmd_C)//command received..
    {
        DBG_PRINTF("C\r\n");    //Echo 
        adcPrintConfig();
    }
    else if (RB.cmd == cmd_O)
    {
        DBG_PRINTF("O");
        //Echo 
        uint8_t len = sizeof(RB.valstr)/sizeof(char);
        for(uint8_t i=0; i < len; i++)
        {
            if (RB.valstr[i] == 0)
            {
                len--;
                //RB.valstr[i] = '*';
            }
            DBG_PRINTF("%c", RB.valstr[i]);
        }
        DBG_PRINTF("\r\n");
        /* Parse DAC command data First nibble is channel*/
        switch (RB.valstr[0])
        {
            case ASCII_A:
                offsetCalibration(ON_CH_A);
            break;
            case ASCII_B:
                offsetCalibration(ON_CH_B);
            break;
            default:
            DBG_PRINTF("TEST>  Error! Bad channel.\r\n"); 
            break;
        }
    }    
    else //command unrecognized - echo unrecognized command
    {
        DBG_PRINTF("%c\r\n", RB.cmd);
        DBG_PRINTF("TEST>  Command %c is not recognized.\r\n", RB.cmd);//echo command and value  
    }
}


/*******************************************************************************
* Function Name: HexToAscii
********************************************************************************
* Summary:
*        Converts either the higher or lower nibble of a hex byte to its
* corresponding ASCII.
*
* Parameters:
*  value - hex value to be converted to ASCII
*  nibble - 0 = lower nibble, 1 = higher nibble
*
* Return:
*  char - hex value for the value/nibble specified in the parameters
*
*******************************************************************************/
char HexToAscii(uint8_t value, uint8_t nibble)
{
    if(nibble == 1)
    {
        /* bit-shift the result to the right by four bits */
        value = value & 0xF0;
        value = value >> 4;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else if (nibble == 0)
    {
        /* extract the lower nibble */
        value = value & 0x0F;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else
    {
        value = ' ';  /* return space for invalid inputs */
    }
    return value;
}

unsigned char atoh (unsigned char data)
{ 
    if (data > '9') 
    { 
        data += 9;
    }
    return (data &= 0x0F);
}
/* [] END OF FILE */