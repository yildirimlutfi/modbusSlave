#include "programInterrupt.h"
#include <stdio.h>
#include <stdint.h>

#define xTxEnable485 RG8_bit
//MODBBUS ADDRESS Baud=19200, Data Size=8, Parity=None,Stop Bit=1
#define modbusSlaveAddress 0x01
/*PIN MODE
The AD1PCFGL and TRIS registers control the operation of the A/D port pins. If the TRIS bit is cleared "output" or not "input"
*/
sbit pinDirectionTx at TRISG7_bit;
sbit pinDirectionRxTxEnable at TRISG8_bit;
sbit pinDirectionRx at TRISG9_bit;

char temp=0,dataCount=0;
unsigned char rxData[100];
unsigned char txData[100];
bit uartTxFlag;
unsigned  modbusStartAddress;
unsigned char modbusError[5];
unsigned int modbusErrorCode=0;

extern int i,j,k,x,y;
extern unsigned char modbusRegisters[2000];
extern unsigned char crcResult[2];
extern int doneFnx;

unsigned int readUintFromModbusRegisterReturn=0;


void init_pins()
{
  pinDirectionRx=1;//input
  pinDirectionRxTxEnable=0;//output
  pinDirectionTx=0;//output
  xTxEnable485=0;//false

  //////////////////////UART////////////////////////
  oscconbits.IOLOCK=0; //unlock
  RPINR18bits.U1RXR=27; //uart1 I RP2
  RPOR13bits.RP26R=3; //uart1 O RP3
  UART1_Init(38400); //uart baud rate ayarlandi init 38400 gercekte 19200
  // U1BRG=51;
  UART_Set_Active(&UART1_Read, &UART1_Write, &UART1_Data_Ready, &UART1_Tx_Idle);
  U1STAbits.UTXEN = 1; //enable transmission
  IFS0bits.U1RXIF = 0;
  IEC0bits.U1RXIE = 1; //enable U1RX interrupt
  oscconbits.IOLOCK=1;
}

void init_timer()
{
//Timer1 = 100ms
  T1CON            = 0x8020;
  T1IE_bit         = 1;
  T1IF_bit         = 0;
  IPC0             = IPC0 | 0x1000;
  PR1              = 25000;
}

void Timer1Interrupt() iv IVT_ADDR_T1INTERRUPT
{
  if(IFS0bits.T1IF==1)
  {
    modbusRegisters[0]=0;
    modbusRegisters[1]=k++;
    modbusRegisters[2]=0;
    modbusRegisters[3]=x++;
    modbusRegisters[4]=y;
    modbusRegisters[5]=x;
    if(k>100)
    {
     k=0;
    }
    if(x>256)
    {
     y++;
     x=0;
    }
    if(y>256)
    {
     y=0;
    }
    T1IF_bit=0;
  }
}

void UART1RXInterrupt() iv IVT_ADDR_U1RXINTERRUPT
{
  if(UART1_Data_Ready())
  {
    temp = UART1_Read();
    if((dataCount==0 && temp==modbusSlaveAddress) || rxData[0]==modbusSlaveAddress)//slave id  uygunsa paket doldur
    {
      rxData[dataCount] = temp;
      dataCount++;
    }
    else
    {
      dataCount=0;
      rxData[0]=0;
    }
  }

  if(dataCount>=8 && rxData[1]==0x03)//03 fonksiyon paketi tamamladi ise
  {
    doneFnx=fnxChecksum(rxData, 6,crcResult);//crc hesaplandi

    if((rxData[6] & crcResult[1]) == rxData[6]  && (rxData[7] & crcResult[0]) == rxData[7])
    {
     modbusErrorCode=0x00;//hata yok
    }
    else
    {
     modbusErrorCode=0xFF;//invalid checksum
    }

    dataCount=0;//rx icin olusturulan counter sifirlandi
    uartTxFlag=1;//tx fonksiyonuna girme sarti 1 yapildi
  }
  else if(dataCount > rxData[6]+8 && rxData[1]==0x10)//10 fonksiyon paketi tamamladi ise
  {
    doneFnx=fnxChecksum(rxData, rxData[6]+7,crcResult);//crc hesaplandi

   if((rxData[dataCount-2] & crcResult[1]) == rxData[dataCount-2]  && (rxData[dataCount-1] & crcResult[0]) == rxData[dataCount-1])
   {
    modbusErrorCode=0x00;//hata yok
   }
   else
   {
    modbusErrorCode=0xFF;//invalid checksum
   }

   dataCount=0;//rx icin olusturulan counter sifirlandi
   uartTxFlag=1;//tx fonksiyonuna girme sarti 1 yapildi
  }

  IFS0bits.U1RXIF = 0;//rx flag 0 yapilarak tekrar interrupt'a girmesi saglandi
}

