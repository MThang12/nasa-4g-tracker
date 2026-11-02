#ifndef __SCFW_SOCKET_H__
#define __SCFW_SOCKET_H__

#include "scfw_socket_define.h"


int lwip_socket(int domain, int type, int protocol);
int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);
int lwip_listen(int s, int backlog);
int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
int lwip_recv(int s, void *mem, size_t len, int flags);
int lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
int lwip_send(int s, const void *dataptr, size_t size, int flags);
int lwip_sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int lwip_close(int s);
int lwip_shutdown(int s, int how);
int lwip_getpeername(int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockname(int s, struct sockaddr *name, socklen_t *namelen);
int lwip_getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
int lwip_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);


int lwip_ioctl(int s, long cmd, void *argp);
int lwip_getsockerrno(int s);
int lwip_socket_with_callback(int domain, int type, int protocol, socket_callback callback);

int lwip_geterrno(void);
#undef  errno
#define errno lwip_geterrno()

static inline int close(int __fildes)
{
    return lwip_close(__fildes);
}

static inline int socket(int __domain, int __type, int __protocol)
{
    int fd = -1;

    fd = lwip_socket(__domain, __type, __protocol);
    return fd;
}

static inline int bind(int fd, const struct sockaddr *__addr, socklen_t __len)
{
    int ret = -1;

    ret = lwip_bind(fd, __addr, __len);
    return ret;
}

static inline int shutdown(int fd, int __how)
{
    int ret = -1;

    ret = lwip_shutdown(fd, __how);
    return ret;
}

static inline int getpeername(int __fd, struct sockaddr *__name, socklen_t *__namelen)
{
    int fd = __fd;
    int ret = -1;

    ret = lwip_getpeername(fd, __name, __namelen);
    return ret;
}

static inline int getsockname(int fd, struct sockaddr *__name, socklen_t *__namelen)
{
    int ret = -1;

    ret = lwip_getsockname(fd, __name, __namelen);
    return ret;
}

static inline int connect(int fd, const struct sockaddr *__addr, socklen_t __len)
{
    int ret = -1;

    ret = lwip_connect(fd, __addr, __len);
    return ret;
}

static inline int send(int fd, const void *__buf, unsigned int __n, int __flags)
{
    int ret = -1;

    ret = lwip_send(fd, __buf, __n, __flags);
    return ret;
}

static inline int recv(int fd, void *__buf, unsigned int __n, int __flags)
{
    int ret = -1;

    ret = lwip_recv(fd, __buf, __n, __flags);
    return ret;
}

static inline int sendto(int fd, const void *__buf, unsigned int __n, int __flags,
                         const struct sockaddr *__addr, socklen_t __addr_len)
{
    int ret = -1;

    ret = lwip_sendto(fd, __buf, __n, __flags, __addr, __addr_len);
    return ret;
}

static inline int recvfrom(int fd, void *__restrict __buf, unsigned int __n, int __flags,
                           struct sockaddr *__restrict __addr, socklen_t *__restrict __addr_len)
{
    int ret = -1;

    ret = lwip_recvfrom(fd, __buf, __n, __flags, __addr, __addr_len);
    return ret;
}

static inline int getsockopt(int fd, int __level, int __optname, void *__restrict __optval,
                             socklen_t *__restrict __optlen)
{
    int ret = -1;

    ret = lwip_getsockopt(fd, __level, __optname, __optval, __optlen);
    return ret;
}

static inline int setsockopt(int fd, int __level, int __optname, const void *__optval, socklen_t __optlen)
{
    int ret = -1;
    ret = lwip_setsockopt(fd, __level, __optname, __optval, __optlen);
    return ret;
}

static inline int listen(int fd, int __n)
{
    int ret = -1;

    ret = lwip_listen(fd, __n);
    return ret;
}

static inline int accept(int fd, struct sockaddr *__addr, socklen_t *__restrict __addr_len)
{
    int ret = -1;

    ret = lwip_accept(fd, __addr, __addr_len);
    return ret;
}

static inline int select(int nfds, fd_set *readfds, fd_set *writefds,
                         fd_set *exceptfds, struct timeval *timeout)
{
    return lwip_select(nfds, readfds, writefds, exceptfds, timeout);
}


#ifdef FEATURE_MAPPING_SOCKET_ALL

int lwip_read(int s, void *mem, size_t len);
int lwip_write(int s, const void *dataptr, size_t size);
int lwip_close2(int s, char *filename, int line);
int lwip_shutdown2(int s, int how, char *filename, int line);
int lwip_fcntl(int s, int cmd, int val);
int lwip_eventfd(unsigned int initval, int flags);
int lwip_getthreaderrno(int s);
int lwip_getsockvalid(int s);
int lwip_getsocktype(int s);
struct netconn *lwip_getsocknetconn(int s);
struct tcp_pcb *lwip_getsocktcppcb(int s);
struct udp_pcb *lwip_getsockudppcb(int s);
int lwip_get_sock_localport(int s);
int lwip_get_sock_remoteport(int s);
int lwip_get_sock_isipv6(int s);
int lwip_trigger_process(char *name, sys_thread_t threadid);
ipX_addr_t *lwip_get_sock_localipXaddr(int s);
ipX_addr_t *lwip_get_sock_remoteipXaddr(int s);

int readn(int fd, void *buf, size_t count);
int writen(int fd, const void *buf, size_t count);
int readline(int fd, void *buf, size_t maxline);

int lwip_readv(int s, const struct iovec *iov, int iovcnt);
int lwip_writev(int s, const struct iovec *iov, int iovcnt);
int lwip_recvmsg(int s, struct msghdr *message, int flags);
int lwip_sendmsg(int s, const struct msghdr *message, int flags);

#endif /* FEATURE_MAPPING_SOCKET_ALL */

#endif
