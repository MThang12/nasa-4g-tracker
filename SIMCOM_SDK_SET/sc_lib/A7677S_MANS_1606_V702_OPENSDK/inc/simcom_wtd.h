/*begin added byxiaobing.fang for jira-A76801606-1884 20221027  */
#ifndef __SIMCOM_WTD_H__
#define __SIMCOM_WTD_H__



#define WATCHDOG_TIMEOUT_REG      0x11
#define WATCHDOG_FAULTWAKEUP_REG  0xE7
#define WATCHDOG_ENABLE_REG       0x1D
#define WATCHDOG_FEED_REG         0x0D


/*
*Bit [3:0] sets the active timeout period, and bit[5:4] sets the timeout period in Sleep mode
*/
int sAPI_SetWtdTimeOutPeriod(unsigned char value);


/*WRITE 1  enable fault_wu_en
 *WRITE 3  enable fault_wu
 *Note : The two bits must be enabled separately, and the first command cannot be omitted
*/
int sAPI_FalutWakeEnable(unsigned char enable);

int sAPI_SoftWtdEnable(unsigned char enable);


/*
*Note: The interval of feeding the dog cannot exceed the timeout set by the register 0x11. After starting the WDT,
*      the dog needs to be fed once before the timeout set by the register 0x11 will load
*/
int sAPI_FeedWtd(void);

#endif
/*end added byxiaobing.fang for jira-A76801606-1884 20221027  */

