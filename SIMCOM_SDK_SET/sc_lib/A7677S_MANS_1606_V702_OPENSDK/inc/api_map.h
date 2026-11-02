/*
 * file "api_map.h" is generate automatic, it must not be modifiee manually.
 * author: Louis-Qiu
 * time: 2024-07-20 23:50:45
 */
#ifndef __API_MAP_H__
#define __API_MAP_H__


typedef struct
{
    void * appref;
    void *appstackPtr;
    long stackSize;
    unsigned char priority; /* 100 - 200 */
    char * appname;
    void  (*taskStart)(void*);
    void * argv;
}userSpaceEntry_t;

void ApiMapInit(void *ptr);

#endif /* __API_MAP_H__ */
