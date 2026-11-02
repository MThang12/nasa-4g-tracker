/*
 * @Author: Wei.Wang
 * @Date: 2023-12-18
 * @Last Modified by: Wei.Wang
 * @Last Modified time: 2023-12-21
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "simcom_demo_init.h"
#include "sal_os.h"
#include "sal_log.h"
#include "pub_string.h"
#include "pub_cache.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "simcom_common.h"
#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"
#include "simcom_os.h"
#include "demo_tcpip.h"
#ifdef HAS_DEMO_SSL
#include "demo_ssl.h"
#include "simcom_ssl.h"
#ifdef HAS_DEMO_FS
#include "simcom_file.h"
#endif
#endif

#define LOG_ERROR(...) sal_log_error("[DEMO-TCP] " __VA_ARGS__)
#define LOG_INFO(...)  sal_log_info("[DEMO-TCP] " __VA_ARGS__)
#define LOG_TRACE(...) sal_log_trace("[DEMO-TCP] " __VA_ARGS__)


static int tcpip_tcpsend_need_size(void);
static int tcpip_udpsend_need_size(void);
static ret_msg_t tcp_demo_task_init(void);
static ret_msg_t tcpip_init_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_deinit_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_tcpsend_init(void);
static ret_msg_t tcpip_tcpsend_uninit(void);
static ret_msg_t tcpip_udpsend_init(void);
static ret_msg_t tcpip_udpsend_uninit(void);
static ret_msg_t tcpip_tcpopen_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_tcpsend_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_tcpsend_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t socket_close_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_udpopen_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_udpsend_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_udpsend_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms));
static ret_msg_t tcpip_serveropen_cmd_proc(op_t op, arg_t *argv, int argn);
static ret_msg_t tcpip_serverclose_cmd_proc(op_t op, arg_t *argv, int argn);

#ifdef HAS_DEMO_SSL
enum demo_ssl_ret demo_sapi_ssl_open(int sock_fd, unsigned char cfg_id, char *hostname);
enum demo_ssl_ret demo_sapi_ssl_close(int cfg_id);
int demo_sapi_ssl_write(int cfg_id, const char *buf, unsigned int len);
int demo_sapi_ssl_read(int cfg_id, char *buf, unsigned int len);
#ifdef HAS_DEMO_FS
static void release_cert(int client_id);
static ret_msg_t read_cert_form_fs(const char *cert_name , char **dest_cert);
#endif
#endif

const value_t tcppdptype_range[] =
{
    {
        .bufp = "IPV4",
        .size = 4,
    },
    {
        .bufp = "IPV6",
        .size = 4,
    },
};

const value_list_t pdptype_range =
{
    .isrange = false,
    .list_head = tcppdptype_range,
    .count = 2,
};

arg_t tcpipinit_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ABGTCP_TCP_PDPTYPE,
        .value_range = &pdptype_range,
    }
};

const menu_list_t func_tcpip_init =
{
    .menu_name = "TCPIP init",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = tcpipinit_args,
        .argn = 1,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_init_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const value_t client_range[] =
{
    {
        .bufp = "TCP_client",
        .size = 10,
    },
#ifdef HAS_DEMO_SSL
    {
        .bufp = "0:SSL_client0",
        .size = 13,
    },
    {
        .bufp = "1:SSL_client1",
        .size = 13,
    },
#endif
};

const value_list_t connectid_range =
{
    .isrange = false,
    .list_head = client_range,
#ifdef HAS_DEMO_SSL
    .count = 3,
#else
    .count = 1,
#endif
};

const value_list_t port_range =
{
    .isrange = true,
    .min = 0,
    .max = 65535,
};

arg_t tcpopen_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGTCP_CONNECT_ID,
        .value_range = &connectid_range,
    },
    {
        .type = TYPE_STR,
        .arg_name = ARGTCP_SERVERIP,
        .msg = "String type, identifies the IP address of server  e.g: 123.45.67.89 or www.example.com"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTCP_TCP_PORT,
        .msg = "Integer type, identifies the port of TCP server",
        .value_range = &port_range,
    }
};

const menu_list_t func_tcp_client_open =
{
    .menu_name = "TCP client open",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = tcpopen_args,
        .argn = 3,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_tcpopen_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

arg_t tcpsend_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGTCP_TCP_SENDSIZE,
        .msg = "Integer type, indicates the length of sending data"
    }
};

const menu_list_t func_tcp_client_send =
{
    .menu_name = "TCP client send",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = tcpsend_args,
        .argn = 1,
        .RAWSizeLink = ARGTCP_TCP_SENDSIZE,
        .methods.__init = tcpip_tcpsend_init,
        .methods.__uninit = tcpip_tcpsend_uninit,
        .methods.cmd_handler = tcpip_tcpsend_cmd_proc,
        .methods.raw_handler = tcpip_tcpsend_raw_proc,
        .methods.needRAWData = tcpip_tcpsend_need_size,
    },
};

const menu_list_t func_socekt_close =
{
    .menu_name = "scoket close",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = socket_close_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_udp_client_open =
{
    .menu_name = "UDP client open",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_udpopen_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

arg_t udpsend_args[] =
{
    {
        .type = TYPE_STR,
        .arg_name = ARGTCP_REMOTEIP,
        .msg = "String type, identifies the IP address of server"
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTCP_UDP_PORT,
        .msg = "Integer type, identifies the port of server",
        .value_range = &port_range,
    },
    {
        .type = TYPE_INT,
        .arg_name = ARGTCP_UDP_SENDSIZE,
        .msg = "Integer type, indicates the length of sending data"
    }
};

const menu_list_t func_udp_client_send =
{
    .menu_name = "UDP client send",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = udpsend_args,
        .argn = 3,
        .RAWSizeLink = ARGTCP_UDP_SENDSIZE,
        .methods.__init = tcpip_udpsend_init,
        .methods.__uninit = tcpip_udpsend_uninit,
        .methods.cmd_handler = tcpip_udpsend_cmd_proc,
        .methods.raw_handler = tcpip_udpsend_raw_proc,
        .methods.needRAWData = tcpip_udpsend_need_size,
    },
};

arg_t tcpserver_args[] =
{
    {
        .type = TYPE_INT,
        .arg_name = ARGTCP_SERVER_LOCALPORT,
        .msg = "Integer type, identifies the listening port of module when used as a TCP server",
        .value_range = &port_range,
    }
};

const menu_list_t func_tcp_server_open =
{
    .menu_name = "TCP server open",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = tcpserver_args,
        .argn = 1,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_serveropen_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_tcp_server_close =
{
    .menu_name = "TCP server close",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_serverclose_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t func_tcpip_deinit =
{
    .menu_name = "TCPIP Deinit",
    .menu_type = TYPE_FUNC,
    .parent = &tcpip_menu,
    .function = {
        .argv = NULL,
        .argn = 0,
        .methods.__init = NULL,
        .methods.__uninit = NULL,
        .methods.cmd_handler = tcpip_deinit_cmd_proc,
        .methods.raw_handler = NULL,
        .methods.needRAWData = NULL,
    },
};

const menu_list_t *tcpip_menu_sub[] =
{
    &func_tcpip_init,
    &func_tcp_client_open,
    &func_tcp_client_send,
    &func_socekt_close,
    &func_udp_client_open,
    &func_udp_client_send,
    &func_tcp_server_open,
    &func_tcp_server_close,
    &func_tcpip_deinit,
    NULL  // MUST end by NULL
};

const menu_list_t tcpip_menu =
{
    .menu_name = "tcpip",
    .menu_type = TYPE_MENU,
    .__init = tcp_demo_task_init,
    .__uninit = NULL,
    .submenu_list_head = tcpip_menu_sub,
    .parent = &root_menu,
};

INT32 pdp_id = 1;

static sal_msgq_ref gClientMsgQueue = NULL;
static sal_task_ref clientProcesser;

static sal_msgq_ref gServerMsgQueue = NULL;
static sal_task_ref serverProcesser;

static sal_msgq_ref sockMutexRef = NULL;
int g_tcp_udp_client_sockfd = -1;
int g_tcp_server_sockfd = -1;
int g_sockfd = -1;
int tcp_sendsize;
int udp_sendsize;
int tcppdpisipv6 = 0;
int tcpip_pdpactive_flag = 0;

sTaskRef g_tcpsend_task_ref;
pub_cache_p g_tcpsend_cache;
int g_tcpsend_flag = 0;
ret_msg_t g_tcpsend_result;

sTaskRef g_udpsend_task_ref;
pub_cache_p g_udpsend_cache;
char *udpremoteIP = NULL;
int udp_port = 0;
int g_udpsend_flag = 0;
ret_msg_t g_udpsend_result;

#ifdef HAS_DEMO_SSL
int g_ssl_cfg_id = -1;
#endif

/**
  * @brief  Get the ip address of the module and convert it into a struct sockaddr structure
  * @param  type
  *            AF_INET： Get ipv4 address
  *            AF_INET6：Get ipv6 address
  *         local_addr struct to hold addresses
  * @note
  * @retval 0 is sucess,other is fail
  */
