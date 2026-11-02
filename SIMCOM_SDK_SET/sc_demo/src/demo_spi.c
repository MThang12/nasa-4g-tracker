/**
  ******************************************************************************
  * @file    demo_spi.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of SPI operation.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "simcom_os.h"
#include "simcom_debug.h"
#include "sc_spi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "simcom_common.h"
#include "simcom_gpio.h"
#include "uart_api.h"


#define NEW_SPI_API
#define MAX_SPI_BUFFER_DATA_LEN 300
#define SPI_TX_RX_DATA_LED 4

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

typedef enum
{
    READ_ID = 1,
    READ_WRITE_STATUS,
    READ_ID_DMA,
    BACK = 99,
} SPI_OPS;

typedef enum
{
    SPI_FLASH_INIT = 1,
    SPI_FLASH_READ_ID = 2,
    SPI_FLASH_ERASE,
    SPI_FLASH_READ,
    SPI_FLASH_WRITE,
    SPI_NAND_FLASH_YAFFS_TEST,
    SPI_FLASH_BACK = 99,
} SPI_FLASH;


/**
 * @brief  This demo show that how to use spi api by operating "XM25QU128B" flash IC.
 * @param  void
 * @note
 * @retval void
 *
 * Demo describe:
 * This demo show that how to use spi api by operating "XM25QU128B" flash.
 */
void SpiDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. spi flash read id",
        "2. spi flash read/write status reg",
        "3. spi flash read id for DMA",
        "99. back",
    };
    int ret = 0;
    char disBuff[128] = {0};
#ifdef NEW_SPI_API
    /**************** spi init ****************/
    SC_SPI_DEV spiDev;
    /*set spi clock*/
#ifdef SPIDRV_SUPPORT
    spiDev.clock = SC_SPI_CLK_100KHZ;
#else
    spiDev.clock = 0x1;
#endif
    /* set spi mode */
    spiDev.mode = SPI_MODE_PH0_PO0;
    /*
    set cs mode
    GPIO_MODE:cs remains low level during data stream transmission.
    SSP_MODE:cs remains low level during Byte transmission.
     */
    spiDev.csMode = GPIO_MODE;
    /* Set SPI channel, default is 0 */
    spiDev.index = 1;
    sAPI_SpiConfigInitEx(&spiDev);
#else
    sAPI_SPIConfigInit(SPI_CLOCK_6MHz, SPI_MODE_PH0_PO0);
#endif
    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
#ifdef NEW_SPI_API
            case READ_ID:
            {

                unsigned char SendData[4] = {0x9F, 0, 0, 0};
                unsigned char RevData[4] = {0};

                ret = sAPI_SpiReadBytesEx(&spiDev, SendData, 1, RevData, 3);
                if (ret != 0)
                {
                    sprintf(disBuff, "\r\nspi0 read bytes fail\r\n");
                }
                else
                {
                    sprintf(disBuff, "\r\nspi0 read device ID:%x\r\n", (RevData[0] << 16) | (RevData[1] << 8) | RevData[2]);
                }

                PrintfResp(disBuff);
                break;
            }
            case READ_WRITE_STATUS:
            {
                /* read status register*/
                unsigned char writeData[2] = {0x01};
                unsigned char readCmd = 0x05;
                unsigned char status = 0;

                ret = sAPI_SpiReadBytesEx(&spiDev, &readCmd, 1, &status, 1);
                if (ret != 0)
                    sAPI_Debug("ReadBytes fail\r\n");
                sAPI_Debug("read bytes seccess 0x%x\r\n", status);

                writeData[1] = status | 0x02;
                ret = sAPI_SpiWriteBytesEx(&spiDev, writeData, 2);
                if (ret != 0)
                    sAPI_Debug("ReadBytes fail\r\n");
                sAPI_Debug("write bytes seccess 0x%x\r\n", writeData[1]);

                break;
            }
            case READ_ID_DMA:
            {
#ifdef SPIDRV_SUPPORT
                unsigned char SendData1[4] = {0x9F, 0, 0, 0};
                unsigned char RevData1[4] = {0};

                ret = sAPI_SpiReadBytesEx(&spiDev, SendData1, 1, RevData1, 3);
                if (ret != 0)
                {
                    sprintf(disBuff, "\r\nspi0 read bytes fail\r\n");
                }
                else
                {
                    //sprintf(disBuff, "\r\n RevData[0] %x ,RevData[1] %x , RevData[2] %x , RevData[3]  %x , \r\n", RevData[0] , RevData[1], RevData[2],RevData[3]);
                    sprintf(disBuff, "\r\nspi0 read device ID:%x\r\n", (RevData1[1] << 16) | (RevData1[2] << 8) | RevData1[3]);
                }

                PrintfResp(disBuff);
#else
                 PrintfResp("\r\nNot yet opened\r\n");
#endif
                break;
            }
