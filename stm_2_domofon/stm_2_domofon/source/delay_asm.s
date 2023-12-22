	AREA    |.text|, CODE, READONLY
	EXPORT _Delay_us
_Delay_us proc
	;ldr r1,=0x01
	;ldr r2,=0x01
	movs r0, r0, lsl #2
while
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	
	subs r0, r0, #1
	;add r1,r2
	;cmp r1,r0
	bne while
	mov PC, LR
	endp
	end
		