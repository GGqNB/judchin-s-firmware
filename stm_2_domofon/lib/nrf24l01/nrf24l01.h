#ifndef _BYTE_DEF_
#define _BYTE_DEF_
typedef unsigned char BYTE;
#endif   /* _BYTE_DEF_ */


#define SPI1_DR_8b (*(__IO uint8_t *)((uint32_t)SPI1 + 0x0C))

#define TX_ADR_WIDTH    5   // 5 bytes TX(RX) address width
#define TX_PLOAD_WIDTH  20  // 20 bytes TX payload

//****************************************************************//
// SPI(nRF24L01) commands
#define NRF_READ_REG        0x00  // Define read command to register
#define NRF_WRITE_REG       0x20  // Define write command to register
#define RD_RX_PLOAD     0x61  // Define RX payload register address
#define WR_TX_PLOAD     0xA0  // Define TX payload register address
#define FLUSH_TX        0xE1  // Define flush TX register command
#define FLUSH_RX        0xE2  // Define flush RX register command
#define REUSE_TX_PL     0xE3  // Define reuse TX payload register command
#define NOP             0xFF  // Define No Operation, might be used to read status register

//***************************************************//
// SPI(nRF24L01) registers(addresses)
#define CONFIG          0x00  // 'Config' register address
#define EN_AA           0x01  // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR       0x02  // 'Enabled RX addresses' register address
#define SETUP_AW        0x03  // 'Setup address width' register address
#define SETUP_RETR      0x04  // 'Setup Auto. Retrans' register address
#define RF_CH           0x05  // 'RF channel' register address
#define RF_SETUP        0x06  // 'RF setup' register address
#define STATUS          0x07  // 'Status' register address
#define OBSERVE_TX      0x08  // 'Observe TX' register address
#define CD              0x09  // 'Carrier Detect' register address
#define RX_ADDR_P0      0x0A  // 'RX address pipe0' register address
#define RX_ADDR_P1      0x0B  // 'RX address pipe1' register address
#define RX_ADDR_P2      0x0C  // 'RX address pipe2' register address
#define RX_ADDR_P3      0x0D  // 'RX address pipe3' register address
#define RX_ADDR_P4      0x0E  // 'RX address pipe4' register address
#define RX_ADDR_P5      0x0F  // 'RX address pipe5' register address
#define TX_ADDR         0x10  // 'TX address' register address
#define RX_PW_P0        0x11  // 'RX payload width, pipe0' register address
#define RX_PW_P1        0x12  // 'RX payload width, pipe1' register address
#define RX_PW_P2        0x13  // 'RX payload width, pipe2' register address
#define RX_PW_P3        0x14  // 'RX payload width, pipe3' register address
#define RX_PW_P4        0x15  // 'RX payload width, pipe4' register address
#define RX_PW_P5        0x16  // 'RX payload width, pipe5' register address
#define FIFO_STATUS     0x17  // 'FIFO Status Register' register address

// *********************************************************
// bit of config register
#define MASK_RX_DR 		(1 << 6)
#define MASK_TX_DS 		(1 << 5)
#define MASK_MAX_RT		(1 << 4)
#define EN_CRC			(1 << 3)
#define CRC0			(1 << 2)
#define PWR_UP			(1 << 1)
#define PRIM_RX			(1 << 0)
// **********************************************************

// bit of RF_SETUP register
#define CONT_WAVE 		(1 << 7)
#define RF_DR_LOW 		(1 << 5)
#define PLL_LOCK		(1 << 4)
#define RF_DR_HIGH		(1 << 3)
#define RF_PWR_1		(1 << 2)
#define RF_PWR_0		(1 << 1)

#define RF_PWR_18dBm	(0x0 << 1)
#define RF_PWR_12dBm	(0x1 << 1)
#define RF_PWR_6dBm		(0x2 << 1)
#define RF_PWR_0dBm		(0x3 << 1)

#define RF_DR_1Mbps		(0 << 5) | (0 << 3)
#define RF_DR_2Mbps		(0 << 5) | (1 << 3)
#define RF_DR_250kbps	(1 << 5) | (0 << 3)
// **********************************************************

unsigned char SPI1_readWriteReg(unsigned char reg, unsigned char value);
unsigned char SPI1_readReg(unsigned char reg);
unsigned char SPI1_writeBuf(unsigned char reg, unsigned char *pBuf, unsigned char bytes);
unsigned char SPI1_readBuf(unsigned char reg,unsigned char *pBuf, unsigned char bytes);
void RX_Mode(char RF_DR);
void TX_Mode(char RF_DR);
void TX_Mode_1(void);
void RX_Mode_1(void);
void init_NRF24L01(void);
void init_io(void);
unsigned char get_status (void);
void NRF_Power_Down (void);

void nRF24L01_TxPacket(unsigned char * tx_buf, unsigned char TX_WIDTH);
// функция приема пакета данных
unsigned char nRF24L01_RxPacket(unsigned char* rx_buf);
// прием данных	
int NRF_Receive(void);
// передача данных
void NRF_Send(unsigned char*  buf, unsigned char TX_WIDTH);
// очистка флагов
void clearFlag(void);
