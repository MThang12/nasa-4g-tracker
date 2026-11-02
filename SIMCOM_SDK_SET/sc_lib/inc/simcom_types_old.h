#ifndef __SIMCOM_TYPES_OLD_H__
#define __SIMCOM_TYPES_OLD_H__


#ifndef CHAR
typedef char            CHAR;
#endif

struct dirent
{
    unsigned char type;
    long size;
    char name[MAX_FULL_NAME_LENGTH];
};


// 临时定义
#define SC_MODULE_GPIO_MAX 100

#endif /* __SIMCOM_TYPES_OLD_H__ */
