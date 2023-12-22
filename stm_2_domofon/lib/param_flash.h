#include <stm32f0xx.h>

#define COUNT_PARAM           0x64        // Количество параметров, хранимых на FLASH-памяти
#define BEGIN_ADDRESS_PARAM   0x8003C00   // Начальный адрес страницы Flash с параметрами (последняя 16 страница)


// Разблокировка FLASH-памяти
void FLASH_Unlock(void);
// Блокировка FLASH-памяти
void FLASH_Lock(void);
// Очистка страницы FLASH-памяти
void FLASH_ClearPage(uint32_t Page_Address);
// Чтение из FLASH-памяти 2 байт
uint16_t FLASH_Read(uint32_t Address);
// Запись в FLASH-память 2 байт
void FLASH_Write(uint32_t Address, uint16_t Data);
// чтение из FLASH-памяти одного параметра
uint8_t ReadParamFlash(uint8_t NUM_PARAM);
// запись параметра в FLASH-памяти
int WriteParamFlash(uint8_t NUM_PARAM, uint16_t VALUE);
void WriteMultiplyParamFlash(uint16_t* param);
int Write32ParamFlash(uint32_t VALUE, uint8_t n);
int Write32ParamFlash_scan(uint32_t VALUE);
uint16_t Read16ParamFlash(uint8_t NUM_PARAM);

