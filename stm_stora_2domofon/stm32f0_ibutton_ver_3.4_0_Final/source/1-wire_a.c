#include "1-wire_a.h"
#include <stdio.h>
#include <stdlib.h>
#include <main.h>
#include <stm32f0xx.h>
#include "delay.h"

//extern void _Delay(int i);
unsigned char ow_a_AutoReset=0;
//uint16_t* ow_a_list_devices;

void ow_a_init(void)
{
	//ow_a_list_devices=malloc(sizeof(uint16_t)*10);
	ow_a_AutoReset=0;
	ow_a_setAutoReset(1);
}

void ow_a_setAutoReset(unsigned char f)
{
	ow_a_AutoReset=f==0 ? 0:1;
}
unsigned char ow_a_getAutoReset(void)
{
	return ow_a_AutoReset;
}
void ow_a_reset() 
{ 	
	//GPIOA->ODR = GPIO_Pin_0;
	//GPIOA->BSRR = GPIO_BSRR_BS_0;
	GPIOA->BSRR=GPIO_BSRR_BR_0;
	_Delay_us(500);//500us
	GPIOA->BSRR=GPIO_BSRR_BS_0;
	
	//GPIOA->ODR = 0;
	//GPIOA->BSRR = GPIO_BSRR_BR_0;
	//GPIOA->BSRR=GPIO_BSRR_BS_0;
	_Delay_us(500); //500us
	//Delay_ms(15); //150us
	//GPIOA->ODR = GPIO_Pin_0;
	//GPIOA->BSRR = GPIO_BSRR_BS_0;
	_Delay_us(1);
}
	
void ow_a_write_bit(uint8_t bit)
{
	GPIOA->BSRR = GPIO_BSRR_BR_0;
	_Delay_us(bit ? 2 : 90);//2 ? 90 us
	GPIOA->BSRR=GPIO_BSRR_BS_0;
	_Delay_us(bit ? 90 : 2); //us 90 ? 2 us
	
}

uint8_t ow_a_read_bit()
{
	uint8_t bit = 0;
	GPIOA->BSRR = GPIO_BSRR_BR_0;
	_Delay_us(2);
	
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;
	_Delay_us(18);
	
	bit = (GPIOA->IDR&GPIO_Pin_0?1:0); //_3?
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	_Delay_us(40); //40 us 2+18+40=60us
	
	GPIOA->BSRR = GPIO_BSRR_BS_0;
	_Delay_us(1);
	return bit;
}

void ow_a_sendData(uint8_t data)
{
	uint8_t i;
	for(i= 0; i<8; i++) 		
		ow_a_write_bit(data>>i & 1);
}

uint16_t ow_a_getData16 (void)
{
	uint8_t i;
	uint16_t data = 0;
	for(i = 0; i<16; i++) data += (uint16_t)ow_a_read_bit()<<i;
	return data;
}
uint8_t ow_a_getData8 (void)
{
	uint8_t i;
	uint8_t data = 0;
	for(i = 0; i<8; i++) data += (uint8_t)ow_a_read_bit()<<i;
	return data;
}
uint32_t ow_a_getData32 (void)
{
	uint8_t i;
	int data = 0;
	for(i = 0; i<32; i++) data += (int)ow_a_read_bit()<<i;
	return data;
}
uint64_t ow_a_getData64(void)
{
	uint8_t i;
	uint64_t data = 0;
	for(i = 0; i<64; i++) data += (uint64_t)ow_a_read_bit()<<i;
	return data;
}
void ow_a_sendAll(uint16_t data)
{
	if(ow_a_AutoReset)ow_a_reset();
	ow_a_sendData(0xcc);
	ow_a_sendData(data);
}

void ow_a_sendTo(uint64_t to,uint16_t data)
{
	int i;
	if(ow_a_AutoReset)ow_a_reset();
	ow_a_sendData(0x55);
	for(i=0;i<8;i++)ow_a_sendData((uint8_t)(to>>8*i));
}

uint8_t ow_a_cheack(void)//not test
{
	uint8_t bit = 0;
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPDR0;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1;
	bit = (GPIOA->IDR&GPIO_Pin_0?1:0);
	GPIOA->MODER |= GPIO_MODER_MODER0_0;
	return bit;
}


