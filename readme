/*Bu yazilim mikroC PRO'da yazilmistir
fnx03(read holding register) ve fnx10(write multiple register)
sorgularina cevap verebilir

https://rapidscada.net/modbus/ModbusParser.aspx
https://www.simplymodbus.ca/
https://www.simplymodbus.ca/FC03.htm
https://www.simplymodbus.ca/FC16.htm
https://www.productinfo.schneider-electric.com/powertaglinkuserguide/powertag-link-user-guide/English/BM_PowerTag%20Link%20D%20User%20Manual_4af62430_T000501355.xml/$/TPC_ModbusTableFormatandDataTypes_4af62430_T000501590
https://product-help.schneider-electric.com/ED/ES_Power/NT-NW_Modbus_IEC_Guide/EDMS/DOCA0054EN/DOCA0054xx/Master_NS_Modbus_Protocol/Master_NS_Modbus_Protocol-5.htm

Read Numeric Variables (FC=03)
Request
This command is requesting the contents of numeric variable #0000
from the slave device with address 1

01 03 00 00 00 03 05 CB
01: The Slave Address (11 hex = address17 )
03: The Function Code (read Numeric variables)
0000: The Data Address of the first variable requested.
0003: The total number of variables requested. (read 3 variable)
05CB: The CRC (cyclic redundancy check) for error checking.
Read Numeric Variables (FC=03)
Response

{01}{03}{06}{00}{25}{00}{EF}{00}{F0}{5D}{03}

01: The Slave Address (11 hex = address17 )
03: The Function Code (read Numeric variables)
06 The number of data bytes to follow (3 registers x 4 bytes each = 12 bytes)
002500EF00F0: The contents of  3x32-bit variable  //datalar degiskenlik gosterebilir
5D03: The CRC (cyclic redundancy check).*/


