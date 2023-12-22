#include <stm32f0xx.h>
void ow_a_reset(void); //reset bus or start signal for send data.
uint16_t ow_a_getData16(void); //get 2 byte from bus;
uint8_t ow_a_getData8(void); //get 1 byte
uint32_t ow_a_getData32(void); //get 4 byte;
uint64_t ow_a_getData64(void);
void ow_a_sendData(uint8_t data); //send 1 byte to bus;
void ow_a_sendTo(uint64_t to,uint16_t data);
void ow_a_sendAll(uint16_t data);
void ow_a_setAutoReset(unsigned char f);
uint16_t* ow_a_getAllDevices(void);
void ow_a_init(void);
unsigned char ow_a_getAutoReset(void);
uint8_t ow_a_cheack(void);