static int get_ipaddr(int type, struct sockaddr *local_addr)
{
    struct sockaddr_in ipv4sa;
    struct sockaddr_in6 ipv6sa;

    struct in_addr addr1;
    int isIpv6 = 0;
    int ret = -1;
    int result = -1;
    struct SCipInfo ipinfo;

    if (type == AF_INET6)
        isIpv6 = 1;

    ret = sAPI_TcpipGetSocketPdpAddr(1, 1, &ipinfo);
    if (ret != SC_TCPIP_SUCCESS)
        goto exit;

    if (ipinfo.type == TCPIP_PDP_IPV4)
    {
        addr1.s_addr = ipinfo.ip4;
        LOG_INFO("ipinfo.type[%d]", ipinfo.type);

        if (isIpv6 == 1)
        {
            LOG_ERROR("can't get ipv6 addr");
            goto exit;
        }

        LOG_INFO("\r\nPDPCTX type: IPV4,\r\nADDR: %s\r\n", inet_ntoa(addr1));
        memset(&ipv4sa, 0, sizeof(ipv4sa));
        ipv4sa.sin_family = AF_INET;

        memcpy(&ipv4sa.sin_addr, &ipinfo.ip4, sizeof(ipv4sa.sin_addr));
        memcpy(local_addr, &ipv4sa, sizeof(ipv4sa));
    }
    else if (ipinfo.type == TCPIP_PDP_IPV6)
    {
        if (isIpv6 == 0)
        {
            LOG_ERROR("can't get ipv4 addr");
            goto exit;
        }
        char dststr[100] = {0};
        inet_ntop(AF_INET6, ipinfo.ip6, dststr, sizeof(dststr));
        LOG_INFO("\r\nPDPCTX type: IPV6,\r\nADDR: [%s]\r\n", dststr);
        memset(&ipv6sa, 0, sizeof(ipv6sa));
        ipv6sa.sin6_family = AF_INET6;

        memcpy(&ipv6sa.sin6_addr, &ipinfo.ip6, sizeof(ipv6sa.sin6_addr));
        memcpy(local_addr, &ipv6sa, sizeof(ipv6sa));

    }
    else if (TCPIP_PDP_IPV4V6 == ipinfo.type)
    {

        char dststr[100] = {0};
        inet_ntop(AF_INET6, ipinfo.ip6, dststr, sizeof(dststr));
        addr1.s_addr = ipinfo.ip4;
        LOG_INFO("\r\nPDPCTX type: IPV4V6,\r\nADDR: %s\r\nADDR: [%s]\r\n", inet_ntoa(addr1), dststr);
        if (isIpv6 == 1)
        {
            memset(&ipv6sa, 0, sizeof(ipv6sa));
            ipv6sa.sin6_family = AF_INET6;
            memcpy(&ipv6sa.sin6_addr, &ipinfo.ip6, sizeof(ipv6sa.sin6_addr));
            memcpy(local_addr, &ipv6sa, sizeof(ipv6sa));
        }
        else
        {
            memset(&ipv4sa, 0, sizeof(ipv4sa));
            ipv4sa.sin_family = AF_INET;
            memcpy(&ipv4sa.sin_addr, &ipinfo.ip4, sizeof(ipv4sa.sin_addr));
            memcpy(local_addr, &ipv4sa, sizeof(ipv4sa));
        }

    }

    result = 0;


exit:

    return result;
}
/**
  * @brief  Convert struct sockaddr to string
  * @param
  * @note
  * @retval 0 is sucess,other is fail
  */
int Inet_ntop(const struct sockaddr *sa,
              socklen_t salen,
              char str[128],
              UINT16 *port)
{


    switch (sa->sa_family)
    {
        case AF_INET:
        {
            struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, 128) == NULL)
                return -1;

            *port = ntohs(sin->sin_port);
            return 0;
        }
        case AF_INET6:
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, 128) == NULL)
                return (-1);

            *port = ntohs(sin6->sin6_port);
            return 0;
        }
    }

    return -1;
}

/**
  * @brief  initiate a connection on a socket
  * @param
  * @note
  * @retval 0 is sucess,other is fail
  */
static int Connect(int *socketfd, int cid,
                   const char *host,
                   unsigned short port)
{
#define PORT_MAX_LEN   6
    int ret = -1;
    int fd = -1;
    char portstr[PORT_MAX_LEN] = {0};
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    if (socketfd == NULL || host == NULL)
        return -1;

    LOG_INFO("host[%s] port[%d]", host, port);
    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(host, portstr, &hints, &addr_list) != 0)
    {
        LOG_ERROR("getaddrinfo error");
        return -1;
    }

    for (rp = addr_list; rp != NULL; rp = rp->ai_next)
    {
        if ((fd = socket(rp->ai_family, rp->ai_socktype, 0)) < 0)
        {
            continue;
        }
        if ((ret = connect(fd, rp->ai_addr, rp->ai_addrlen)) == 0)
        {
            *socketfd = fd;
            LOG_INFO("connect server sucess");
            break;
        }
        close(fd);
    }

    freeaddrinfo(addr_list);
    return ret;
}

/**
  * @brief  send a message on a socket
  * @param
  * @note
  * @retval
  */
static int Send(int fd,
                const char *data,
                int len,
                int flag,
                const struct sockaddr *destcAddr)
{
    int bytes = 0;
    int index = 0;

    while (len)
    {
        if (destcAddr == NULL)
        {
            bytes = send(fd, data + index, len, flag);
        }
        else
        {
            bytes = sendto(fd, data + index, len, flag, destcAddr, sizeof(struct sockaddr));
        }
        if (bytes < 0)
        {
            return -1;
        }
        else
        {
            len = len - bytes;
            index = index + bytes;
        }
    }

    return index;
}

