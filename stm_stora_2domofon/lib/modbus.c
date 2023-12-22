#include "usart.h"
#include "modbus.h"
#include "main.h"

UART_DATA uart1;//структура для усарта
uint16_t *adr = &uart1.reg[0]; 

unsigned int Crc16 (unsigned char *ptrByte, int byte_cnt)
{
	unsigned int w=0;
	char shift_cnt;

	if(ptrByte) 	{
		w=0xffffU;
		for(; byte_cnt>0; byte_cnt--) {
			w=(unsigned int)((w/256U)*256U+((w%256U)^(*ptrByte++)));
			for(shift_cnt=0; shift_cnt<8; shift_cnt++){
				if((w&0x1)==1)
					w=(unsigned int)((w>>1)^0xa001U);
				else
					w>>=1;
			}
		}
	}
	return w;
}

void modbus_3_4 (void)
{
	int adr0, tmp, m, n;
	uint16_t tmp_val;
	
	adr0 = ((uart1.buffer[2]<<8)+uart1.buffer[3]);   
	//4-5 - number of registers
	tmp =((uart1.buffer[4]<<8)+uart1.buffer[5]);
	
	if ( (adr0 + tmp) <= (reg_col)) {	// если запрашиваемые регистры не превышают адресное пространство modbus
		n = 3;	//default answer length if no error
		for(m=0; m < tmp; m++){
			tmp_val=uart1.reg[m+adr0];
			uart1.buffer[n]=tmp_val>>8;
			uart1.buffer[n+1]=tmp_val;
			n=n+2;
		}

		uart1.buffer[2]=m*2; 	//byte count
		uart1.txlen=m*2+5;		//responce length
	}
	else {
		uart1.buffer[1] |= 0x80;	// флаг ошибки
		uart1.buffer[2] = 2; // 1 — Принятый код функции не может быть обработан
		uart1.txlen = 5;
	}
}

//*******************************************************
//Writing
//*******************************************************
void modbus_6 (void)
{
	uint16_t tmp;
   
   //2-3  - adress   , 4-5 - value
	tmp=((uart1.buffer[2]<<8)+uart1.buffer[3]); //adress

	uart1.txlen=uart1.rxcnt; //responce length
    uart1.reg[tmp]=(uart1.buffer[4]<<8)+uart1.buffer[5];
	
	com_modbus6(tmp);
}

void modbus_16 (void)
{   
	int adr0, tmp, m, n;
	//uint16_t tmp_val;
	uint8_t ct;
	
	//2-3  - adress 
	adr0 = ((uart1.buffer[2]<<8)+uart1.buffer[3]);   
	//4-5 - number of registers
	tmp =((uart1.buffer[4]<<8)+uart1.buffer[5]);
	
	if ( (adr0 + tmp) <= (reg_col)) {	// если запрашиваемые регистры не превышают адресное пространство modbus
		ct = uart1.buffer[6];
		n = 7;	//default start number if no error
		
		for(m=0; m < tmp; m++){
			uart1.reg[m+adr0] = ( uart1.buffer[n] << 8 ) +  uart1.buffer[n+1];
			n=n+2;
			com_modbus6(m+adr0);
		}
		uart1.txlen = 8;
	}
	else {
		uart1.buffer[1] |= 0x80;	// флаг ошибки
		uart1.buffer[2] = 2; // 1 — Принятый код функции не может быть обработан
		uart1.txlen = 5;
	}
}

void modbus_slave (void)
{
	unsigned int tmp;
	//WR_on();
	if (uart1.buffer[0] == ((*adr)&0xff)) {	// проверка адреса подчиненного устройства		
		tmp=Crc16(uart1.buffer,uart1.rxcnt-2);

		// проверка контрольной суммы (CRC16) для пришедшей посылки 
		if((uart1.buffer[uart1.rxcnt-2]==(tmp&0x00FF)) && (uart1.buffer[uart1.rxcnt-1]==(tmp>>8))) { // если CRC16 совпала, то обрабатываем пакет
		
			if (uart1.buffer[1] == 3){ 	//Read Holding Registers 
				modbus_3_4 ();
			}
			
			else if (uart1.buffer[1] == 4){ 	//Read Input Registers  
				modbus_3_4 ();
			}
			
			else if (uart1.buffer[1] == 6){ 	//Preset Single Register 
				modbus_6 ();
			}
			
			else if (uart1.buffer[1] == 16){ 	////Preset Multiple Register
				modbus_16 ();
			}
			else { 	// иначе ошибка (функция не поддерживается)
				uart1.buffer[1] |= 0x80;
				uart1.buffer[2] = 1; // 1 — Принятый код функции не может быть обработан
				uart1.txlen = 5;
			}
			
			tmp=Crc16(uart1.buffer,uart1.txlen-2);
			uart1.buffer[uart1.txlen-2]=tmp;
			uart1.buffer[uart1.txlen-1]=tmp>>8;
			uart1.txcnt=0;
			
			
			// разрешаем прерывание по отправке и запрещаем по приему
			USART1->ICR |= USART_ICR_TCCF;	// сброс флага (зачем?)
			
			USART1->CR1 |= USART_CR1_TCIE;
			USART1->CR1 &= ~USART_CR1_RE;
			
			WR_on();
			// отправляем первый байт 
			uart1.txcnt = 0;
			USART_SendData (uart1.buffer[uart1.txcnt]);			
		}
	}
	uart1.rxcnt = 0;
}
