#include <stm32f0xx.h>
void ow_b_reset(void); //reset bus or start signal for send data.
uint16_t ow_b_getData16(void); //get 2 byte from bus;
uint8_t ow_b_getData8(void); //get 1 byte
uint32_t ow_b_getData32(void); //get 4 byte;
uint64_t ow_b_getData64(void);
void ow_b_sendData(uint8_t data); //send 1 byte to bus;
void ow_b_sendTo(uint64_t to,uint16_t data);
void ow_b_sendAll(uint16_t data);
void ow_b_setAutoReset(unsigned char f);
uint16_t* ow_b_getAllDevices(void);
void ow_b_init(void);
unsigned char ow_b_getAutoReset(void);
uint8_t ow_b_cheack(void);
