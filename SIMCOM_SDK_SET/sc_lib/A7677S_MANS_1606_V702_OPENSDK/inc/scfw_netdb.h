#ifndef __SCFW_NETDB_H__
#define __SCFW_NETDB_H__

#include "scfw_netdb_define.h"


struct hostent *lwip_gethostbyname(const char *name);

int lwip_getaddrinfo(const char *nodename,
                     const char *servname,
                     const struct addrinfo *hints,
                     struct addrinfo **res);

void lwip_freeaddrinfo(struct addrinfo *ai);

int lwip_getaddrinfo_with_pcid(const char *nodename,
                               const char *servname,
                               const struct addrinfo *hints,
                               struct addrinfo **res,
                               unsigned char pcid);


static inline struct hostent *gethostbyname(const char *name)
{
    return lwip_gethostbyname(name);
}

static inline int getaddrinfo(const char *nodename,
                              const char *servname,
                              const struct addrinfo *hints,
                              struct addrinfo **res)
{
    return lwip_getaddrinfo(nodename, servname, hints, res);
}

static inline int getaddrinfo_with_pcid(const char *nodename,
                                        const char *servname,
                                        const struct addrinfo *hints,
                                        struct addrinfo **res,
                                         unsigned char pcid)
{
    return lwip_getaddrinfo_with_pcid(nodename, servname, hints, res, pcid);
}

static inline void freeaddrinfo(struct addrinfo *ai)
{
    return lwip_freeaddrinfo(ai);
}


#ifdef FEATURE_MAPPING_SOCKET_ALL

struct hostent *lwip_gethostbyname_with_pcid(const char *name, unsigned char pcid);
struct hostent *lwip_gethostbyname_with_netif(const char *name, const char *netif_name);

int lwip_gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                         unsigned int buflen, struct hostent **result, int *h_errnop);
int lwip_gethostbyname_r_with_pcid(const char *name, struct hostent *ret, char *buf,
                                   unsigned int buflen, struct hostent **result, int *h_errnop, unsigned char pcid);
int lwip_gethostbyname_r_with_netif(const char *name, struct hostent *ret, char *buf,
                                    unsigned int buflen, struct hostent **result, int *h_errnop, const char *netif_name);

int lwip_getaddrinfo_with_netif(const char *nodename,
                                const char *servname,
                                const struct addrinfo *hints,
                                struct addrinfo **res, const char *netif_name);

int lwip_getaddrinfo_r(const char *nodename,
                       const char *servname,
                       const struct addrinfo *hints,
                       struct addrinfo **res);


static inline struct hostent *gethostbyname_with_pcid(const char *name, unsigned char pcid)
{
    return lwip_gethostbyname_with_pcid(name, pcid);
}

static inline struct hostent *gethostbyname_with_netif(const char *name, const char *netif_name)
{
    return lwip_gethostbyname_with_netif(name, netif_name);
}

static inline int gethostbyname_r(const char *name, struct hostent *ret, char *buf,
                                  unsigned int buflen, struct hostent **result, int *h_errnop)
{
    return lwip_gethostbyname_r(name, ret, buf, buflen, result, h_errnop);
}

static inline int gethostbyname_r_with_pcid(const char *name, struct hostent *ret, char *buf,
        unsigned int buflen, struct hostent **result, int *h_errnop, unsigned char pcid)
{
    return lwip_gethostbyname_r_with_pcid(name, ret, buf, buflen, result, h_errnop, pcid);
}

static inline int gethostbyname_r_with_netif(const char *name, struct hostent *ret, char *buf,
        unsigned int buflen, struct hostent **result, int *h_errnop, const char *netif_name)
{
    return lwip_gethostbyname_r_with_netif(name, ret, buf, buflen, result, h_errnop, netif_name);
}

static inline int getaddrinfo_with_netif(const char *nodename,
        const char *servname,
        const struct addrinfo *hints,
        struct addrinfo **res, const char *netif_name)
{
    return lwip_getaddrinfo_with_netif(nodename, servname, hints, res, netif_name);
}

static inline int getaddrinfo_r(const char *nodename,
                                const char *servname,
                                const struct addrinfo *hints,
                                struct addrinfo **res)
{
    return lwip_getaddrinfo_r(nodename, servname, hints, res);
}

#endif /* FEATURE_MAPPING_SOCKET_ALL */

#endif