#else
            case READ_ID:
            {
                char SendData[4] = {0x9F, 0, 0, 0};
                char RevData[4] = {0};

                ret = sAPI_ExtSpiReadBytes(SendData, 1, RevData, 3);
                if (ret != 0)
                    sAPI_Debug("ReadBytes fail\r\n");
                sAPI_Debug("spi0 read device ID:%x\r\n", (RevData[0] << 16) | (RevData[1] << 8) | RevData[2]);
                break;
            }
            case READ_WRITE_STATUS:
            {
                break;
            }
#endif
            case BACK:
                return;
        }
    }

}

#define START_ADDR  (4096*0)
/**
 * @brief  This demo show that how to use spi api by operating nor flash IC.
 * @param  void
 * @note
 * @retval void
 */
void SpiNorDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. spi flash init",
        "2. spi flash read id",
        "3. spi flash erase",
        "4. spi flash read",
        "5. spi flash write",
        "99. back",
    };

#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
    int ret = 0;
    char disBuff[256] = {0};
#endif

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
#if defined(SPINOR_SUPPORT) || defined (SC_DRIVER_NORFLASH)
            case SPI_FLASH_INIT:
            {
                sAPI_Debug("sAPI_ExtNorFlashInit");
                sAPI_ExtNorFlashInit();
                sAPI_Debug("sAPI_ExtNorFlashInit...");

#if 0  /*Show the nor flash which is supported*/
                unsigned char Buffer[50] = {0};
                int i=0;
                ret = sAPI_ExtNorFlashShowSupportList(Buffer, 50);
                if (!ret)
                {
                    for(i=0;i<50;i++)
                    {
                        sprintf(disBuff, "%c",Buffer[i]);
                        PrintfResp(disBuff);
                    }
                }
#endif
                sprintf(disBuff, "\r\nspi nor flash init finished...\r\n");
                PrintfResp(disBuff);
                break;
            }
            case SPI_FLASH_READ_ID:
            {
                unsigned int id = 0;
                sAPI_ExtNorFlashReadID((unsigned char *)&id);

                sprintf(disBuff, "\r\nsAPI_ExtNorFlashReadID [0x%x]\r\n", id);
                PrintfResp(disBuff);
                break;
            }
            case SPI_FLASH_ERASE:
            {
                ret = sAPI_ExtNorFlashSectorErase(START_ADDR, 0x1000);
                if (ret)
                {
                    memset(disBuff, 0, sizeof(disBuff));
                    sprintf(disBuff, "\r\nerase fail\r\n");
                    PrintfResp(disBuff);
                }

                break;
            }
            case SPI_FLASH_READ:
            {
                int i;
                char *Buffer = (char *)sAPI_Malloc(256);
                memset(Buffer, 0X00, 256);
                char *ptrDisBuff = disBuff;
                memset(disBuff, 0, 256);

                ret = sAPI_ExtNorFlashRead(START_ADDR, 256, (uint32_t)Buffer);
                if (ret != 0)
                    sAPI_Debug("read fail\r\n");

                for (i = 0; i < 128; i++)
                {
                    sprintf(ptrDisBuff, "%02x  ", Buffer[i]);
                    ptrDisBuff += 4;
                }
                sAPI_Free(Buffer);
                PrintfResp(disBuff);
                break;
            }
            case SPI_FLASH_WRITE:
            {
                int i;
                unsigned char Buffer[256] = {0};
                memset(Buffer, 0x43, 256);
                ret = sAPI_ExtNorFlashWrite(START_ADDR, 256, (uint32_t)Buffer);
                if (ret != 0)
                    sAPI_Debug("write fail\r\n");
                for (i = 0; i < 0x10; i++)
                {
                    sAPI_Debug("[spi test]data write:%x,", Buffer[i]);
                }
                break;
            }
