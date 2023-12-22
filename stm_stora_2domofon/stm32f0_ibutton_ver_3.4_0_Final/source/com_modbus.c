#include "modbus.h"
#include <stm32f0xx.h>
#include "com_modbus.h"
#include "commands.h"
#include "param_flash.h"

// **** ????????? ModBus
extern UART_DATA uart1;

void  com_modbus6 (uint16_t adr_reg)
{
	//*t_up_m = Read16ParamFlash(2);
	//*t_down_m = Read16ParamFlash(3);
	if (adr_reg < 10){
		Write16ParamFlash(adr_reg, uart1.reg[adr_reg]);
	}
	 if (adr_reg == 11){
		 // Если пришла команда закрыть вентиль
		if (uart1.reg[11] == 2){
			commands_modbus(2);
		}
		else if (uart1.reg[11] == 1){
			commands_modbus(1);			
		}
		else if (uart1.reg[11] == 3){
			commands_modbus(3);			
		}
		uart1.reg[11] = 0;
	}
	else if (adr_reg == 12){
		// Если пришла команда открыть вентиль
		if (uart1.reg[12]){
			commands_modbus(1);			
		}
		uart1.reg[12] = 0;
	}
	else if (adr_reg == 13){
		// Если пришла команда открыть вентиль
		if (uart1.reg[13]){
			commands_modbus(3);		// Стоп			
			uart1.reg[13] = 0;
		}
	}
}
