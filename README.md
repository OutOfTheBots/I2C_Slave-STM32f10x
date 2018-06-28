# I2C_Slave-STM32f10x
This is a basic software I2C Slave for a STM32F10x chipset. It uses a interupt on the SDA line to detect a start condition sent from the master. It uses PA0 as SDA line and PA1 as SCL line. It has a 256 byte data_register in which both the master and the slave can read/write too. 

It uses standard write sequence of : start bit, 7 bit slave address, 1 bit RW=0, SLave ACK, 8 bit register address, slave ACK, n number of bytes with slave ACK after each byte, stop bit to end transmission

It uses standard read sequence of : start bit, 7 bit slave address, 1 bit RW=0, slave ACK, 8 bit register address, slave ACK, repeat start bit, slave adress, slave ACK, slave sends n bits with ACK from master after each bit till Slave doesn't reciev ACK from master.
