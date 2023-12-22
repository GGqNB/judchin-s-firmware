#include "1-wire_b.h"
#include <stdio.h>
#include <stdlib.h>
#include <main.h>
#include <stm32f0xx.h>
#include "delay.h"

unsigned char ow_b_AutoReset=0;
//uint16_t* ow_b_list_devices;

void ow_b_init(void)
{
	//ow_b_list_devices=malloc(sizeof(uint16_t)*10);
	ow_b_AutoReset=0;
	ow_b_setAutoReset(1);
}

void ow_b_setAutoReset(unsigned char f)
{
	ow_b_AutoReset=f==0 ? 0:1;
}
unsigned char ow_b_getAutoReset(void)
{
	return ow_b_AutoReset;
}
void ow_b_reset() 
{ 	
	//GPIOA->ODR = GPIO_Pin_0;
	//GPIOA->BSRR = GPIO_BSRR_BS_1;
	GPIOA->BSRR=GPIO_BSRR_BR_1;
	Delay_ms(50);//500us
	GPIOA->BSRR=GPIO_BSRR_BS_1;
	
	//GPIOA->ODR = 0;
	//GPIOA->BSRR = GPIO_BSRR_BR_1;
	//GPIOA->BSRR=GPIO_BSRR_BS_1;
	Delay_ms(50); //500us
	//Delay_ms(15); //150us
	//GPIOA->ODR = GPIO_Pin_0;
	//GPIOA->BSRR = GPIO_BSRR_BS_1;
	Delay_ms(1);
}
	
void ow_b_write_bit(uint8_t bit)
{
	GPIOA->BSRR = GPIO_BSRR_BR_1;
	_Delay_us(bit ? 2 : 90);//2 ? 90 us
	GPIOA->BSRR=GPIO_BSRR_BS_1;
	_Delay_us(bit ? 90 : 2); //us 90 ? 2 us
}



uint8_t ow_b_read_bit()
{
	uint8_t bit = 0;
	GPIOA->BSRR = GPIO_BSRR_BR_1;
	_Delay_us(2);
	
	GPIOA->MODER &= ~GPIO_MODER_MODER1;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR1;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR1_1;
	_Delay_us(18);
	
	bit = (GPIOA->IDR&GPIO_Pin_1?1:0); //_3?
	GPIOA->MODER |= GPIO_MODER_MODER1_0;
	_Delay_us(40); //40 us 2+18+40=60us
	
	GPIOA->BSRR = GPIO_BSRR_BS_1;
	_Delay_us(1);
	return bit;
}

void ow_b_sendData(uint8_t data)
{
	uint8_t i;
	for(i= 0; i<8; i++) 		
		ow_b_write_bit(data>>i & 1);
}

uint16_t ow_b_getData16 (void)
{
	uint8_t i;
	uint16_t data = 0;
	for(i = 0; i<16; i++) data += (uint16_t)ow_b_read_bit()<<i;
	return data;
}
uint8_t ow_b_getData8 (void)
{
	uint8_t i;
	uint8_t data = 0;
	for(i = 0; i<8; i++) data += (uint8_t)ow_b_read_bit()<<i;
	return data;
}
uint32_t ow_b_getData32 (void)
{
	uint8_t i;
	int data = 0;
	for(i = 0; i<32; i++) data += (int)ow_b_read_bit()<<i;
	return data;
}
uint64_t ow_b_getData64(void)
{
	uint8_t i;
	uint64_t data = 0;
	for(i = 0; i<64; i++) data += (uint64_t)ow_b_read_bit()<<i;
	return data;
}
void ow_b_sendAll(uint16_t data)
{
	if(ow_b_AutoReset)ow_b_reset();
	ow_b_sendData(0xcc);
	ow_b_sendData(data);
}

void ow_b_sendTo(uint64_t to,uint16_t data)
{
	int i;
	if(ow_b_AutoReset)ow_b_reset();
	ow_b_sendData(0x55);
	for(i=0;i<8;i++)ow_b_sendData((uint8_t)(to>>8*i));
}


uint8_t ow_b_cheack(void)//not test
{
	uint8_t bit = 0;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;
	bit = (GPIOA->IDR&GPIO_Pin_0?1:0);
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	return bit;
}


