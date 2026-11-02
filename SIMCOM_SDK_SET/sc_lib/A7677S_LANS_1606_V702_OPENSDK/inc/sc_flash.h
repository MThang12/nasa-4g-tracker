/*** 
 * @Author: yangkui.zhang
 * @Date: 2023-05-06 15:36:35
 * @LastEditTime: 2023-11-09 16:07:29
 * @LastEditors: yangkui.zhang
 * @Description: 
 * @FilePath: \ASR160X\simcom\open_api\inc\sc_flash.h
 * @no error,no warning
 */
#ifndef __SC_FLASH_H__
#define __SC_FLASH_H__


unsigned int sAPI_AppPartitionBinSize(void);
int sAPI_ReadFlashAppPartition(unsigned int offset, char *buff, unsigned int size);
int sAPI_ReadFlash(unsigned int offset, char *buff, unsigned int size);
int sAPI_WriteFlash(unsigned int offset, char *buff, unsigned int size);
int sAPI_EraseFlashSector(unsigned int offset, unsigned int size);
int sAPI_GetFlashSize(void);
int sAPI_FlashRestoreFactory(void);
int sAPI_ReadFlashNvmPartition(unsigned int offset, char* buff, unsigned int size);
int sAPI_WriteFlashNvmPartition(unsigned int offset, char* buff, unsigned int size);
int sAPI_EraseFlashNvmPartition(unsigned int offset, unsigned int size);
void sAPI_FBF_Disable(void);

#ifdef CUS_YJ
int sAPI_ReadFlashFotaParam(unsigned int offset, char* buff, unsigned int size);
int sAPI_EraseFlashSectorFotaParam(unsigned int offset, unsigned int size);
int sAPI_WriteFlashFotaParam(unsigned int offset, char* buff, unsigned int size);
#endif
#endif
