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
//Design of microprocessor based systems course project - ME235 UC Berkeley, Prof- George Anwar
// Author - Sharnam Shah, Date - May 2020
#include "project.h"

uint8 ByteCount = 0;
uint8 firstbyte = 0;
uint8 CommandReady = 0;
uint8 Count = 0;
uint8 Status;
uint8 sum = 0;
uint8 ReceivedBuffer[66];
uint8 TransmitBuffer[66];
uint8 stx=1;
uint8 sty=5;

struct command_protocol
{
    uint8 packet_size;
    uint8 command;
    uint8 buffer[64];
} Command_Packet;

struct command_protocol Transmit_Packet;

CY_ISR(ByteReceived)
{ 
    LEDDrive_Write(1);
    ReceivedBuffer[ByteCount++] = (uint8) (LabVIEW_UART_GetByte()&0x00ff);
    if(firstbyte == 0)
    {
        ByteCounter_WriteCompare(ReceivedBuffer[0]);
        firstbyte = 1;
   
    }
    Count = ByteCounter_ReadCounter();
    Status = ByteCounter_ReadStatusRegister();
    LEDDrive_Write(0);
    
}

CY_ISR(CommandReceived)
{
    uint8 i;
    
    if(CommandReady == 0)
    {
        Command_Packet.packet_size = ReceivedBuffer[0];
        Command_Packet.command = ReceivedBuffer[1];
    
        for(i=0;i<(Command_Packet.packet_size - 2);i++)
        {
            Command_Packet.buffer[i] =  ReceivedBuffer[i+2];
        }
    
        firstbyte = 0;
        CommandReady = 1;
        ByteCount = 0;
    }
    ByteCounter_ReadStatusRegister();
    LEDDrive_Write(0);
    //sum = sum+2;
    stx = stx+1;
}

int main(void)
{  
    uint8 i;
    uint8 j;
    uint8 flag = 0;
    uint8 dist = 0;
    
    UARTReset_Write(0);
    ByteCountReset_Write(0);
    
    LabVIEW_UART_Start();
    
    
    ByteCounter_Start();
    ByteCounter_Enable();
    ByteCounter_SetInterruptMode(ByteCounter_STATUS_CMP_INT_EN_MASK);
       
    ByteReceived_Start();
    ByteReceived_StartEx(ByteReceived);
    ByteReceived_Enable();
    
    CommandReceived_Start();
    CommandReceived_StartEx(CommandReceived);
    CommandReceived_Enable();
    
   
    
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    //LabVIEW_UART_PutString("PSOC Started\r\n");
    
    for(;;)
    {
        /* Place your application code here. */
        
        if(CommandReady)
        {
            switch(Command_Packet.command)
            {
                case 5:
                    Transmit_Packet.command = Command_Packet.command;
                    TransmitBuffer[1] = Transmit_Packet.command;
                    Transmit_Packet.packet_size = Command_Packet.packet_size;
                    TransmitBuffer[0] = Transmit_Packet.packet_size;
                    for(i=0;i<Transmit_Packet.packet_size;i++)
                    {
                            Transmit_Packet.buffer[i+2] = Command_Packet.buffer[i];
                            TransmitBuffer[i+2] = Transmit_Packet.buffer[i+2];
                        //    diff = TransmitBuffer[i+2] - stx;
                        //    if(diff<2)
                        //    {   //printf("%d",&diff);
                        //        flag = 7;
                        //        TransmitBuffer[1] = flag;
                        //    }
                    }
                    
                    TransmitBuffer[Transmit_Packet.packet_size] = stx;
                    TransmitBuffer[Transmit_Packet.packet_size+1] = sty;
                    
                    Transmit_Packet.packet_size = Command_Packet.packet_size+2;
                    for(j=2;j<2+(TransmitBuffer[0]/2);j=j+2)
                    {
                    dist = ((TransmitBuffer[j] - stx)*(TransmitBuffer[j] - stx))+((TransmitBuffer[j+1] - sty)*(TransmitBuffer[j+1] - sty));
                        if(dist<2)
                            {   //printf("%d",&diff);
                                flag = 100;
                                TransmitBuffer[1] = flag;
                            }
                    }
                        
                    CommandReady = 0;
                break;
                default:
                break;
            }
            LabVIEW_UART_PutArray(TransmitBuffer,Transmit_Packet.packet_size);
        }
        
                        
    }
}

/* [] END OF FILE */
