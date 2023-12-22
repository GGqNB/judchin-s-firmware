#include "nRF24L01.h"
#include <stm32f0xx.h>
#include "main.h"
#include "delay.h"
#include "usart.h"


//char TxBuf[32]={0};
//char RxBuf[32]={0};
//unsigned int sta;

unsigned char TX_ADDRESS[TX_ADR_WIDTH]  = {0x01,0x23,0x45,0x67,0x89}; // Define a static TX address
unsigned char rx_buf[TX_PLOAD_WIDTH];
unsigned char tx_buf[TX_PLOAD_WIDTH];
unsigned char flag;
//unsigned char rx_com_buffer[10];
//unsigned char tx_com_buffer[10];
//unsigned char i;
//unsigned char accept_flag;
__align(4) unsigned char buf[32] = {0};		// буффер выровнен по границе 4 байт
//unsigned char buf_p[32] = {0};

unsigned char reg_status;    // для хранения значения статуса в момент прерывания
//unsigned char flag_irq;      // флаг наступления прерывания IRQ_15

extern char adr;
// SPI(nRF24L01) commands

#define RF_READ_REG    0x00  // Define read command to register
#define RF_WRITE_REG   0x20  // Define write command to register
#define RD_RX_PLOAD 0x61  // Define RX payload register address
#define WR_TX_PLOAD 0xA0  // Define TX payload register address
#define FLUSH_TX    0xE1  // Define flush TX register command
#define FLUSH_RX    0xE2  // Define flush RX register command
#define REUSE_TX_PL 0xE3  // Define reuse TX payload register command
#define NOP         0xFF  // Define No Operation, might be used to read status register

#define  RX_DR  ((sta>>6)&0X01)
#define  TX_DS  ((sta>>5)&0X01)
#define  MAX_RT  ((sta>>4)&0X01)


//Chip Enable Activates RX or TX mode
#define CE_H()   GPIOA->BSRR = GPIO_BSRR_BS_13;
#define CE_L()   GPIOA->BSRR = GPIO_BSRR_BR_13;

//SPI Chip Select
#define CSN_H()  GPIOA->BSRR = GPIO_BSRR_BS_4;
#define CSN_L()  GPIOA->BSRR = GPIO_BSRR_BR_4;


/*******************************************************************************
* Function Name   : SPI1_RW
* Description : Sends a byte through the SPI interface and return the byte
*                received from the SPI bus.
* Input       : byte : byte to send.
* Output       : None
* Return       : The value of the received byte.
*******************************************************************************/
char SPI1_readWrite(char byte)
{
	/* Loop while DR register in not emplty */
	//while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    while (!(SPI1->SR & SPI_SR_TXE)); 
    SPI1_DR_8b = byte;
	/* Wait to receive a byte */
	//while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    
    while(!(SPI1->SR & SPI_SR_RXNE));
    /* Return the byte read from the SPI bus */
    return SPI1->DR;
}

unsigned char SPI1_readWriteReg(unsigned char reg, unsigned char value)
{
	unsigned char status;
	CSN_L();
	// select register 
	status = SPI1_readWrite(reg);
	// set value
	SPI1_readWrite(value);
	CSN_H();
	return(status);
}

unsigned char SPI1_readReg(unsigned char reg)
{
    unsigned char reg_val;
	CSN_L();
	SPI1_readWrite(reg);
	reg_val = SPI1_readWrite(0);
	CSN_H();
	return(reg_val);
}


unsigned char SPI1_readBuf(unsigned char reg,unsigned char *pBuf, unsigned char bytes)
{
        unsigned char status,i;
        CSN_L();
        // Select register to write to and read status byte
        status = SPI1_readWrite(reg);
        for(i=0;i<bytes;i++)
			pBuf[i] = SPI1_readWrite(0);
        CSN_H();
        return(status);
}

unsigned char SPI1_writeBuf(unsigned char reg, unsigned char *pBuf, unsigned char bytes)
{
        unsigned char status,i;
        CSN_L();
        // Select register to write to and read status byte
        status = SPI1_readWrite(reg);
        for(i=0; i<bytes; i++) // then write all byte in buffer(*pBuf)
        	SPI1_readWrite(*pBuf++);
        CSN_H();
        return(status);
}


