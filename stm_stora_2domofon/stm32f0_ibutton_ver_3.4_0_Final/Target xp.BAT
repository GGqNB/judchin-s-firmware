SET PATH=C:\Keil\ARM\ARMCC\bin\;C:\PROGRA~1\Borland\CBUILD~1\Bin;C:\PROGRA~1\Borland\CBUILD~1\Projects\Bpl;C:\WinAVR-20100110\bin;C:\WinAVR-20100110\utils\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\Python26;C:\Program Files\ATI Technologies\ATI.ACE\Core-Static;C:\Program Files\ATI Technologies\ATI Control Panel;c:\qt\2010.01\qt\bin\;c:\altera\91\quartus\bin;C:\Program Files\CodeSourcery\Sourcery G++ Lite\bin;C:\Program Files\Raisonance\Ride\bin;C:\Program Files\Raisonance\Ride\arm-gcc\bin;C:\Program Files\QuickTime\QTSystem\;C:\Program Files\IVI Foundation\VISA\WinNT\Bin;C:\Program Files\CodeSourcery\Sourcery G++ Lite\bin;C:\Python26;c:\qt\2010.01\qt\bin\
SET CPU_TYPE=STM32F050F4
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=Target 1
SET CPU_CLOCK=0x02DC6C00
del ".\build\*.o"
del ".\build\*.hex"
del ".\build\*.axf"
"C:\Keil\ARM\ARMCC\bin\ArmCC" --Via ".\build\main.__i"
"C:\Keil\ARM\ARMCC\bin\ArmAsm" --Via ".\build\startup_stm32f0xx._ia"
"C:\Keil\ARM\ARMCC\bin\ArmCC" --Via ".\build\system.__i"
"C:\Keil\ARM\ARMCC\bin\ArmCC" --Via ".\build\usart.__i"
"C:\Keil\ARM\ARMCC\bin\ArmCC" --Via ".\build\led.__i"
"C:\Keil\ARM\ARMCC\bin\ArmCC" --Via ".\build\nrf24l01.__i"
"C:\Keil\ARM\ARMCC\bin\ArmLink" --Via ".\build\f0ledblink.lnp"
"C:\Keil\ARM\ARMCC\bin\fromelf.exe" ".\build\f0ledblink.axf" --i32combined --output ".\build\f0ledblink.hex"
C:\Keil\ARM\ARMCC\bin\fromelf.exe --bin --output=.\build\f0ledblink.bin .\build\f0ledblink.axf
pause

