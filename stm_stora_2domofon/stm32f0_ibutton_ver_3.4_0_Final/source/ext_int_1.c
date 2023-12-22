#include <se8r.h>
#include <stm32f0xx.h>
#include "main.h"
#include "usart.h"


extern unsigned char buf[20];
extern char adr, pass[];
extern unsigned char flag_1;
extern unsigned int temp_1, temp_2;
extern unsigned char flag_tim3;

void EXTI4_15_IRQHandler(void)
{
	int i, crc_reg ;
	
	//USART_SendData(0xaa);
	
	if (NRF_Receive()){
		CRC->CR |= CRC_CR_RESET; //Делаем сброс CRC
		for (i=0; i<9; i++){     // в цикле читаем принятый пакет
			buf[i]  = (buf[i] ^ pass[i]);   // сразу декодируем его
			if ( i < 5  )                   // для полезных данных высчитываем CRC
				CRC->DR = buf[i];           //Загоняем данные из буфера в регистр данных CRC
		}
		i = 0;
		crc_reg = buf[5] << 24 | buf[6] << 16 | buf[7] << 8 | buf[8] ;
		if (crc_reg == CRC->DR) {
		// проверка принятой CRC с посчитанной 
			if (buf[0] == adr )  {       // если адрес совпадает
				//USART_SendData(0xbb);
				buf[0] = buf[1];        // формирование адресов ответа: копирование адреса mastera
				buf[1] = adr;           // задаем отправителя (себя)
				// формат посылки: адр_slave (1) | адр_master (1) | команда_slave (1) | data (2) |
				// если data == 0x0, то выключить функцию
				//if ( func_status & (1 << buf[2] )) { // проверка поддержки этой функции
				if (buf[2] == 6){ 	// включить/выключить нагрузку
					if ( buf[4] & 0xf)  { // есть команда для включения устройств
						GPIOA->BSRR |= (buf[4] & 0xf);
					}
					if ( buf[4] & 0xf0) { // есть команда для выключения устройств
						GPIOA->BSRR |= (buf[4] & 0xf0) << 12;
					}
				}	
				else if (buf[2] == 12)
				{
					if (buf[3] == 0){
						temp_1 = buf[4];
					}
					else {
						temp_2 = buf[4];
					}
				}
				else if (buf[2]==7)
				{
					flag_tim3=buf[4];
				}
				else {
					buf[2] |= 0x80;       // установка 7 бита в поле команда (ошибка: функция запрещена)
				}
				flag_1 = 1;
			}
			else if (buf[0] == 0) { // Широковещательный пакет
				Delay_ms( adr * 10000 );
				buf[0] = 1;        //  адрес mastera
				buf[1] = adr;      // задаем отправителя (себя)
				buf[2] = 6;  
				buf[3] = 0; 
				buf[4] = 0; 
				flag_1 = 1;
			}
		}
	}
	// Сбрасываем флаг прерывания
	EXTI->PR |= EXTI_PR_PR14;
}