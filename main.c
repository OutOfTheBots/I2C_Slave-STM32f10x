#include "stm32f10x.h"
#include "I2C_slave.h"

void i2c_slave_setup (void){
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; //enable port A
	GPIOA->CRL &= ((~GPIO_CRL_CNF0) | (GPIO_CRL_CNF0_0)) & (~GPIO_CRL_MODE0); //set SDA as input float
	GPIOA->CRL &= ((~GPIO_CRL_CNF1) | (GPIO_CRL_CNF1_0)) & (~GPIO_CRL_MODE1); //set SCL as input float

	//wait till both SDA and SCL go high
	//while ( !(GPIOA->IDR & GPIO_IDR_IDR0)  ||  !(GPIOA->IDR & GPIO_IDR_IDR1) ){
	//}

	AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PA;  //enable interupt on port A
	EXTI->FTSR |= EXTI_FTSR_TR0; //set falling trigger SDA
	EXTI->IMR |= EXTI_IMR_MR0;  //enable interupt on SDA
	NVIC_EnableIRQ(EXTI0_IRQn);  //set NVIC callbacks line 0
	NVIC_SetPriority(EXTI0_IRQn, 0); //set piroty
}

void EXTI0_IRQHandler(void){
	unsigned char rw;
	unsigned char sent_slave;
	unsigned int to_count;
	unsigned char data = 0;
	unsigned char reg_address;
	if (GPIOA->IDR & GPIO_IDR_IDR1){  //check for start condition
		EXTI->IMR &= ~EXTI_IMR_MR0; //disable interupt on SDA

		wait_SCL_low();

		read_slave_addrs();
		if (!(sent_slave == slave_address)){ //not our slave address reset and return
			reset_slave();
			return;
		}

		read_rw_bit();
		if (rw){ //if rw=1 then there is an error, reset and return
			reset_slave();
			return;
		}

		send_ACK();

		read_reg_addr();

		send_ACK();

		//check for second start condition
		wait_SCL_high();

		data = (GPIOA->IDR & GPIO_IDR_IDR0); //read 1 bit in case not a repeat start condition

		if (GPIOA->IDR & GPIO_IDR_IDR0){  //possible repeat start
			to_count = 0;
			while ((GPIOA->IDR & GPIO_IDR_IDR0) && (GPIOA->IDR & GPIO_IDR_IDR1)){ //wait till a line goes low
				to_count++;
				if (to_count > time_out){
					reset_slave();
					return;
				}
			}
			if (!(GPIOA->IDR & GPIO_IDR_IDR0)  && (GPIOA->IDR & GPIO_IDR_IDR1)){ //repeat start condition meet
				wait_SCL_low();

				//all code here only executes if a repeat start condition i.e master read sequence

				//read and check 7 bit slave address
				read_slave_addrs();
				if (!(sent_slave == slave_address)){ //not our slave address reset and return
					reset_slave();
					return;
				}

				read_rw_bit();
				if (!(rw)){ //if rw=0 then there is an error, reset and return
					reset_slave();
					return;
				}

				send_ACK();

				while (1) {

					//write 8 bits of data
					data = data_reg[reg_address ];
					send_8bits();
					reg_address++;

					if ((reg_address ) > 255){ //past end of data register
						reset_slave();
						return;
					}

					//check for ACK or NACK
					wait_SCL_high();
					if (GPIOA->IDR & GPIO_IDR_IDR0){  //recived a NACK
						reset_slave();
						return;
					}
					wait_SCL_low();
				}
			}
		}


		//code below here only executes if there isn't a repeat start condition i.e master write sequence
		else{
			wait_SCL_low();
		}

		while (1){

			//read only 7 more bits of data because bit 1 already read
			read_7bits();

			data_reg[reg_address] = data;
			reg_address++;

			if (reg_address > 255){ //overflow data register
				reset_slave();
				return;
			}

			send_ACK();

			//check for stop condition
			wait_SCL_high();

			data = GPIOA->IDR & GPIO_IDR_IDR0; //read 1 bit in case not a stop condition
			if (!(GPIOA->IDR & GPIO_IDR_IDR0)){  //possible stop
				to_count = 0;
				while ((!(GPIOA->IDR & GPIO_IDR_IDR0)) && (GPIOA->IDR & GPIO_IDR_IDR1)){ //wait till a line changes
					to_count++;
					if (to_count > time_out){
						reset_slave();
						my_print("time out");
						return;
					}
				}
					if (GPIOA->IDR & GPIO_IDR_IDR0){ //stop condition meet
						reset_slave();
						return;
					}
			}
			else{
				wait_SCL_low();
			}
		}
	}
	reset_slave();
}


void main(void){
	i2c_slave_setup();
    slave_address = 80;
    time_out = 2000;

    while (1){
    	//foreground code goes here

    }
}
