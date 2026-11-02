#ifndef __SCFW_INET_H__
#define __SCFW_INET_H__

#include "scfw_socket_define.h"
#include "scfw_inet_define.h"


unsigned int inet_addr(const char *cp);
char *inet_ntoa(struct in_addr addr);
char *inet_ntop(int af, const void *src, char *dst, int size);
int inet_pton(int af, const char *src, void *dst);

static inline unsigned int htonl(unsigned int n)
{
    return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000UL) >> 8) | ((n & 0xff000000UL) >> 24);
}

static inline unsigned short htons(unsigned short n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

static inline unsigned int ntohl(unsigned int n)
{
    return htonl(n);
}

static inline unsigned short ntohs(unsigned short n)
{
    return htons(n);
}

#ifdef FEATURE_MAPPING_SOCKET_ALL

int inet_aton(const char *cp, struct in_addr *addr);
char *inet_ntoa_r(struct in_addr addr, char *buf, int buflen);

#endif /* FEATURE_MAPPING_SOCKET_ALL */

#endif /* __LWIP_INET_H__ */
