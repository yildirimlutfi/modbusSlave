#include "programInterrupt.h"

extern bit uartTxFlag;
unsigned char modbusRegisters[2000];
unsigned char crcResult[2];
int doneFnx;
int i=0,j=0,k=0,x=0,y=0;//for donguleri icin olusturuldu

void main()
{
   init_pins();
   init_timer();
   for(i=0 ; i<10;i++)
   {
      if(i<10)
      {
         modbusRegisters[i*2]=0;
         modbusRegisters[i*2+1]=i;
      }
   }

   while(1)
   {
     if(uartTxFlag)
     {
       UART1TX();
       uartTxFlag=0;
     }
   }

}
