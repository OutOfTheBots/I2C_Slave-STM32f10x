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
