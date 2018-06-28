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
*/


unsigned char slave_address;
unsigned char* data_reg[256];
unsigned int time_out = 0;

#define wait_SCL_low() \
	to_count = 0; \
	while (GPIOA->IDR & GPIO_IDR_IDR1){ \
		to_count++; \
		if (to_count > time_out){ \
			reset_slave(); \
			return; \
		} \
	} \

#define wait_SCL_high() \
		to_count = 0; \
		while (!(GPIOA->IDR & GPIO_IDR_IDR1)){ \
			to_count++; \
			if (to_count > time_out){ \
				reset_slave(); \
				return; \
			} \
		} \

#define send_ACK() \
	GPIOA->CRL |= GPIO_CRL_MODE0; \
	GPIOA->ODR &= ~GPIO_ODR_ODR0; \
	wait_SCL_high(); \
	wait_SCL_low (); \
	GPIOA->CRL &= ~GPIO_CRL_MODE0; \

#define reset_slave() \
	EXTI->PR |= EXTI_PR_PR0;  \
	EXTI->IMR |= EXTI_IMR_MR0;  \

#define read_slave_addrs() \
	sent_slave = 0; \
	for (char i=0; i<7; ++i){ \
		wait_SCL_high(); \
		sent_slave <<=1; \
		sent_slave |= (GPIOA->IDR & GPIO_IDR_IDR0); \
		wait_SCL_low(); \
	} \

#define read_7bits() \
		for (char i=0; i<7; ++i){ \
			wait_SCL_high();  \
			data <<=1; \
			data |= (GPIOA->IDR & GPIO_IDR_IDR0); \
			wait_SCL_low(); \
		} \

#define read_rw_bit() \
		wait_SCL_high(); \
		rw = (GPIOA->IDR & GPIO_IDR_IDR0); \
		wait_SCL_low(); \


#define send_8bits() \
	GPIOA->CRL |= GPIO_CRL_MODE0; \
	for (char i=0; i<8; ++i){ \
		GPIOA->ODR &= ~GPIO_ODR_ODR0; \
		GPIOA->ODR |= ((data & 0x80)>>7); \
		data <<=1; \
		wait_SCL_high(); \
		wait_SCL_low(); \
	} \
	GPIOA->CRL &= ~GPIO_CRL_MODE0; \

#define read_reg_addr() \
	reg_address = 0; \
	for (char i=0; i<8; ++i){ \
		wait_SCL_high(); \
		reg_address <<=1; \
		reg_address|= (GPIOA->IDR & GPIO_IDR_IDR0); \
		wait_SCL_low(); \
	} \

void i2c_slave_setup (void);