void RX_Mode(char RF_DR)
{
	unsigned char buf_[5]={0};
	//USART_SendData(0x77);
	CE_L();

	SPI1_readBuf(TX_ADDR, buf_, TX_ADR_WIDTH);
	SPI1_writeBuf(NRF_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
	SPI1_readWriteReg(NRF_WRITE_REG + EN_AA, 0x00);
	// Enable Auto.Ack:Pipe0
	SPI1_readWriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); // Enable Pipe0
	SPI1_readWriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1f); // 500us + 86us, 10 retrans...
	SPI1_readWriteReg(NRF_WRITE_REG + RF_CH, 120);
	// Select RF channel 120
	SPI1_readWriteReg(NRF_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);
	SPI1_readWriteReg(NRF_WRITE_REG + RF_SETUP, RF_DR_250kbps | RF_PWR_0dBm);
	SPI1_readWriteReg(NRF_WRITE_REG + SETUP_AW, 0x02); // рекомендации форума 
	
	SPI1_readWriteReg(NRF_WRITE_REG + CONFIG,  EN_CRC | PWR_UP | PRIM_RX); // EN_CRC | 
	CE_H(); // Set CE pin high to enable RX device
}


void TX_Mode(char RF_DR)
{
	CE_L();
	SPI1_writeBuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	SPI1_writeBuf(NRF_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH);
	SPI1_readWriteReg(NRF_WRITE_REG + EN_AA, 0x00);
	// Enable Auto.Ack:Pipe0
	SPI1_readWriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); // Enable Pipe0
	SPI1_readWriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1f); // 500us + 86us, 15 retrans...
	SPI1_readWriteReg(NRF_WRITE_REG + RF_CH, 120);       // Select RF channel 120
	SPI1_readWriteReg(NRF_WRITE_REG + RF_SETUP, RF_DR_250kbps | RF_PWR_0dBm);	
	SPI1_readWriteReg(NRF_WRITE_REG + SETUP_AW, 0x02); // рекомендации форума 
	
	SPI1_readWriteReg(NRF_WRITE_REG + CONFIG,  EN_CRC | PWR_UP); //  
}

void RX_Mode_1(void)
{
	//char buf_[5]={0};
	CE_L();
	SPI1_readWriteReg(NRF_WRITE_REG + CONFIG, EN_CRC | PWR_UP | PRIM_RX); // EN_CRC | 
	CE_H(); // Set CE pin high to enable RX device
}


void TX_Mode_1(/*unsigned char * BUF*/)
{
	CE_L();
	SPI1_readWriteReg(NRF_WRITE_REG + CONFIG,  PWR_UP); // EN_CRC |
}


void init_NRF24L01(void)
{
}

void init_io(void)
{
	CE_L();
	CSN_H();
	//RX_Mode();
}


// функция передачи пакета данных
void nRF24L01_TxPacket(unsigned char * tx_buf, unsigned char TX_WIDTH)
{
    SPI1_writeBuf(NRF_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_WIDTH);
	SPI1_writeBuf(WR_TX_PLOAD, tx_buf, TX_PLOAD_WIDTH);
}

// функция приема пакета данных
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf)
{
	unsigned char flag1=0;
	unsigned char status;

	status=SPI1_readReg(STATUS);
	if(status & 0x40){
		SPI1_readBuf(RD_RX_PLOAD,rx_buf,TX_PLOAD_WIDTH);
		flag1 =1;
	}
	SPI1_readWriteReg(NRF_WRITE_REG+STATUS, status);
	return flag1;
}

unsigned char get_status (void)
{
	return (SPI1_readReg(STATUS) >> 4);
}

void NRF_Power_Down (void)
{
	SPI1_readWriteReg(NRF_WRITE_REG + CONFIG,  0);
	
}

// прием данных	
int NRF_Receive(void)
{
	//unsigned char Buf[2]= {0};
    if(nRF24L01_RxPacket(buf)){    
		SPI1_readWriteReg(FLUSH_RX,0);
		return 1;
    }
	else return 0;
}

// передача данных
void NRF_Send(unsigned char*  buf, unsigned char TX_WIDTH)
{
    SPI1_readWriteReg(FLUSH_TX,0);
	nRF24L01_TxPacket(buf, TX_WIDTH);
	CE_H();
	Delay_ms(2);
	CE_L();
}

void clearFlag(void)
{	 
	SPI1_readWriteReg(NRF_WRITE_REG+STATUS,0xff);	
}



