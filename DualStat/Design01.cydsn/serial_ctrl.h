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
#ifndef SERIAL_CTRL_H
#define SERIAL_CTRL_H
    
#define RxBufferSize	128     //size of the Rx circular buffer

//any standard terminator (or their combination) will work
#define EOM_CR        0x0D    //message separator char (\r)
#define EOM_LF        0x0A    //message separator char (\n)

//define input commands-> 
#define cmd_V   'V' //read value for Rev
#define cmd_L   'L' //Red Led_2: 0 = OFF, 1 = ON
#define cmd_R   'R' //DAC Reference: 0 = OFF, 1 = ON
#define cmd_S   'S' //set DAC output
#define cmd_G   'G' //get ADC
#define cmd_A   'A' //ampero test
#define cmd_C   'C' //print hardware config
#define cmd_O   'O' //offset cal


//ascii to hex conversions
#define ASCII_0     48  //ascii code for 0
#define ASCII_1     49  //ascii code for 1
#define ASCII_2     50  //ascii code for 2
#define ASCII_3     51  //ascii code for 3
#define ASCII_4     52  //ascii code for 4
#define ASCII_5     53  //ascii code for 5
#define ASCII_6     54  //ascii code for 6
#define ASCII_7     55  //ascii code for 7
#define ASCII_8     56  //ascii code for 8
#define ASCII_9     57  //ascii code for 9 
#define ASCII_A     65  //ascii code for A
#define ASCII_B     66  //ascii code for B
#define ASCII_C     67  //ascii code for C
#define ASCII_D     68  //ascii code for D
#define ASCII_E     69  //ascii code for E
#define ASCII_F     70  //ascii code for F    

#define MAC_LENGTH  12  //length of mac address in hex characters    
    
// each Rx command consists of: <char command><string value><CR>
    union TRxBuffer {
        char   RxStr[20];   // received Rx packet (to hold individual command message)
        struct {            // anonimous structure
            char cmd;       // 1-byte command
            char valstr[13];// command value           
        };
    } RB;

void serialCtrl_ProcessEvents(void);  
void startSerialCtrlInt(void);

#endif //serial_ctrl_h
/* [] END OF FILE */
