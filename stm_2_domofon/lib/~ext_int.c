#include "nRF24L01.h"
#include "main.h"
#include "usart.h"
#include "param_flash.h"
#include "main1.h"
#include "commands.h"
#include <delay.h>
#include "ext_int.h"
#include "pass.h"
//void set_flag (void);

extern unsigned char buf[32];
//extern char pass[];
extern unsigned char flag_1;

//__align(4) unsigned char buf_send[32] = {0};		// буффер для отправки сообщений. выровнен по границе 4 байт
int buf_p;				// переменная для хранения прошлого пакета
int *point_buf;
char buf_p4; 			// переменная для хранения последнего байта данных

char rx_tx = 0; 			// переменная для хранения направления работы nrf24, 0 - прием, 1 - передача

unsigned char fff;
unsigned char ram_parametrs[128];
void set_flag (void)
{
	fff = 1;
}

void clear_flag (void)
{
	fff = 0;
}

unsigned char get_flag (void)
{
	return fff;
}
int check_new_buf (void)	// проверить буфер на повтор
{
	int flag;
	point_buf = (int*)buf; 	// получение указателя на буфер
	if  (!check_RX_timeout() && (buf_p == *point_buf) && (buf_p4 == buf[4]))  {		// Если повтор посылки за время timeout
		flag = 0;
		if (ReadParamFlash(debug)){
			USART_SendData(0xff);     
		}
	}
	else {
		flag = 1;
	}
	
	// *** копирование посылки для проверки повторов от ретранслятора *** 
	buf_p = *point_buf;
	buf_p4 = buf[4];			

	return flag;
}

void send_p (unsigned char* buf_s)
{
	TX_Mode(RF_DR_250kbps); 
	rx_tx = 1;
	// ***** вычисление CRC *****
	CRC->CR |= CRC_CR_RESET; //Делаем сброс CRC	
	point_buf = (int*)(buf_s); 	// получение указателя на буфер
	CRC->DR = *point_buf;
	CRC->DR = buf_s[4];
	
	/*for (i=0; i<5; i++){
		CRC->DR = buf_send[i]; //Загоняем данные из буфера в регистр данных CRC
	}*/
	// ***** дополнение CRC к пакету *****
	buf_s[5]=((CRC->DR)>>24)&0xff; // 
	buf_s[6]=((CRC->DR)>>16)&0xff;
	buf_s[7]=((CRC->DR)>>8)&0xff;
	buf_s[8]=(CRC->DR)&0xff;
	//count = 9;
	
	point_buf = (int*)(buf_s); 	// получение указателя на буфер
	point_pass = (int*)(pass);
	
	point_buf[0]  = point_buf[0] ^ point_pass[0];
	point_buf[1]  = point_buf[1] ^ point_pass[1];
	
	//*point_buf  = *point_buf ^ *point_pass;
	buf_s[8] ^= pass[8];
	
	NRF_Send (buf_s, 9); // отправка пакета
	//flag_1 = 0;         // обнуление флага готовности буфера
}

void EXTI4_15_IRQHandler(void)
{
	int crc_reg;
	char deb;
	deb = ReadParamFlash(debug);
	if (deb) {
		USART_SendData(0x11);     
	}
	if (get_status() & 3) {
		#if mode 
			RX_Mode(RF_DR_250kbps);
			rx_tx = 0;
		#endif
		set_flag();
		//USART_SendData(0x11);     
	}
	while (NRF_Receive()){
		CRC->CR |= CRC_CR_RESET; //Делаем сброс CRC
		
		point_buf = (int*)(buf); 	// получение указателя на буфер
		point_buf[0]  = point_buf[0] ^ point_pass[0];
		point_buf[1]  = point_buf[1] ^ point_pass[1];
		
		//*point_buf  = *point_buf ^ *point_pass;
		buf[8] ^= pass[8];
		
		point_buf = (int*)(buf); 	// получение указателя на буфер
		CRC->DR = *point_buf;
		CRC->DR = buf[4];
		
		// проверка принятой CRC с посчитанной 
		crc_reg = buf[5] << 24 | buf[6] << 16 | buf[7] << 8 | buf[8] ;
		if ( (crc_reg == CRC->DR)) {
			if (deb == 2) {			// если включена полная отладка 
				USART_SendData(buf[0]);
				USART_SendData(buf[1]);
				USART_SendData(buf[2]);
				USART_SendData(buf[3]);
				USART_SendData(buf[4]);				
			}
				
			if (check_new_buf() ) {		// маскирование дублирующихся пакетов, для корректной маршрутизации
				if  ( (ReadParamFlash(adr) == 1) && (deb != 2)) {		// если модуль сервера, то весь пакет отправляем на UART, вне зависимости от получателя
					USART_SendData(buf[0]);
					USART_SendData(buf[1]);
					USART_SendData(buf[2]);
					USART_SendData(buf[3]);
					USART_SendData(buf[4]);
				}
				if ( buf[0] == ReadParamFlash(adr) ){ 	// если пакет пришел нам и он не является дублирующим //
					if (deb) {
						USART_SendData(0x22);     
					}
					
					if (ReadParamFlash(adr) == 1) {		// если модуль сервера
						flag_1 &= ~1;						// сервер не отвечает на запросы и не воспринимает команды						
					}
					else {
						if (buf[2] & 0x80)				// если ответ на наш запрос
							flag_1 &= ~1; 				// то не отвечаем на него
						else 
							flag_1 |= 1;				// предполагается, что будет ответ. если ответа не будет, то flag_1 очищается в обработчике команды
						
						// ***** общие команды для всех *****
						if (buf[2] == 1){					// запрос версии
							buf[3] = 0;
							buf[4] = version;
						}
						else if (buf[2] == 0x0F){	// пинг
						}
						else if ((buf[2] == 0x16) && (buf[1] == 1)){ 	// записать параметр на FLASH-память. Запись производится только от имени сервера
							if (buf[3] < 128) {
								WriteParamFlash(buf[3],buf[4]);
								buf[4] = ReadParamFlash(buf[3]);
							}
							else {
								ram_parametrs[buf[3] - 128] = buf[4];
								buf[4] = ram_parametrs[buf[3] - 128] = buf[4];
							}
						}
						else if (buf[2] == 0x17){ 	// считать параметр из FLASH-памяти
							if (buf[3] < 128) {
								buf[4] = ReadParamFlash(buf[3]);
							}
							else{
								buf[4] = ram_parametrs[buf[3]-128];
								ram_parametrs[buf[3]-128] = 0;
							}
						}
						else {
							// ***** остальные команды уникальные для устройств *****
							commands();			
						}
						buf[0] = buf[1];        		//  адрес mastera
						buf[1] = ReadParamFlash(adr);	// задаем отправителя (себя)	
						buf[2] |= 128;					// для ответа устанавливаем старший бит в поле команда.  
					}
					
				}
				else if (ReadParamFlash(retr)){		// если ретрансляция включена
					flag_1 |= 2;
					ram_parametrs[0]++;
					if (deb) 
						USART_SendData(0xdd);     
				}
				else if (buf[0] == 0) { // Широковещательный пакет
					// тут будет обработчик
				}
			}
			else {
				ram_parametrs[1]++;
			}
		}
		else {
			USART_SendData(0xee);
		}
	}
	clearFlag();		// на всякий случай очищаем флаги, т.к. проверяется только 1 прерывание.	
	EXTI->PR |= EXTI_PR_PR14;	
}
