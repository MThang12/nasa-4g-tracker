#ifndef _SC_SPI_H_
#define _SC_SPI_H_

//#include "spi_nor.h"

typedef enum
{
    SC_SPI_RC_OK=0,
    SC_SPI_RC_ERROR=-1,
}SC_SPI_ReturnCode;

typedef struct
{
    unsigned char index;
    int clock;
    int mode;
    int csMode; //when the value is 0,the cs config to gpio
}SC_SPI_DEV;
#ifdef SPIDRV_SUPPORT
typedef enum {
	SC_SPI_CLK_812_5KHZ,
	SC_SPI_CLK_1_625MHZ,
	SC_SPI_CLK_3_25MHZ,
	SC_SPI_CLK_6_5MHZ,
	SC_SPI_CLK_13MHZ,
	SC_SPI_CLK_26MHZ,
	SC_SPI_CLK_52MHZ,
	SC_SPI_CLK_100KHZ,
	SC_SPI_CLK_200KHZ,
	SC_SPI_CLK_300KHZ,
	SC_SPI_CLK_400KHZ,
	SC_SPI_CLK_500KHZ,
	SC_SPI_CLK_600KHZ,
	SC_SPI_CLK_700KHZ,
	SC_SPI_CLK_800KHZ,
	SC_SPI_CLK_900KHZ,
	SC_SPI_CLK_1MHZ,
}SC_SPI_CLOCK;
#else
typedef enum
{
    SPI_CLOCK_6MHz = 0,
    SPI_CLOCK_13MHz,
    SPI_CLOCK_26MHz,
    SPI_CLOCK_52MHz,
    SPI_CLOCK_3250KHz,
    SPI_CLOCK_1625KHz,
    SPI_CLOCK_812KHz,
}SC_SPI_CLOCK;
#endif
typedef enum
{
    SPI_MODE_PH0_PO0 = 0,
    SPI_MODE_PH0_PO1,
    SPI_MODE_PH1_PO0,
    SPI_MODE_PH1_PO1,
}SC_SPI_MODE;

typedef enum
{
    SSP_MODE = 0,
    GPIO_MODE,
    CUS_CS_MODE,
}CS_MODE;

typedef enum
{
#ifndef BOOT_FLASH
#ifndef SAVE_STATE_FLASH
    BOOT_FLASH          = 0,
    SAVE_STATE_FLASH    = 1
#endif
#endif
} SC_FlashBootType_T;

SC_SPI_ReturnCode sAPI_SpiConfigInitEx(SC_SPI_DEV *handle);
SC_SPI_ReturnCode sAPI_SpiReadBytesEx(SC_SPI_DEV *handle,unsigned char *sendBuff,unsigned int sendSize, unsigned char *revBuff,unsigned int revSize);
SC_SPI_ReturnCode sAPI_SpiWriteBytesEx(SC_SPI_DEV *handle,unsigned char *sendBuff,unsigned int sendSize);

SC_SPI_ReturnCode sAPI_SPIReadBytes(unsigned int *SendData,unsigned int *RevData,int Size);
SC_SPI_ReturnCode sAPI_ExtSpiReadBytes(unsigned char *sendBuff,unsigned int sendSize, unsigned char *revBuff,unsigned int revSize);
SC_SPI_ReturnCode sAPI_SPIWriteBytes(unsigned int *SendData,int Size);
void sAPI_SPIConfigInit(int ssp_clk, int ssp_mode);

SC_SPI_ReturnCode sAPI_ExtNorFlashShowSupportList(void *outBuff, int outBuffLen); //The API is compatible with old customers and prohibited for new customers.

SC_SPI_ReturnCode sAPI_SpiCustomCs(int sspport,int pin); //This API must be used after the SPI initialization is complete  (To use it, you need to call sAPI_SpiConfigInitEx)
SC_SPI_ReturnCode sAPI_SpiCustomCsSetLevel(int sspport,int level);



#ifdef SC_DRIVER_NORFLASH
unsigned int sAPI_ExtNorFlashSectorErase(int offset, int size);
unsigned int sAPI_ExtNorFlashWrite(unsigned int Address, int Size, unsigned int Buffer);
unsigned int sAPI_ExtNorFlashRead(unsigned int FlashOffset, int Size, unsigned int Buffer);
unsigned int sAPI_ExtNorFlashReadID(unsigned char* id);
unsigned int sAPI_ExtNorFlashBlock64kErase(int offset, int size);
void sAPI_ExtNorFlashInit(void);
int sAPI_extSpiNorInit(void);

#endif

#ifdef  SIMCOM_SPINAND_FLASH_SUPPORT
int extSpiNandInit(void);
unsigned int  sAPI_SPINANDDevice_Init(unsigned char flashnu, SC_FlashBootType_T flashbootType, unsigned char *P_DefaultPartitonNum);
unsigned int  sAPI_SPINAND_Read(unsigned int FlashOffset, unsigned int Buffer,unsigned int Size, SC_FlashBootType_T fbt);
unsigned int  sAPI_SPINAND_Write(unsigned int FlashOffset, unsigned int Buffer,unsigned int Size, SC_FlashBootType_T fbt);
unsigned int  sAPI_SPINAND_Erase(unsigned int FlashOffset, unsigned int Size,SC_FlashBootType_T fbt);
unsigned int sAPI_SPINAND_GetFeatures(void);
unsigned int sAPI_SPINAND_Write_Page_SpareData(unsigned int FlashOffset, unsigned int Buffer,  unsigned int size);
signed int sAPI_SPINAND_Read_Page_SpareData(unsigned int FlashOffset, unsigned int Buffer,  unsigned int size);
typedef enum
{
	NAND_READID_MODE_OPCODE,
	NAND_READID_MODE_OPCODE_ADDR,
	NAND_READID_MODE_OPCODE_DUMMY,
}NAND_FLASH_READID_MODE;
unsigned int sAPI_ExtNnandFlashReadID(int mode,unsigned char* id);
#endif

#ifdef YAFFS2
void sAPI_yaffs_II_test(void);
#endif

#endif
