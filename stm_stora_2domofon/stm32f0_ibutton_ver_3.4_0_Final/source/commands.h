#include <stm32f0xx.h>

void close_lock (void);
void open_lock (void);

void commands_modbus (uint8_t b);
void commands (uint8_t *b);
