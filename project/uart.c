#include "reg.h"
#include "uart.h"
#include "typedefs.h"
#include "CONFIG.h"

uint8 OS_Q_MEM_SEL SerialInData[16];        //����������Ϣ���з���Ķ��пռ�
uint8 OS_Q_MEM_SEL SerialOutData[32];       //��д������Ϣ���з���Ķ��пռ�
bit SerialCanSend = 1;                      //���ڷ��ͱ�־��0�����ڷ���

void init_uart(void)
{
	SFR_ES = 1;			
	SFR_UART0FIFO = 0x80;	
	
	OSQCreate(SerialInData, 16);
  OSQCreate(SerialOutData, 32);	
}

void put_char(unsigned char c)
{
    OS_ENTER_CRITICAL();
	
    if (SerialCanSend == 1)                         
    {
        SerialCanSend = 0;
        SBUF = c;                                /* ���ڿ��У�ֱ�ӷ��� */
    }
    else
    {
        OSQIntPost(SerialOutData, c);            /* ������� */
    }

    OS_EXIT_CRITICAL();
}  

#pragma disable
void uart0_isr(void) interrupt 4 using 1
{
	uint8	count;
	uint8 data temp;

	SFR_EA = 0;
	
	if (RI == 1)
	{
		OS_INT_ENTER();
		RI = 0;
		if (SFR_UART0FIFO & 0x80)
		{		// use fifo?
			count = SFR_UART0FIFO & 0x1F;
			if (count & 0x10)
			{
				SFR_UART0FIFO = 0x90;		// overflowed, buffer clear
			}
			else
			{
				while (count)
				{
					OSQIntPost(SerialInData, SBUF);
					count--;
				};
			}
		}
		else 
		{
			OSQIntPost(SerialInData, SBUF);
		}
		OSIntExit();
		
		return;
	}
	
	if (TI == 1)
	{
		TI = 0;
		if (OSQAccept(&temp, SerialOutData) == OS_Q_OK)
		{
		    SBUF = temp;
		}
		else
		{
		    SerialCanSend = 1;
		}
	}
	
	SFR_EA = 1;
}