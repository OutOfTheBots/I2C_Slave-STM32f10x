# I2C_Slave-STM32f10x
This is a basic software I2C Slave for a STM32F10x chipset. It uses a interupt on the SDA line to detect a start condition sent from the master. It uses PA0 as SDA line and PA1 as SCL line. It has a 256 byte data_register in which both the master and the slave can read/write too. At the time of this commit there isn't a time out watch dog so if the master starts a tranmission but then breaks befroe trnasmission is finished it will become stuch in a blocking manner.

At the time of this commit the driver is only able to handle transmissions with reads or writes of only 1 byte. 
