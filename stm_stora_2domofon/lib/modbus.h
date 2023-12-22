#include <stm32f0xx.h>
#include "usart.h"
#include "com_modbus.h"

//#define adr 26		// адрес modbus
#define reg_col 64	// количество регистров в адресном пространстве modbus

unsigned int Crc16 (unsigned char *ptrByte, int byte_cnt);
void modbus_3_4 (void);
void modbus_6 (void);
void modbus_16 (void);
void modbus_slave (void);

	
typedef struct {
	unsigned int rxtimer;	// *** этим мы считаем таймаут (не используется т.к. используется аппаратный таймаут по приему)
	unsigned char delay;	// *** задержка (не используется т.к. используется аппаратный таймаут по приему)
	unsigned char buffer[256];//буфер для хранения запроса
	uint16_t reg [reg_col];//буфер для хранения запроса
	unsigned char rxcnt; 	//количество принятых символов
	unsigned char txcnt;	//количество переданных символов
	unsigned char txlen;	//длина посылки на отправку
	unsigned char rxgap;	//окончание приема
} UART_DATA;