int udp_send(int socketfd, const char *remoteIP, unsigned short port, void *data, int datelen)
{
    int socket_errno = -1;
    char portstr[PORT_MAX_LEN] = {0};
    struct addrinfo hints;
    struct addrinfo *addr_list = NULL;

    int ret = -1;
    if (socketfd < 0)
    {
        LOG_ERROR("udp not't open [%d]", socketfd);
        return -1;
    }

    snprintf(portstr, sizeof(portstr), "%d", port);
    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family   = AF_UNSPEC/*AF_INET*/;
    hints.ai_protocol = IPPROTO_UDP;

    if (getaddrinfo(remoteIP, portstr, &hints, &addr_list) != 0)
    {
        LOG_ERROR("getaddrinfo error");
        return -1;
    }

    ret = Send(socketfd, data, datelen, 0, addr_list->ai_addr);
    freeaddrinfo(addr_list);
    if (ret == datelen)
    {
        LOG_INFO("send sucess");
    }
    else
    {
        socket_errno = lwip_getsockerrno(socketfd);
        LOG_ERROR("send fail socket_errno[%d]", socket_errno);
        if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
        {
            ret = 0;
        }
        else
        {
            ret = -1;
        }
    }

    return ret;
}

void set_tcp_udp_client_sockfd(int sockfd)
{
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    g_tcp_udp_client_sockfd = sockfd;
    sal_mutex_unlock(sockMutexRef);
}
int get_tcp_udp_client_sockfd(void)
{
    int sockfd = -1;
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    sockfd = g_tcp_udp_client_sockfd;
    sal_mutex_unlock(sockMutexRef);
    return sockfd;
}

#ifdef HAS_DEMO_SSL
static void set_ssl_cfg_id(int sslid)
{
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    g_ssl_cfg_id = sslid;
    sal_mutex_unlock(sockMutexRef);
}
static int get_ssl_cfg_id(void)
{
    int sslid = -1;
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    sslid = g_ssl_cfg_id;
    sal_mutex_unlock(sockMutexRef);
    return sslid;
}
#endif

void set_tcp_server_sockfd(int sockfd)
{
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    g_tcp_server_sockfd = sockfd;
    sal_mutex_unlock(sockMutexRef);
}
int get_tcp_server_sockfd(void)
{
    int sockfd = -1;
    sal_mutex_lock(sockMutexRef, SAL_OS_TIMEOUT_SUSPEND);
    sockfd = g_tcp_server_sockfd;
    sal_mutex_unlock(sockMutexRef);
    return sockfd;

}

static void tcp_udp_client_recv(int fd,int service_type)
{
    fd_set master, read_fds;
    int fdmax = 0;
    int ret = -1;
    struct timeval tv;
    int socket_errno = 0;
    socklen_t addr_len;
    struct sockaddr addr;
    int i = 0;
    char recvbuf[1024];
    char ipstr[128] = {0};
    UINT16 port = 0;
    char indbuf[200];
    int size =0;
    arg_t out_arg[1] = {0};
    arg_t out_arg1[1] = {0};

    if (fd < 0)
    {
        return;
    }
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(fd, &master);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while (1)
    {
        read_fds = master;
        fdmax = fd + 1;
        ret = select(fdmax, &read_fds, NULL, NULL, &tv);
        if (ret > 0)
        {
            for (i = 0; i < fd + 1; i++)
            {
                if (!FD_ISSET(i, &read_fds))
                {
                    continue;
                }
                if (i == fd)
                {
                    memset(recvbuf, 0, sizeof(recvbuf));
                    addr_len = sizeof(struct sockaddr);
                    memset(&addr, 0, sizeof(addr));
#ifdef HAS_DEMO_SSL
                    if(get_ssl_cfg_id()>=0)
                    {
                        ret = demo_sapi_ssl_read(get_ssl_cfg_id(), recvbuf, sizeof(recvbuf));
                    }
                    else
#endif
                    {
                        ret = recvfrom(fd, recvbuf, sizeof(recvbuf), 0, &addr, &addr_len);
                    }
                    if (ret > 0)
                    {
                        memset(ipstr, 0, sizeof(ipstr));
                        if (Inet_ntop(&addr, addr_len, ipstr, &port) != 0)
                        {
                            LOG_ERROR("converts  the  network  address fail");
                        }
                        memset(indbuf,0,sizeof(indbuf));
                        snprintf(indbuf,sizeof(indbuf),"RECV DATA FROM %s[%d]",ipstr,port);
                        out_arg[0].arg_name = indbuf;
                        out_arg[0].type = TYPE_RAW;
                        out_arg[0].value = pl_demo_make_value(0, recvbuf, ret);
                        LOG_INFO("servic_type[%d]0:TCP 1:UDP",service_type);
                        if (service_type == NET_TYPE_TCPCLI)
                        {
                            pl_demo_post_urc(&func_tcp_client_send,TYPE_URC_DATA,out_arg, 1, NULL, 0);
                        }
                        else
                        {
                            pl_demo_post_urc(&func_udp_client_send,TYPE_URC_DATA,out_arg, 1, NULL, 0);
                        }
                        pl_demo_release_value(out_arg[0].value);
                    }
                    else
                    {
                        socket_errno = lwip_getsockerrno(fd);

                        if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
                        {
                            LOG_ERROR("recv fail errno[%d]", socket_errno);
                            memset(indbuf,0,sizeof(indbuf));
                            size = snprintf(indbuf,sizeof(indbuf),"recv fail or client is closed");
                            out_arg1[0].arg_name = "IPCLOSED";
                            out_arg1[0].type = TYPE_RAW;
                            out_arg1[0].value = pl_demo_make_value(0, indbuf,size);
                            pl_demo_post_urc(&func_tcp_client_open,TYPE_URC_NOTICE,out_arg1, 1, NULL, 0);
                            pl_demo_release_value(out_arg1[0].value);
                            goto exit;
                        }
                    }
                }
            }
        }
        else if (ret == 0)
        {
            continue;
        }
        else
        {
            LOG_ERROR("select fail");
            goto exit;
        }
    }
exit:
#ifdef HAS_DEMO_SSL
    if(get_ssl_cfg_id()>=0)
    {
        demo_sapi_ssl_close(get_ssl_cfg_id());
        set_ssl_cfg_id(-1);
    }
#endif
    close(fd);
    set_tcp_udp_client_sockfd(-1);
    return;
}

