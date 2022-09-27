unsigned int fnxChecksum(unsigned char *input, int length,unsigned char *crcResult);
unsigned int fnx03(int startAddress, unsigned char *rxData, unsigned char *txData, int length);
unsigned int fnx10(int startAddress, unsigned char *rxData, unsigned char *txData, int length);
void writeUintToModbusRegister(unsigned int value, unsigned int registerAddress);
void writeDintToModbusRegister(long value, unsigned int registerAddress);
unsigned int readUintFromModbusRegister(unsigned int registerAddress);
unsigned long readDintFromModbusRegister(unsigned int registerAddress);


void Timer1Interrupt();
void init_timer();
void init_pins();
void UART1RXInterrupt();
void UART1TX();
