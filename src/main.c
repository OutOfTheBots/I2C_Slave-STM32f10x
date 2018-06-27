/*
Copyright (c) 2018 Out of the BOTS
MIT License (MIT)
Author: Shane Gingell
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
ll copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

This is a software I2C slave driver for a STM32F10x chipset.
It uses an interupt on the SDA line to detect a start condition sent from Master
so it can run in backgound of your main code and just interupt when needed.
It is very basic and doesn't have a timeout watch dog at this stage so it is
possible to end up in a blocking state waiting on master it transmission fails
halfway through
*/

#include "stm32f10x.h"

unsigned char slave_address;
unsigned char reg_address;
unsigned char* data_reg[256];

void i2c_slave_setup (void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //enable port A
	GPIOA->CRL &= ((~GPIO_CRL_CNF0) | (GPIO_CRL_CNF0_0)) & (~GPIO_CRL_MODE0); //set SDA as input float
	GPIOA->CRL &= ((~GPIO_CRL_CNF1) | (GPIO_CRL_CNF1_0)) & (~GPIO_CRL_MODE1); //set SCL as input float

	//wait till both SDA and SCL go high
	while ( !(GPIOA->IDR & GPIO_IDR_IDR0)  ||  !(GPIOA->IDR & GPIO_IDR_IDR1) ){
	}

	AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PA;  //enable interupt on port A
	EXTI->FTSR |= EXTI_FTSR_TR0; //set falling trigger SDA
	EXTI->IMR |= EXTI_IMR_MR0;  //enable interupt on SDA
	NVIC_EnableIRQ(EXTI0_IRQn);  //set NVIC callbacks line 0
	NVIC_SetPriority(EXTI0_IRQn, 0); //set piroty
}

void reset_slave(void){
	EXTI->PR |= EXTI_PR_PR0;  //reset pending reg
	EXTI->IMR |= EXTI_IMR_MR0;  //enable interupt on SDA
}

void send_ACK(void){
	//send ACK back to master
	GPIOA->CRL |= GPIO_CRL_MODE0; //set SDA to output OD
	GPIOA->ODR &= ~GPIO_ODR_ODR0; //pull SDA low
	while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
	}
	while (GPIOA->IDR & GPIO_IDR_IDR1){   // wait for SCL to go low
	}
	GPIOA->CRL &= ~GPIO_CRL_MODE0; //set SDA back to input float
}

void send_8bits(unsigned char data){
	GPIOA->CRL |= GPIO_CRL_MODE0; //set SDA to output OD
	for (char i=0; i<8; ++i){
		GPIOA->ODR &= ~GPIO_ODR_ODR0;
		GPIOA->ODR |= ((data & 0x80)>>7);
		data <<=1;
		while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
		}
		while (GPIOA->IDR & GPIO_IDR_IDR1){   // wait for SCL to go low
		}
	}
	GPIOA->CRL &= ~GPIO_CRL_MODE0; //set SDA back to input float
}

unsigned char read_8bits(void){
	unsigned char data = 0;
	for (char i=0; i<8; ++i){
		while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
		}
		data <<=1;
		data |= (GPIOA->IDR & GPIO_IDR_IDR0);
		while (GPIOA->IDR & GPIO_IDR_IDR1){   // wait for SCL to go low
		}
	}return data;
}

unsigned char read_7bits(void){
	unsigned char data = 0;
	for (char i=0; i<7; ++i){
		while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
		}
		data <<=1;
		data |= (GPIOA->IDR & GPIO_IDR_IDR0);
		while (GPIOA->IDR & GPIO_IDR_IDR1){   // wait for SCL to go low
		}
	}return data;
}

unsigned char read_1bit(viod){
	unsigned char data=0;
	while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
	}
	data = (GPIOA->IDR & GPIO_IDR_IDR0);
	while (GPIOA->IDR & GPIO_IDR_IDR1){   // wait for SCL to go low
	}return data;
}





void EXTI0_IRQHandler(void){
	unsigned char rw;
	unsigned char sent_slave;
	unsigned char data = 0;
	if (GPIOA->IDR & GPIO_IDR_IDR1){  //check for start condition
		EXTI->IMR &= ~EXTI_IMR_MR0; //disable interupt on SDA
		while (GPIOA->IDR & GPIO_IDR_IDR1){ //wait for SCL to go low
		}

		sent_slave = read_7bits();
		if (!(sent_slave == slave_address)){ //not our slave address reset and return
			reset_slave();
			return;
		}

		rw = read_1bit();
		if (rw){ //if rw=1 then there is an error, reset and return
			reset_slave();
			return;
		}

		send_ACK();

		reg_address = read_8bits();

		send_ACK();

		//check for second start condition
		while (!(GPIOA->IDR & GPIO_IDR_IDR1)){  //wait for SCL to go high
		}
		data |= (GPIOA->IDR & GPIO_IDR_IDR0); //read 1 bit in case not a repeat start condition

		if (GPIOA->IDR & GPIO_IDR_IDR0){  //possible repeat start
			while ((GPIOA->IDR & GPIO_IDR_IDR0) && (GPIOA->IDR & GPIO_IDR_IDR1)){ //wait till a line goes low
			}
			if (!(GPIOA->IDR & GPIO_IDR_IDR0)  && (GPIOA->IDR & GPIO_IDR_IDR1)){ //repeat start condition meet
				while (GPIOA->IDR & GPIO_IDR_IDR1){ //wait for SCL to go low
				}

				//all code here only executes if a repeat start condition i.e master read sequence

				//read and check 7 bit slave address
				sent_slave = read_7bits();
				if (!(sent_slave == slave_address)){ //not our slave address reset and return
					reset_slave();
					return;
				}

				rw = read_1bit();
				if (!(rw)){ //if rw=0 then there is an error, reset and return
					reset_slave();
					return;
				}

				send_ACK();

				//write 8 bits of data
				send_8bits(data_reg[reg_address]);

				send_ACK();

				reset_slave();
				return;
			}
		}

		//code below here only executes if there isn't a repeat start condition i.e master write sequence
		else while (GPIOA->IDR & GPIO_IDR_IDR1){   //not repeat start so complete read first bit cycle
			   	   }                               //wait for SCL to go low


		//read only 7 more bits of data because bit 1 already read
		data <<=7;
		data|= read_7bits();
		data_reg[reg_address] = data; //write data to data_register at register address

		send_ACK();
	}
	reset_slave();
}


int main(){
	//I2C slave must be setup and also set a slave address
	i2c_slave_setup();  //run setup code
	slave_address = 80; //set slave address

	//data_reg is 256 bytes big and master or slave can read/write to it.
	data_reg[100] = 120; //place a value in register address 100

    while (1){
    	// foreground code goes here
    }
}
