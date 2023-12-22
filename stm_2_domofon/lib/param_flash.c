// файл для работы с параметрами, которые хранятся в FLASH-памяти

#include "param_flash.h"

// Разблокировка FLASH-памяти
void FLASH_Unlock(void)
{
	if((FLASH->CR & FLASH_CR_LOCK) != RESET){
		FLASH->KEYR = FLASH_FKEY1;
		FLASH->KEYR = FLASH_FKEY2;
	}
}

// Блокировка FLASH-памяти
void FLASH_Lock(void)
{
	FLASH->CR |= FLASH_CR_LOCK;
}

// Очистка страницы FLASH-памяти
void FLASH_ClearPage(uint32_t Page_Address)
{
	FLASH->CR|= FLASH_CR_PER; //Устанавливаем бит стирания одной страницы
	FLASH->AR = Page_Address; // Задаем её адрес
	FLASH->CR|= FLASH_CR_STRT; // Запускаем стирание
	while ((FLASH->SR&FLASH_SR_BSY)); //Ждем пока страница сотрется.
	FLASH->CR&= ~FLASH_CR_PER; //Сбрасываем бит обратно
}

// Чтение из FLASH-памяти 2 байт
uint16_t FLASH_Read(uint32_t Address)
{
	return (*(__IO uint16_t*)Address);
}

uint16_t Read16ParamFlash(uint8_t NUM_PARAM)
{
	return ((FLASH_Read(BEGIN_ADDRESS_PARAM + (NUM_PARAM << 1) ) )) ; 	// маскирование последнего бита номера параметра, чтобы он был четным
}


// запись слова в FLASH-памяти
int Write32ParamFlash_scan(uint32_t VALUE)
{
	//uint16_t buf_param[250] = {0};  // Буфер для хранения параметров.  одная ячейка -- два параметра
	//uint16_t buf_mask_address[256] = {0};  // Буфер для хранения параметров
	uint16_t i, x, y;
	
    for (i=0; i<250; i++){
        x = Read16ParamFlash( (i) << 2 );
        y = Read16ParamFlash( ((i) << 2 ) + 2);
        
        if (((y << 16) | x) == 0xffff){
            FLASH_Unlock();            
            for (i=0; i < COUNT_PARAM >> 1; i++) {
                x = VALUE & 0xffff;
                y = VALUE >> 16;
                FLASH_Write(BEGIN_ADDRESS_PARAM + (i << 2) , x);
                FLASH_Write(BEGIN_ADDRESS_PARAM + ( (i << 2) + 2) , y);
            }
            FLASH_Lock();
            return 0;
        }
    }
	return 1;
}

int Write32ParamFlash(uint32_t VALUE, uint8_t n)
{
	//uint16_t buf_param[250] = {0};  // Буфер для хранения параметров.  одная ячейка -- два параметра
	//uint16_t buf_mask_address[256] = {0};  // Буфер для хранения параметров
	//uint16_t x, y;
	
    
	//x = Read16ParamFlash( (n) << 2 );
	//y = Read16ParamFlash( ((n) << 2 ) + 2);
	
	
	FLASH_Unlock();            
	
	//x = VALUE & 0xffff;
	//y = VALUE >> 16;
	FLASH_Write(BEGIN_ADDRESS_PARAM + (n << 2) , VALUE & 0xffff);
	FLASH_Write(BEGIN_ADDRESS_PARAM + ( (n << 2) + 2) , VALUE >> 16);
	
	FLASH_Lock();
	return 1;
	
}

// Запись в FLASH-память 2 байт
void FLASH_Write(uint32_t Address, uint16_t Data)
{
	FLASH->CR |= FLASH_CR_PG; //Разрешаем программирование флеша
	while ((FLASH->SR&FLASH_SR_BSY)); //Ожидаем готовности флеша к записи
	*(__IO uint16_t*)Address = Data; //Пишем младшие 2 бата
	while ((FLASH->SR&FLASH_SR_BSY)); //Ожидаем окончания записи во флеш
	FLASH->CR &= ~(FLASH_CR_PG); //Запрещаем программирование флеша
}

// чтение из FLASH-памяти одного параметра
uint8_t ReadParamFlash(uint8_t NUM_PARAM)
{
	if (NUM_PARAM & 1) {		// если параметр из старшей части полуслова
	//return ((uint8_t)FLASH_Read(BEGIN_ADDRESS_PARAM+NUM_PARAM*2));
		return ((uint8_t) (FLASH_Read(BEGIN_ADDRESS_PARAM + (NUM_PARAM & 0xfe)) >> 8 )) ; 	// маскирование последнего бита номера параметра, чтобы он был четным
	}
	else {						// если параметр из младшей части полуслова
		return ((uint8_t) FLASH_Read(BEGIN_ADDRESS_PARAM + NUM_PARAM )); 	//
	}
}

// запись параметра в FLASH-памяти
int WriteParamFlash(uint8_t NUM_PARAM, uint16_t VALUE)
{
	uint16_t buf_param[COUNT_PARAM] = {0};  // Буфер для хранения параметров.  одная ячейка -- два параметра
	uint8_t i;
	uint16_t par;
	//NUM_PARAM = NUM_PARAM  << 1;
	
	par = Read16ParamFlash(NUM_PARAM);
	if (par == VALUE){ // во флеш уже лежит нужное значение. переписывать его нет необходимости
		return 1;
	}
	
	if (par == 0xffff) { 		// если ячейка параметра пустая, то нет смысла очищать всю страницу
		FLASH_Unlock();
		FLASH_Write(BEGIN_ADDRESS_PARAM + (NUM_PARAM << 1), VALUE);
		FLASH_Lock();
		return 2;
	}
		
	for (i=0; i< (COUNT_PARAM); i++) {
		buf_param[i] = FLASH_Read(BEGIN_ADDRESS_PARAM + (i<<1));
	}
	buf_param[NUM_PARAM] = VALUE;
	
	FLASH_Unlock();
	FLASH_ClearPage(BEGIN_ADDRESS_PARAM);
	for (i=0; i<COUNT_PARAM; i++) {
		FLASH_Write(BEGIN_ADDRESS_PARAM+i*2, buf_param[i]);
	}
	FLASH_Lock();
	return 0;
}


// запись параметров в FLASH-памяти
void WriteMultiplyParamFlash(uint16_t* param)
{
	uint16_t buf_param[COUNT_PARAM >> 1] = {0};  // Буфер для хранения параметров.  одная ячейка -- два параметра
	//uint16_t buf_mask_address[256] = {0};  // Буфер для хранения параметров
	uint8_t i;
	
	//if (FLASH_Read(BEGIN_ADDRESS_PARAM+i*2))
		
	for (i=0; i< (COUNT_PARAM >> 1); i++) {
		buf_param[i] = FLASH_Read(BEGIN_ADDRESS_PARAM+i*2);
	}
	//NUM_PARAM &= 0xfe;		// очистить младший бит номера параметра, сделать его всегда четным 
	for ( i = 0; i < 5; i++){
		buf_param[i] = param[i];
	}
	
	FLASH_Unlock();
	FLASH_ClearPage(BEGIN_ADDRESS_PARAM);
	for (i=0; i<COUNT_PARAM >> 1; i++) {
		FLASH_Write(BEGIN_ADDRESS_PARAM+i*2, buf_param[i]);
	}
	FLASH_Lock();
}