#endif
            case SPI_FLASH_BACK:
                return;
        }
    }

}


void SpiNandDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] =
    {
        "1. spi nand flash init",
        "2. spi nand flash read id",
        "3. spi nand flash erase",
        "4. spi nand flash read",
        "5. spi nand flash write",
        "6. yaffs test ",
        "99. back",
    };
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT

    char disBuff[1024] = {0};
    char disBuff1[1024] = {0};
#define SPI_NAND_PAGE_SZIE_temp  0x800
#endif

    while (1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt)
        {
#ifdef SIMCOM_SPINAND_FLASH_SUPPORT
            case SPI_FLASH_INIT:
            {
                int ret1 = 3;
                PrintfResp("\r\n sAPI_ExtNandFlashInit start \r\n");
                ret1 = extSpiNandInit();
                if (ret1 == 0)
                {
                    PrintfResp("\r\n SPINANDDevice_Init success \r\n");
                }
                else
                {
                    PrintfResp("\r\n SPINANDDevice_Init error \r\n");
                }
                PrintfResp("\r\n sAPI_ExtNandFlashInit finish \r\n");


                //sprintf(disBuff, "\r\nspi nor flash init finished...\r\n");
                //PrintfResp(disBuff);
                break;
            }

            case SPI_FLASH_READ_ID:
            {
                PrintfResp("\r\n Please use SpiDemo read flash id \r\n");
                break;
            }
            case SPI_FLASH_ERASE:
            {
                unsigned int FlashOffset_temp = 0x0; //address
                sAPI_SPINAND_Erase(FlashOffset_temp, 0x800, BOOT_FLASH);
                PrintfResp("\r\n erase block \r\n");
                break;
            }
            case SPI_FLASH_READ:
            {
                PrintfResp("\r\n kaishi read 0\r\n");
                unsigned int FlashOffset_temp = 0x0; //address

                //unsigned int ret=0;
                unsigned int i = 0;
                char *rbuf = (char *)malloc(SPI_NAND_PAGE_SZIE_temp);

                memset(rbuf, 0x0, SPI_NAND_PAGE_SZIE_temp);
                PrintfResp("rbuf:");
                PrintfResp(rbuf);
                PrintfResp("\r\n kaishi read 1\r\n");
                sAPI_SPINAND_Read(FlashOffset_temp, (unsigned int)rbuf, SPI_NAND_PAGE_SZIE_temp, BOOT_FLASH);
                PrintfResp("\r\n kaishi read 2\r\n");
                for (i = 0; i < SPI_NAND_PAGE_SZIE_temp; i = i + 16)
                {
                    sprintf(disBuff, "i=%d: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", i,
                            rbuf[i], rbuf[i + 1], rbuf[i + 2], rbuf[i + 3], rbuf[i + 4], rbuf[i + 5], rbuf[i + 6], rbuf[i + 7],
                            rbuf[i + 8], rbuf[i + 9], rbuf[i + 10], rbuf[i + 11], rbuf[i + 12], rbuf[i + 13], rbuf[i + 14], rbuf[i + 15]);
                    PrintfResp(disBuff);

                }
                PrintfResp("\r\n kaishi read 3\r\n");
                free(rbuf);
                PrintfResp(disBuff);
                PrintfResp("\r\n kaishi read 4\r\n");
                break;

            }
            case SPI_FLASH_WRITE:
            {
                PrintfResp("\r\n kaishi write 0\r\n");
                unsigned int FlashOffset_temp = 0; //address
                //  unsigned int ret=0;
                unsigned int i = 0;
                char *wbuf = (char *)malloc(SPI_NAND_PAGE_SZIE_temp);
                char *rbuf = (char *)malloc(SPI_NAND_PAGE_SZIE_temp);

                //  UINT8 wbuffpr[1024]={0};
                //  unsigned char buf[1024];
                char buff[2048];
                //  unsigned int buff2[1024];
                //  unsigned int buff3[3072];
                char buff4[3072];
                //RTI_LOG("address:0x%x",FlashOffset_temp);
                memset(wbuf, 's', SPI_NAND_PAGE_SZIE_temp);
                sprintf(buff, "\r\n char-wbuf:%s \r\n", wbuf);
                PrintfResp(buff);

                //PrintfResp("\r\n kaishi write 1\r\n");
                //for (i = 0; i < SPI_NAND_PAGE_SZIE_temp; i++){
                //  wbuf[i] = i%256;
                //PrintfResp("sdvfd");
                // PrintfResp(wbuf[i]);
                //}
                //  PrintfResp("\r\n kaishi write 2\r\n");
                sAPI_SPINAND_Erase(FlashOffset_temp, 0x10000, BOOT_FLASH);

                //  PrintfResp("\r\n kaishi write 3\r\n");

                //  PrintfResp("\r\n kaishi write 4\r\n");
                sAPI_SPINAND_Write(FlashOffset_temp, (unsigned int)wbuf, SPI_NAND_PAGE_SZIE_temp, BOOT_FLASH);
                //  PrintfResp("\r\n kaishi write 5\r\n");

                memset(rbuf, 0x0, SPI_NAND_PAGE_SZIE_temp);

                sAPI_SPINAND_Read(FlashOffset_temp, (unsigned int)rbuf, SPI_NAND_PAGE_SZIE_temp, BOOT_FLASH);
                // sprintf(disBuff1,"rbuf:%d",rbuf);
                //  PrintfResp("\r\n kaishi write 6\r\n");

                if (memcmp(wbuf, rbuf, SPI_NAND_PAGE_SZIE_temp) != 0)
                {
                    PrintfResp("if (memcmp(wbuf, rbuf, SPI_NAND_PAGE_SZIE_temp) != 0)");
                    //RTI_LOG("!!!%s error",__func__);
                    for (i = 0; i < SPI_NAND_PAGE_SZIE_temp; i = i + 1)
                    {

                        //PrintfResp("\r\njinru for(i=0; i<SPI_NAND_PAGE_SZIE_temp; i=i+16) \r\n");
                        sprintf(disBuff, "\r\n i=%d: %x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\r\n", i,
                                rbuf[i], rbuf[i + 1], rbuf[i + 2], rbuf[i + 3], rbuf[i + 4], rbuf[i + 5], rbuf[i + 6], rbuf[i + 7],
                                rbuf[i + 8], rbuf[i + 9], rbuf[i + 10], rbuf[i + 11], rbuf[i + 12], rbuf[i + 13], rbuf[i + 14], rbuf[i + 15]);
                        //PrintfResp(disBuff);
                        //  PrintfResp("\r\n----------------------------\r\n");

                    }
                }
                if (memcmp(wbuf, rbuf, SPI_NAND_PAGE_SZIE_temp) == 0)
                {

                    PrintfResp("\r\n wbuf == rbuf inadress \r\n");
                    for (i = 0; i < 10; i = i + 1)
                    {

                        //PrintfResp("\r\njinru for(i=0; i<SPI_NAND_PAGE_SZIE_temp; i=i+16) \r\n");
                        sprintf(buff4, "\r\n i=%d: %x\r\n", i, rbuf[i]);
                        PrintfResp(buff4);

                        PrintfResp("\r\n wbuf == rbuf inadress \r\n");

                    }

                }

                PrintfResp("\r\ndisBuff:\r\n");
                PrintfResp(disBuff);
                PrintfResp("\r\n----------------------------\r\n");


                PrintfResp("\r\ndisBuff1;");

                PrintfResp(disBuff1);
                free(wbuf);
                free(rbuf);

                PrintfResp("\r\n kaishi write end\r\n");

                break;

            }
#endif
            case SPI_NAND_FLASH_YAFFS_TEST:
            {
                PrintfResp("\r\n SPI_NAND_FLASH_YAFFS_TEST\r\n");
#ifdef YAFFS2
                sAPI_yaffs_II_test();
#endif
                PrintfResp("\r\n SPI_NAND_FLASH_YAFFS_TEST finsih\r\n");
                break;
            }
            case SPI_FLASH_BACK:
                return;
        }
    }

}