void UART1TX()
{
  xTxEnable485=1;
  Delay_ms(1);

  /*modbusStartAddress
  40351'in karsiligi modbusRegister arrayinde 698. ve 699. eleman
  40351 350=15E olarak geldigi icin burada 698 e karsilik gelmesi icin duzenleme yapildi
  ilk olarak 1 atamasi yapilip 8 bit kaydirildi daha sonra 5E bir sayi artirildiktan sonra toplanip 2 kati alindi
  modbusRegister'in eleman sayisinin yarisi kadar cevap verilebilir */
  modbusStartAddress= rxData[2];//yuksek degerli bit atandi
  modbusStartAddress=modbusStartAddress<<8;//8 bit kaydirildi
  modbusStartAddress+=rxData[3];//dusuk degerli bit ile toplandi
  modbusStartAddress*=2;//adres masterin verdiginin 2 kati olarak olusturuldu

  if(modbusStartAddress + rxData[5]*2 >= 2000 || rxData[5]>100)
  {
   modbusErrorCode=0x02;//illegal data address
  }
  if(rxData[0] > 255 || rxData[1] > 255 || rxData[2] > 255 || rxData[3] > 255 || rxData[4] > 255 || rxData[5] > 255 || rxData[6] > 255 || rxData[7] > 255)
  {
   modbusErrorCode=0x03;//illegal data value
  }

  //data dogru geldi ise cevap ilgili cevap fonksiyonu cagirilarak yazilmasi saglandi
  if(modbusErrorCode==0x00)
  {
    if(rxData[1]==0x03)//read holding register
    {
     doneFnx=fnx03(modbusStartAddress,rxData, txData,rxData[5]*2);
    }
    else if(rxData[1]==0x10)//write multiple register
    {
     doneFnx=fnx10(modbusStartAddress,rxData, txData,rxData[6]);
    }

  }
  else
  {
    modbusError[0]=modbusSlaveAddress;
    modbusError[1]=0x80+rxData[1];//Error code 0x80 + Fuction code
    modbusError[2]=modbusErrorCode;//Exception code invalid checksum
    doneFnx=fnxChecksum(modbusError, 3,crcResult);//calculate crc
    modbusError[4]=crcResult[0];
    modbusError[3]=crcResult[1];
    for(i=0 ; i< 5 ; i++)
    {
     UART1_Write(modbusError[i]);
    }
  }

  Delay_ms(1);
  xTxEnable485=0;
}

unsigned int fnxChecksum(unsigned char *input, int length,unsigned char *crcResult)
{
  unsigned int temp=0xFFFF;
  for(i=0;i<length;i++)
  {
   temp=input[i]^temp;
   for(j=0;j<8;j++)
   {
    if(temp&0x01)
    {
      temp=temp>>1;
      temp=temp^0xA001;
    }
    else
    {
      temp=temp>>1;
    }
   }
  }
  crcResult[1]=temp;
  crcResult[0]=temp>>8;
  return 1;
}

unsigned int fnx03(int startAddress, unsigned char *rxData, unsigned char *txData, int length)
{
  txData[0]=modbusSlaveAddress; //slave address
  txData[1]=0x03;//function code
  txData[2]=length;//quantity byte count

  for(i=0 ; i<txData[2] ; i=i+2)
  {
    txData[i+3]=modbusRegisters[startAddress+i];
    txData[i+4]=modbusRegisters[startAddress+i+1];
  }

  doneFnx=fnxChecksum(txData,length+3,crcResult);
  txData[length+3]=crcResult[1];//CRC
  txData[length+4]=crcResult[0];//CRC

  for(i=0 ; i<= rxData[5]*2+4 ; i++)
  {
   UART1_Write(txData[i]);
  }
  return 1;
}

unsigned int fnx10(int startAddress, unsigned char *rxData, unsigned char *txData, int length)
{
  for(i=0 ; i<length ; i=i++)
  {
    modbusRegisters[startAddress+i]=rxData[i+7];
  }

  txData[0]=modbusSlaveAddress; //slave address
  txData[1]=0x10;//function code
  txData[2]=startAddress>>8;//quantity byte count
  txData[3]=startAddress;//quantity byte count
  txData[4]=(length>>8)/2;//quantity byte count
  txData[5]=length/2;//quantity byte count
  doneFnx=fnxChecksum(txData,6,crcResult);
  txData[6]=crcResult[1];//CRC
  txData[7]=crcResult[0];//CRC

  for(i=0 ; i<= 7 ; i++)
  {
   UART1_Write(txData[i]);
  }
  return 1;
}

void writeUintToModbusRegister(unsigned int value, unsigned int registerAddress)
{
 modbusRegisters[registerAddress+1]=value;
 modbusRegisters[registerAddress]=value>>8;
}

unsigned int readUintFromModbusRegister(unsigned int registerAddress)
{

 unsigned int rtn=modbusRegisters[registerAddress+1] || modbusRegisters[registerAddress]<<8;
 return rtn;
}

void writeDintToModbusRegister(long value, unsigned int registerAddress)
{
 modbusRegisters[registerAddress+3]=value;
 modbusRegisters[registerAddress+2]=value>>8;
 modbusRegisters[registerAddress+1]=value>>16;
 modbusRegisters[registerAddress]=value>>24;
}

unsigned long readDintFromModbusRegister(unsigned int registerAddress)
{
  unsigned long rtn;
  rtn=modbusRegisters[registerAddress];
  rtn=rtn<<8;
  rtn=rtn|modbusRegisters[registerAddress+1];
  rtn=rtn<<8;
  rtn=rtn|modbusRegisters[registerAddress+2];
  rtn=rtn<<8;
  rtn=rtn|modbusRegisters[registerAddress+3];
  return rtn;
}
