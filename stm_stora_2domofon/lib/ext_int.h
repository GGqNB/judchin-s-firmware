#include <stm32f0xx.h>


void set_flag (void);
void clear_flag (void);
unsigned char get_flag (void);
int check_new_buf (void);	// проверить буфер на повтор
void send_p (unsigned char* buf_send);
void EXTI4_15_IRQHandler(void);