static void server_recv(int fd)
{
    fd_set master, read_fds;
    int fdmax = 0;
    struct timeval tv;
    int i = 0;
    int fdc = -1;
    struct sockaddr claddr;
    socklen_t len = sizeof(struct sockaddr);
    int ret = -1;
    char revData[1024];
    char indbuf[200];
    arg_t out_arg[1] = {0};
    char ipstr[128] = {0};
    UINT16 port = 0;
    if (fd < 0)
    {
        return;
    }
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(fd, &master);

    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while (1)
    {
        read_fds = master;
        fdmax = fd + 1;
        ret = select(fdmax, &read_fds, NULL, NULL, &tv);
        if (ret > 0)
        {
            for (i = 0; i < fd + 1; i++)
            {
                if (!FD_ISSET(i, &read_fds))
                {
                    continue;
                }
                if (i == fd)
                {
                    memset(&claddr,0,sizeof(claddr));
                    len = sizeof(claddr);
                    fdc = accept(fd, &claddr, &len);
                    LOG_INFO("--- newfd[%d]",fdc);
                    if (fdc < 0)
                    {
                        LOG_ERROR("accept fail ");
                        sAPI_TaskSleep(1 * 200);
                        continue;
                    }
                    while (1)
                    {
                        memset(revData, 0, sizeof(revData));
                        ret = recv(fdc, revData, sizeof(revData), 0);
                        if(ret > 0)
                        {
                            memset(ipstr, 0, sizeof(ipstr));
                            if (Inet_ntop(&claddr, sizeof(claddr), ipstr, &port) != 0)
                            {
                            LOG_ERROR("converts  the  network  address fail");
                            }
                            memset(indbuf,0,sizeof(indbuf));
                            snprintf(indbuf,sizeof(indbuf),"TCP SERVER RECV FROM ip[%s] port[%d]",ipstr,port);
                            out_arg[0].arg_name = indbuf;
                            out_arg[0].type = TYPE_RAW;
                            out_arg[0].value = pl_demo_make_value(0, revData, ret);
                            pl_demo_post_urc(&func_tcp_server_open,TYPE_URC_DATA,out_arg, 1, NULL, 0);
                            pl_demo_release_value(out_arg[0].value);
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
        }
        else if (ret == 0)
        {
            continue;
        }
        else
        {
            LOG_ERROR("select fail");
            goto exit;
        }
    }

exit:
    close(fd);
    set_tcp_server_sockfd(-1);
    return;

}

static void tcp_udp_process(void *arg)
{
    SIM_MSG_T msg = {0, 0, 0, NULL};
    SC_STATUS status;
    int sockfd = 0;
    int service_type = 0;
    while (1)
    {
        status = sal_msgq_recv(gClientMsgQueue, &msg, sizeof(SIM_MSG_T), SAL_OS_TIMEOUT_SUSPEND);
        if (status == SC_SUCCESS)
        {
            sockfd = msg.arg1;
            service_type = msg.arg2;

            tcp_udp_client_recv(sockfd,service_type);
        }
    }


}
static void server_process(void *arg)
{
    SIM_MSG_T msg = {0, 0, 0, NULL};
    SC_STATUS status;
    int sockfd = -1;
    while (1)
    {
        status = sal_msgq_recv(gServerMsgQueue, &msg, sizeof(SIM_MSG_T), SAL_OS_TIMEOUT_SUSPEND);
        if (status == SC_SUCCESS)
        {
            sockfd = msg.arg1;
            server_recv(sockfd);
        }
    }
}

static int inited = 0;
static ret_msg_t tcp_demo_task_init(void)
{
    ret_msg_t ret = {0};
    if (inited == 0)
    {
        if(sal_mutex_create(&sockMutexRef) != 0)
        {
            LOG_ERROR("TCPIP Init create mutex Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init create mutex Fail !";
            return ret;
        }
        if(sal_msgq_create(&gClientMsgQueue, "cliqueue", sizeof(SIM_MSG_T), 1) != 0)
        {
            LOG_ERROR("TCPIP Init create msgq Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init create msgq Fail !";
            return ret;
        }
        if(sal_task_create(&clientProcesser, NULL, SAL_4K, sal_task_priority_low_1, "client", tcp_udp_process, NULL) != 0)
        {
            LOG_ERROR("TCPIP Init create task Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init create task Fail !";
            return ret;
        }

        if(sal_msgq_create(&gServerMsgQueue, "srvqueue", sizeof(SIM_MSG_T), 1) != 0)
        {
            LOG_ERROR("TCPIP Init create msgq Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init create msgq Fail !";
            return ret;
        }
        if(sal_task_create(&serverProcesser, NULL, SAL_4K, sal_task_priority_low_1, "server", server_process, NULL) !=0 )
        {
            LOG_ERROR("TCPIP Init create task Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init create task Fail !";
            return ret;
        }
        inited = 1;
    }
    return ret;
}


static ret_msg_t tcpip_init_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    char indbuf[200];
    arg_t out_arg[1] = {0};
    int size = 0;
    struct sockaddr local_addr;
    const char *tcp_pdptype = NULL;
    if (tcpip_pdpactive_flag == 0)
    {
        pl_demo_get_data(&tcp_pdptype, argv, argn, ABGTCP_TCP_PDPTYPE);
        if (strncasecmp((char *)tcp_pdptype, "IPV6", 4) == 0)
        {
            tcppdpisipv6 = 1;
        }
        else
        {
            tcppdpisipv6 = 0;
        }
        ret.errcode = sAPI_TcpipPdpActive(pdp_id, 1);
        if (ret.errcode == SC_TCPIP_SUCCESS)
        {
            char ipstr[128] = {0};
            UINT16 port = 0;
            int family = AF_INET;

            LOG_INFO("pdp active successful");
            LOG_INFO("tcppdptypeisIPV6[%d]",tcppdpisipv6);
            if (tcppdpisipv6 ==1)
            {
                family = AF_INET6;
            }
            if (get_ipaddr(family, &local_addr) != 0)
            {
                LOG_ERROR("can't get ip address");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "can't get ip address";
                return ret;
            }
            memset(ipstr, 0, sizeof(ipstr));
            if (Inet_ntop(&local_addr, sizeof(local_addr), ipstr, &port) != 0)
            {
                LOG_ERROR("converts  the  network  address fail");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "converts  the  network  address fail";
                return ret;
            }
            tcpip_pdpactive_flag = 1;
            memset(indbuf,0,sizeof(indbuf));
            size = snprintf(indbuf, sizeof(indbuf), "%s", ipstr);
            out_arg[0].arg_name = "ipstr";
            out_arg[0].type = TYPE_RAW;
            out_arg[0].value = pl_demo_make_value(0, indbuf,size);
            pl_demo_post_urc(&func_tcpip_init,TYPE_URC_NOTICE,out_arg, 1, NULL, 0);
            pl_demo_release_value(out_arg[0].value);

        }
        else
        {
            LOG_ERROR("TCPIP Init Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Init Fail !";
            return ret;
        }
    }
    else
    {
        LOG_ERROR("TCPIP Init Fail ! You've done the Init.");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "TCPIP Init Fail ! You've done the Init.";
        return ret;
    }
    return ret;
}

static ret_msg_t tcpip_deinit_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    if (tcpip_pdpactive_flag == 1)
    {
#ifdef HAS_DEMO_SSL
        if(get_ssl_cfg_id()>=0)
        {
            demo_sapi_ssl_close(get_ssl_cfg_id());
            set_ssl_cfg_id(-1);
            LOG_INFO("ssl close id[%d]",get_ssl_cfg_id());
        }
#endif
        if (get_tcp_udp_client_sockfd() >= 0)
        {
            close(get_tcp_udp_client_sockfd());
            set_tcp_udp_client_sockfd(-1);
            LOG_INFO("TCP or UDP client close scokfd[%d]",get_tcp_udp_client_sockfd());
            pl_demo_post_comments("TCP or UDP client close");
        }
        if (get_tcp_server_sockfd() >= 0)
        {
            close(get_tcp_server_sockfd());
            set_tcp_server_sockfd(-1);
            LOG_INFO("TCP server close scokfd[%d]",get_tcp_server_sockfd());
            pl_demo_post_comments("TCP server close");
        }
        ret.errcode = sAPI_TcpipPdpDeactive(pdp_id, 1);
        if(ret.errcode == SC_TCPIP_SUCCESS)
        {
            LOG_INFO("pdp deavtive sucess");
            ret.msg = "pdp deavtive sucess";
            tcppdpisipv6 = 0;
            tcpip_pdpactive_flag = 0;
        }
        else
        {
            LOG_ERROR("TCPIP Deinit Fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "TCPIP Deinit Fail";
            return ret;
        }
    }
    else
    {
        LOG_ERROR("TCPIP Deinit Fail ! You haven't done the Init.");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "TCPIP Deinit Fail ! You haven't done the Init.";
        return ret;
    }
    return ret;
}

static ret_msg_t tcpip_tcpopen_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_STATUS status;
    SIM_MSG_T msg = {0, 0, 0, NULL};
    int  service_type;
    const char *serverIP = NULL;
    const char *connect_id = NULL;
    int ssl_cfg_id = -1;
    int port = 0;

    if (tcpip_pdpactive_flag == 1)
    {
        pl_demo_get_data(&connect_id, argv, argn, ARGTCP_CONNECT_ID);
        pl_demo_get_data(&serverIP, argv, argn, ARGTCP_SERVERIP);
        pl_demo_get_value(&port, argv, argn, ARGTCP_TCP_PORT);
        if (get_tcp_udp_client_sockfd() >= 0)
        {
            LOG_ERROR("socket is busy sockfd[%d]", g_sockfd);
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "socket is busy";
            return ret;
        }
        LOG_INFO("connect_id[%s]",connect_id);
        if (strncasecmp((char *)connect_id, "0:SSL_client0", 13) == 0)
        {
            ssl_cfg_id = 0;
        }
        else if (strncasecmp((char *)connect_id, "1:SSL_client1", 13) == 0)
        {
            ssl_cfg_id = 1;
        }
        if ((ret.errcode = Connect(&g_sockfd, 1, serverIP, port)) == 0)
        {
            if (ssl_cfg_id>= 0)
            {
#ifdef HAS_DEMO_SSL
                if ((ret.errcode = demo_sapi_ssl_open(g_sockfd, ssl_cfg_id, (char *)serverIP)) != 0)
                {
                    LOG_ERROR("ssl open fail");
                    close(g_sockfd);
                    if (ret.errcode == 2)
                    {
                        ret.msg = "SSL_IS_NOT_INIT";
                    }
                    else if (ret.errcode == 3)
                    {
                        ret.msg = "SSL_CLIENT_IS_RUNNING";
                    }
                    else if (ret.errcode == 4)
                    {
                        ret.msg = "SSL_HANDSHAKE_FAILED";
                    }
                    return ret;
                }
                set_ssl_cfg_id(ssl_cfg_id);
#endif
            }
            ret.msg = "connect sucess";
            set_tcp_udp_client_sockfd(g_sockfd);
            service_type = NET_TYPE_TCPCLI;
            memset(&msg, 0, sizeof(msg));
            msg.arg1 = g_sockfd;
            msg.arg2 = service_type;

            status = sal_msgq_send(gClientMsgQueue, &msg, sizeof(SIM_MSG_T), SAL_OS_TIMEOUT_NO_SUSPEND);
            if (status != SC_SUCCESS)
            {
                LOG_ERROR("tcp recv thread busy status[%d]", status);
#ifdef HAS_DEMO_SSL
                if (get_ssl_cfg_id()>=0)
                {
                    demo_sapi_ssl_close(get_ssl_cfg_id());
                    set_ssl_cfg_id(-1);
                }
#endif
                close(g_sockfd);
                set_tcp_udp_client_sockfd(-1);
                g_sockfd = -1;
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "tcp recv thread busy";
                return ret;
            }

        }
        else
        {
            LOG_ERROR("connect [%s:%d] fail", serverIP, port);
            ret.msg = "connect fail";
        }
    }
    else
    {
        LOG_ERROR("You haven't done the Init !");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "You haven't done the Init !";
        return ret;
    }

    return ret;
}

static int tcpip_tcpsend_need_size(void)
{
    return tcp_sendsize;
}

static void tcp_send_task(void *args);

static ret_msg_t tcpip_tcpsend_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_tcpsend_result, sizeof(ret_msg_t));
    tcp_sendsize = 0;
    pub_cache_set_status(g_tcpsend_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_tcpsend_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_tcpsend_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_tcpsend_task_ref = NULL;
        }
    }

    if(g_tcpsend_flag)
    {
        g_tcpsend_flag = 0;
    }

    if (g_tcpsend_cache)
    {
        ret.errcode = pub_cache_delete(&g_tcpsend_cache);
        if (ret.errcode != 0)
        {
            LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;
}

static ret_msg_t tcpip_tcpsend_init(void)
{
    ret_msg_t ret = {0};

    ret = tcpip_tcpsend_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_tcpsend_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_tcpsend_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static ret_msg_t tcpip_tcpsend_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int sendsize = 0;
    if (get_tcp_udp_client_sockfd() < 0)
    {
        LOG_ERROR("You haven't opened the tcp client");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "You haven't opened the tcp client";
        return ret;
    }

    pl_demo_get_value(&sendsize, argv, argn, ARGTCP_TCP_SENDSIZE);
    if (sendsize <= 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    tcp_sendsize = sendsize;

    if (sendsize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_tcpsend_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "TCP SEND",
                                      tcp_send_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }

    return ret;
}

static ret_msg_t tcpip_tcpsend_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (tcp_sendsize != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, tcp_sendsize);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0)
    {
        LOG_INFO("remain_size[%d]",remain_size);
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        LOG_INFO("readsize[%d]",read_size);
        if (read_size < 0)
        {
            LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_tcpsend_cache, buf, read_size);
        if (process_size < read_size)
        {
            LOG_ERROR("raw data cache fail");
            LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait send data to server. */
    pub_cache_set_status(g_tcpsend_cache, CACHE_STOPED);
    while (g_tcpsend_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_tcpsend_result;
}

static void tcp_send_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0, process_size = 0;
    int socket_errno = -1;

    buf = sal_malloc(TCPSEND_DATA_CACHE_MAX);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (tcp_sendsize > 0 && g_tcpsend_cache)
    {
        read_size = tcp_sendsize <= TCPSEND_DATA_CACHE_MAX ? tcp_sendsize : TCPSEND_DATA_CACHE_MAX;

        read_size = pub_cache_read(g_tcpsend_cache, buf, read_size);
        if (read_size <= 0)
        {
            LOG_INFO("cache read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }
#ifdef HAS_DEMO_SSL
        if (get_ssl_cfg_id()>=0)
        {
            process_size = demo_sapi_ssl_write(get_ssl_cfg_id(), buf, read_size);
        }
        else
#endif
        {
            process_size = Send(g_sockfd, buf, read_size, 0, NULL);
        }

        if (process_size == read_size)
        {
            LOG_INFO("send sucess size[%d]",process_size);
            tcp_sendsize -= process_size;
        }
        else
        {
            socket_errno = lwip_getsockerrno(g_sockfd);
            LOG_ERROR("send fail socket_errno[%d]", socket_errno);
            if (socket_errno != EAGAIN /*&& errno != EWOULDBLOCK*/)
            {
#ifdef HAS_DEMO_SSL
                if (get_ssl_cfg_id()>=0)
                {
                    demo_sapi_ssl_close(get_ssl_cfg_id());
                    set_ssl_cfg_id(-1);
                }
#endif
                close(g_sockfd);
                set_tcp_udp_client_sockfd(-1);
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg ="send fail";
                goto EXIT;
            }
        }

    }

EXIT:
    sal_free(buf);
    g_tcpsend_flag = 1;
    g_tcpsend_result = ret;
}

static ret_msg_t socket_close_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int socket_errno = -1;
    if (get_tcp_udp_client_sockfd() < 0)
    {
        LOG_ERROR("no tcp client and udp open");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg ="no tcp client and udp open";
        return ret;
    }
    else
    {
#ifdef HAS_DEMO_SSL
        if (get_ssl_cfg_id()>=0)
        {
            if ((ret.errcode = demo_sapi_ssl_close(get_ssl_cfg_id())) != 0)
            {
                LOG_ERROR("ssl close fail");
                ret.msg ="ssl close fail";
                return ret;
            }
            set_ssl_cfg_id(-1);
        }
#endif
        ret.errcode = close(g_sockfd);
        if (ret.errcode == 0)
        {
            LOG_INFO("close client sucess");
            set_tcp_udp_client_sockfd(-1);
            ret.msg ="close client sucess";
        }
        else
        {
            socket_errno = lwip_getsockerrno(g_sockfd);
            LOG_ERROR("close client fail errno[%d]", socket_errno);
            ret.msg ="close client fail";
        }
    }
    return ret;
}
static ret_msg_t tcpip_udpopen_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    SC_STATUS status;
    int  service_type;
    SIM_MSG_T msg = {0, 0, 0, NULL};
    if (tcpip_pdpactive_flag == 1)
    {
        if (get_tcp_udp_client_sockfd() >= 0)
        {
            LOG_ERROR("socket is busy sockfd[%d]", g_sockfd);
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "socket is busy";
            return ret;
        }

        int family = AF_INET;
        LOG_INFO("tcppdptypeisIPV6[%d]",tcppdpisipv6);
        if (tcppdpisipv6 == 1)
        {
            family = AF_INET6;
        }
        g_sockfd = socket(family, SOCK_DGRAM, 0);
        if (g_sockfd < 0)
        {
            LOG_ERROR("UDP client fail !");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "UDP client fail !";
            return ret;
        }
        else
        {
            set_tcp_udp_client_sockfd(g_sockfd);
            service_type = NET_TYPE_UDPCLI;
            memset(&msg, 0, sizeof(msg));
            msg.arg1 = g_sockfd;
            msg.arg2 = service_type;

            status = sal_msgq_send(gClientMsgQueue, &msg, sizeof(SIM_MSG_T), SAL_OS_TIMEOUT_NO_SUSPEND);
            if (status != SC_SUCCESS)
            {
                LOG_ERROR("udp recv thread busy status[%d]", status);
                close(g_sockfd);
                set_tcp_udp_client_sockfd(-1);
                g_sockfd = -1;
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "udp recv thread busy";
                return ret;
            }
            LOG_INFO("UDP client open success !");
            ret.errcode = 0;
            ret.msg = "UDP client open success !";
        }
    }
    else
    {
        LOG_ERROR("You haven't done the Init !");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "You haven't done the Init !";
        return ret;
    }
    return ret;
}

static int tcpip_udpsend_need_size(void)
{
    return udp_sendsize;
}

static void udp_send_task(void *args);

static ret_msg_t tcpip_udpsend_uninit(void)
{
    ret_msg_t ret = {0};

    pub_bzero(&g_udpsend_result, sizeof(ret_msg_t));
    udp_sendsize = 0;
    pub_cache_set_status(g_udpsend_cache, CACHE_STOPED);

    sAPI_TaskSleep(5);  // wait task exit.

    if (g_udpsend_task_ref)
    {
        ret.errcode = sAPI_TaskDelete(g_udpsend_task_ref);
        if (ret.errcode != 0)
        {
            LOG_ERROR("task delete fail, errcode=%d", ret.errcode);
            goto DONE;
        }
        else
        {
            g_udpsend_task_ref = NULL;
        }
    }

    if(g_udpsend_flag)
    {
        g_udpsend_flag = 0;
    }

    if (g_udpsend_cache)
    {
        ret.errcode = pub_cache_delete(&g_udpsend_cache);
        if (ret.errcode != 0)
        {
            LOG_ERROR("cache delete fail");
            goto DONE;
        }
    }

DONE:
    return ret;
}

static ret_msg_t tcpip_udpsend_init(void)
{
    ret_msg_t ret = {0};

    ret = tcpip_udpsend_uninit();
    if (0 != ret.errcode)
    {
        goto DONE;
    }

    g_udpsend_cache = pub_cache_create(SAL_20K, CACHE_TYPE_BUFFER, CACHE_SYNC_R);
    if (NULL == g_udpsend_cache)
    {
        ret.errcode = ERRCODE_DEFAULT;
        LOG_ERROR("cache create fail");
        goto DONE;
    }

DONE:
    return ret;
}

static ret_msg_t tcpip_udpsend_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    const char *remoteIP = NULL;
    int port = 0;
    int sendsize = 0;
    if (get_tcp_udp_client_sockfd() < 0)
    {
        LOG_ERROR("You haven't opened the udp client");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "You haven't opened the udp client";
        return ret;
    }

    pl_demo_get_data(&remoteIP, argv, argn, ARGTCP_REMOTEIP);
    pl_demo_get_value(&port, argv, argn, ARGTCP_UDP_PORT);
    pl_demo_get_value(&sendsize, argv, argn, ARGTCP_UDP_SENDSIZE);
    if (sendsize <= 0)
    {
        LOG_ERROR("param invalid");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "param invalid";
        return ret;
    }
    udpremoteIP=(char *)remoteIP;
    udp_port= port;
    udp_sendsize = sendsize;

    if (sendsize > 0)
    {
        ret.errcode = sAPI_TaskCreate(&g_udpsend_task_ref,
                                      NULL,
                                      SAL_2K,
                                      150,
                                      "UDP SEND",
                                      udp_send_task,
                                      NULL);
        if (ret.errcode != SC_SUCCESS)
        {
            ret.msg = "task creat fail";
        }
    }

    return ret;
}

static ret_msg_t tcpip_udpsend_raw_proc(int size, int (*raw_read)(char *buf, unsigned int buf_size, int timeout_ms))
{
    ret_msg_t ret = {0};
    int read_size = 0;
    int process_size = 0;
    int remain_size = size;
    char buf[SAL_256] = {0};

    if (udp_sendsize != size)
    {
        LOG_ERROR("raw data size invalid");
        LOG_ERROR("size:%d, need:%d", size, udp_sendsize);
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "raw data size invalid";
        return ret;
    }

    while (remain_size > 0)
    {
        LOG_INFO("remain_size[%d]",remain_size);
        read_size = remain_size <= SAL_256 ? remain_size : SAL_256;
        read_size = raw_read(buf, read_size, 10000);
        if (read_size <= 0)
        {
            LOG_ERROR("raw data read fail");
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "raw data read fail";
            goto EXIT;
        }
        else if (0 == read_size)
        {
            LOG_ERROR("raw data read timeout");
            ret.errcode = ERRCODE_RAW_RECV_TIMEOUT;
            ret.msg = "raw data read timeout";
            goto EXIT;
        }

        process_size = pub_cache_write(g_udpsend_cache, buf, read_size);
        if (process_size < read_size)
        {
            LOG_ERROR("raw data cache fail");
            LOG_ERROR("read size:%d, cache size:%d", read_size, process_size);
            ret.errcode = process_size < 0 ? process_size : ERRCODE_DEFAULT;
            ret.msg = "raw data cache fail";
            goto EXIT;
        }

        remain_size -= read_size;
    }

EXIT:
    /* wait send data to server. */
    pub_cache_set_status(g_udpsend_cache, CACHE_STOPED);
    while (g_udpsend_flag != 1) sAPI_TaskSleep(20);

    if (ret.errcode != 0) return ret;
    else return g_udpsend_result;
}

static void udp_send_task(void *args)
{
    ret_msg_t ret = {0};
    char *buf = NULL;
    int read_size = 0, process_size = 0;

    buf = sal_malloc(TCPSEND_DATA_CACHE_MAX);
    if (!buf)
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    while (udp_sendsize > 0 && g_udpsend_cache)
    {
        read_size = udp_sendsize <= TCPSEND_DATA_CACHE_MAX ? udp_sendsize : TCPSEND_DATA_CACHE_MAX;

        read_size = pub_cache_read(g_udpsend_cache, buf, read_size);
        if (read_size <= 0)
        {
            ret.errcode = read_size < 0 ? read_size : ERRCODE_DEFAULT;
            ret.msg = "cache read fail";
            goto EXIT;
        }

        process_size = udp_send(get_tcp_udp_client_sockfd(), udpremoteIP, udp_port, buf, read_size);
        if (process_size == read_size)
        {
            LOG_INFO("send to [%s:%d] sucess", udpremoteIP, udp_port);
            udp_sendsize -= process_size;
        }
        else
        {
                close(get_tcp_udp_client_sockfd());
                set_tcp_udp_client_sockfd(-1);
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg ="send fail";
                goto EXIT;
        }

    }

EXIT:
    sal_free(buf);
    g_udpsend_flag = 1;
    g_udpsend_result = ret;
}

static ret_msg_t tcpip_serveropen_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int port = 0;
    int fd = -1;
    SC_STATUS status;
    SIM_MSG_T msg = {0, 0, 0, NULL};
    struct sockaddr_in sraddr = {0};
    if (tcpip_pdpactive_flag == 1)
    {
        pl_demo_get_value(&port, argv, argn, ARGTCP_SERVER_LOCALPORT);
        if (get_tcp_server_sockfd() >= 0)
        {
            LOG_ERROR("TCP server socket is busy sockfd[%d]", get_tcp_server_sockfd());
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "You've already opened the TCP server";
            return ret;
        }
        int family = AF_INET;
        LOG_INFO("tcppdptypeisIPV6[%d]",tcppdpisipv6);
        if (tcppdpisipv6 == 1)
        {
            family = AF_INET6;
        }
        fd = socket(family, SOCK_STREAM, 0);
        if (fd >= 0)
        {
            LOG_INFO("server sockfd[%d]",fd);
            sraddr.sin_family = family;
            sraddr.sin_addr.s_addr = htonl(INADDR_ANY);
            sraddr.sin_port = htons(port);
            if(bind(fd, (struct sockaddr *)&sraddr, sizeof(sraddr)) == 0)
            {
                if (listen(fd, 5) < 0)
                {
                    LOG_ERROR("listen socket fail");
                    ret.errcode = ERRCODE_DEFAULT;
                    ret.msg = "listen socket fail";
                    close(fd);
                    return ret;
                }
            }
            else
            {
                LOG_ERROR("bind socket fail");
                ret.errcode = ERRCODE_DEFAULT;
                ret.msg = "bind socket fail";
                close(fd);
                return ret;
            }
        }
        else
        {
            LOG_ERROR("creat server socket fail");
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "creat server socket fail";
            return ret;
        }
        set_tcp_server_sockfd(fd);
        memset(&msg, 0, sizeof(msg));
        msg.arg1 = fd;

        status = sal_msgq_send(gServerMsgQueue, &msg, sizeof(SIM_MSG_T), SAL_OS_TIMEOUT_NO_SUSPEND);
        if (status != SC_SUCCESS)
        {
            LOG_ERROR("server recv thread busy status[%d]", status);
            close(fd);
            set_tcp_server_sockfd(-1);
            ret.errcode = ERRCODE_DEFAULT;
            ret.msg = "server recv thread busy";
            return ret;
        }
    }
    else
    {
        LOG_ERROR("You haven't done the Init !");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "You haven't done the Init !";
        return ret;
    }
    return ret;
}

static ret_msg_t tcpip_serverclose_cmd_proc(op_t op, arg_t *argv, int argn)
{
    ret_msg_t ret = {0};
    int socket_errno = -1;
    if (get_tcp_server_sockfd() < 0)
    {
        LOG_ERROR("no tcp server open");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg ="no tcp server open";
        return ret;
    }
    else
    {
        ret.errcode = close(get_tcp_server_sockfd());
        if (ret.errcode == 0)
        {
            LOG_INFO("close tcp server sucess");
            set_tcp_server_sockfd(-1);
            ret.msg ="close tcp server sucess";
        }
        else
        {
            socket_errno = lwip_getsockerrno(get_tcp_server_sockfd());
            LOG_ERROR("close tcp server fail errno[%d]", socket_errno);
            ret.msg ="close tcp server fail";
        }
    }
    return ret;
}

#ifdef HAS_DEMO_SSL
struct demo_ssl_cfg *g_ssl_cfg_list[SSL_CHANNEL_MAX];
ret_msg_t demo_ssl_handshake_cfg(struct demo_ssl_cfg *ssl_cfg)
{
    ret_msg_t ret = {0};
    unsigned char id = ssl_cfg->client_id;
#ifdef HAS_DEMO_FS
    release_cert(id);
    switch (ssl_cfg->auth_mode)
    {
    case 0:
        break;
    case 1:
        ret = read_cert_form_fs(ssl_cfg->root_cert_path,&g_ssl_cfg_list[id]->root_cert_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->root_cert_path == NULL) g_ssl_cfg_list[id]->root_cert_path = sal_malloc(strlen(ssl_cfg->root_cert_path)+1);
        memcpy(g_ssl_cfg_list[id]->root_cert_path, ssl_cfg->root_cert_path,strlen(ssl_cfg->root_cert_path));
        break;
    case 2:
        ret = read_cert_form_fs(ssl_cfg->root_cert_path,&g_ssl_cfg_list[id]->root_cert_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->root_cert_path == NULL) g_ssl_cfg_list[id]->root_cert_path = sal_malloc(strlen(ssl_cfg->root_cert_path)+1);
        memcpy(g_ssl_cfg_list[id]->root_cert_path, ssl_cfg->root_cert_path,strlen(ssl_cfg->root_cert_path));

        ret = read_cert_form_fs(ssl_cfg->client_cert_path,&g_ssl_cfg_list[id]->client_cert_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->client_cert_path == NULL) g_ssl_cfg_list[id]->client_cert_path = sal_malloc(strlen(ssl_cfg->client_cert_path)+1);
        memcpy(g_ssl_cfg_list[id]->client_cert_path, ssl_cfg->client_cert_path,strlen(ssl_cfg->client_cert_path));

        ret = read_cert_form_fs(ssl_cfg->client_key_path,&g_ssl_cfg_list[id]->client_key_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->client_key_path == NULL) g_ssl_cfg_list[id]->client_key_path = sal_malloc(strlen(ssl_cfg->client_key_path)+1);
        memcpy(g_ssl_cfg_list[id]->client_key_path, ssl_cfg->client_key_path,strlen(ssl_cfg->client_key_path));
        break;
    case 3:
        ret = read_cert_form_fs(ssl_cfg->client_cert_path,&g_ssl_cfg_list[id]->client_cert_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->client_cert_path == NULL) g_ssl_cfg_list[id]->client_cert_path = sal_malloc(strlen(ssl_cfg->client_cert_path)+1);
        memcpy(g_ssl_cfg_list[id]->client_cert_path, ssl_cfg->client_cert_path,strlen(ssl_cfg->client_cert_path));

        ret = read_cert_form_fs(ssl_cfg->client_key_path,&g_ssl_cfg_list[id]->client_key_data);
        if(ret.errcode == ERRCODE_DEFAULT) goto ERROR;
        if(g_ssl_cfg_list[id]->client_key_path == NULL) g_ssl_cfg_list[id]->client_key_path = sal_malloc(strlen(ssl_cfg->client_key_path)+1);
        memcpy(g_ssl_cfg_list[id]->client_key_path, ssl_cfg->client_key_path,strlen(ssl_cfg->client_key_path));
        break;
    default:
        LOG_INFO("invalid value auth_mode = %d.", ssl_cfg->auth_mode);
        ret.msg="invalid value auth_mode.";
        ret.errcode = ERRCODE_DEFAULT;
        goto ERROR;
        break;
    }
#else
    if(ssl_cfg->auth_mode != 0)
    {
        LOG_ERROR("File systems are not supported.");
        ret.msg="File systems are not supported.";
        ret.errcode = ERRCODE_DEFAULT;
        goto ERROR;
    }
#endif /*HAS_DEMO_FS*/
    g_ssl_cfg_list[id]->enable_sni = ssl_cfg->enable_sni;
    g_ssl_cfg_list[id]->ignorelocaltime = ssl_cfg->ignorelocaltime;
    g_ssl_cfg_list[id]->ssl_version = ssl_cfg->ssl_version;
    g_ssl_cfg_list[id]->auth_mode = ssl_cfg->auth_mode;
    g_ssl_cfg_list[id]->negotiate_time_s = ssl_cfg->negotiate_time_s;

    ret.errcode = ERRCODE_OK;
    ret.msg = "ssl cfg set succ.";
    return ret;
ERROR:
    return ret;
}

#ifdef HAS_DEMO_FS
static ret_msg_t read_cert_form_fs(const char *cert_name , char **dest_cert)
{
    ret_msg_t ret = {0};
    SCFILE *fd = NULL;
    int fileSize = 0;

    LOG_INFO("begin loard %s\n", cert_name);
    fd = sAPI_fopen(cert_name, "rb");
    if (!fd)
    {
        LOG_ERROR("file open fail");
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "file open fail";
        goto EXIT;
    }

    fileSize = sAPI_fsize(fd);
    *dest_cert = sal_malloc(fileSize + 1);
    if (!(*dest_cert))
    {
        ret.errcode = ERRCODE_DEFAULT;
        ret.msg = "malloc fail";
        goto EXIT;
    }

    fileSize = sAPI_fread((*dest_cert), 1, fileSize, fd);
    if (fileSize <= 0)
    {
        LOG_ERROR("file read fail");
        ret.errcode = fileSize < 0 ? fileSize : ERRCODE_DEFAULT;
        ret.msg = "file read fail";
        goto EXIT;
    }

EXIT:
    if (fd) sAPI_fclose(fd);
    return ret;
}

static void release_cert(int client_id)
{
    struct demo_ssl_cfg *ssl_cfg = g_ssl_cfg_list[client_id];
    if(ssl_cfg->root_cert_data != NULL)
    {
        sal_free(ssl_cfg->root_cert_data);
        ssl_cfg->root_cert_data = NULL;
    }
    if(ssl_cfg->root_cert_path != NULL)
    {
        LOG_INFO("free root_cert_path : %s ", ssl_cfg->root_cert_path);
        sal_free(ssl_cfg->root_cert_path);
        ssl_cfg->root_cert_path = NULL;
    }
    if(ssl_cfg->client_cert_data != NULL)
    {
        sal_free(ssl_cfg->client_cert_data);
        ssl_cfg->client_cert_data = NULL;
    }
    if(ssl_cfg->client_cert_path != NULL)
    {
        LOG_INFO("free client_cert_path : %s ", ssl_cfg->client_cert_path);
        sal_free(ssl_cfg->client_cert_path);
        ssl_cfg->client_cert_path = NULL;
    }
    if(ssl_cfg->client_key_data != NULL)
    {
        sal_free(ssl_cfg->client_key_data);
        ssl_cfg->client_key_data = NULL;
    }
    if(ssl_cfg->client_key_path != NULL)
    {
        LOG_INFO("free client_key_path : %s ", ssl_cfg->client_key_path);
        sal_free(ssl_cfg->client_key_path);
        ssl_cfg->client_key_path = NULL;
    }
}
#endif /*HAS_DEMO_FS*/

enum demo_ssl_ret demo_sapi_ssl_open(int sock_fd, unsigned char cfg_id, char *hostname)
{
    int client_id = cfg_id;
    if(client_id < 0 || client_id >= SSL_CHANNEL_MAX) return SSL_CFG_ID_ERROR;

    if(g_ssl_cfg_list[client_id] == NULL) return SSL_IS_NOT_INIT;

    struct demo_ssl_cfg *ssl_cfg = g_ssl_cfg_list[client_id];
    if(ssl_cfg->client_is_on_use == TRUE)  return SSL_CLIENT_IS_ON_USE;
    
    SCSslCtx_t sslCtx;

    memset(&sslCtx, 0, sizeof(SCSslCtx_t));
    sslCtx.ClientId = client_id;    //only support 2 links now
    sslCtx.fd = sock_fd;
    sslCtx.ciphersuitesetflg = 0;
    sslCtx.ssl_version = (UINT8)ssl_cfg->ssl_version;
    sslCtx.enable_SNI = (UINT8)ssl_cfg->enable_sni;
    sslCtx.ignore_local_time = (UINT8)ssl_cfg->ignorelocaltime;
    sslCtx.auth_mode = (UINT8)ssl_cfg->auth_mode;
    sslCtx.negotiate_time = (UINT16)ssl_cfg->negotiate_time_s;
    sslCtx.ipstr = (INT8 *)hostname;
    if(ssl_cfg->root_cert_data != NULL)
    {
        sslCtx.root_ca = (INT8 *)ssl_cfg->root_cert_data;
        sslCtx.root_ca_len = strlen(ssl_cfg->root_cert_data) + 1;
    }
    if(ssl_cfg->client_cert_data != NULL)
    {
        sslCtx.client_cert = (INT8 *)ssl_cfg->client_cert_data;
        sslCtx.client_cert_len = strlen(ssl_cfg->client_cert_data) + 1;
    }
    if(ssl_cfg->client_key_data != NULL)
    {
        sslCtx.client_key = (INT8 *)ssl_cfg->client_key_data;
        sslCtx.client_key_len = strlen(ssl_cfg->client_key_data) + 1;
    }

    if (sAPI_SslHandShake(&sslCtx) != 0)
    {
        LOG_ERROR("ssl handshake fail.");
        return SSL_HANDSHAKE_FAILED;
    }

    LOG_INFO("Ssl Handshake success.");
    ssl_cfg->client_is_on_use = TRUE;

    return SSL_OK;
}

enum demo_ssl_ret demo_sapi_ssl_close(int cfg_id)
{
    int client_id = cfg_id;
    if(client_id < 0 || client_id >= SSL_CHANNEL_MAX) return SSL_CFG_ID_ERROR;

    struct demo_ssl_cfg *ssl_cfg = g_ssl_cfg_list[client_id];
    if(ssl_cfg->client_is_on_use == FALSE)  return SSL_ERROR;
    ssl_cfg->client_is_on_use = FALSE;

    LOG_INFO("demo sapi ssl close!!");
    sAPI_SslClose(client_id);
    return SSL_OK;
}

int demo_sapi_ssl_write(int cfg_id, const char *buf, unsigned int len)
{
    int client_id = cfg_id;
    if(client_id < 0 || client_id >= SSL_CHANNEL_MAX) return SSL_CFG_ID_ERROR;

    struct demo_ssl_cfg *ssl_cfg = g_ssl_cfg_list[client_id];
    if(buf == NULL || len <= 0 || ssl_cfg->client_is_on_use == FALSE) return SSL_ERROR;

    LOG_INFO("client_id = %d,buf = %s,len = %d",client_id, buf, len);
    int bytes = sAPI_SslSend(client_id,(INT8 *)buf, len);
    LOG_INFO("demo_sapi_ssl_write bytes=%d", bytes);
    return bytes;
}

int demo_sapi_ssl_read(int cfg_id, char *buf, unsigned int len)
{
    int client_id = cfg_id;
    if(client_id < 0 || client_id >= SSL_CHANNEL_MAX) return SSL_CFG_ID_ERROR;

    struct demo_ssl_cfg *ssl_cfg = g_ssl_cfg_list[client_id];
    if(buf == NULL || len <= 0 || ssl_cfg->client_is_on_use == FALSE) return SSL_ERROR;

    int bytes = sAPI_SslRead(client_id, (INT8 *)buf, len);
    LOG_INFO("demo_sapi_ssl_read bytes=%d", bytes);
    return bytes;
}
#endif /*HAS_DEMO_SSL*/