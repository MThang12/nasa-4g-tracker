/**
  ******************************************************************************
  * @file    demo_app_download.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of app download demo,this demo will show how to download and update app for OpenSDK solution with HTTP\HTTPS\FTP\FTPS server remotely.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "scfw_socket.h"
#include "scfw_netdb.h"
#include "scfw_inet.h"
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
#include "simcom_ssl.h"
#endif
#include "simcom_tcpip.h"
#include "sc_app_download.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "sc_app_update.h"
#include "sc_power.h"
#include "simcom_uart.h"
#include "uart_api.h"
#include "sc_flash.h"

extern void PrintfOptionMenu(char *options_list[], int array_size);
extern void PrintfResp(char *format);

#define APP_OTA_URL "http://47.108.134.22/lugc_fota/customer_app.bin" //support http/https, the url is filled in by the customer
//#define APP_OTA_URL "ftp://guocheng.lu:123456@183.230.174.137:6081/fota_pkg/customer_app.bin" //support http/https, the url is filled in by the customer

//"ftp://username:password@ip_address:port/customer_app.bin

#include "sc_app_download.h"

const char   SC_URI_PORT_DELIM                     = ':';
const char   SC_URI_PATH_DELIM                     = '/';
static BOOL  isIPV6Addr;


#define SC_APP_DOWNLOAD_BUF_SIZE 2048
// extern unsigned int RESERVED_PARTITON_SIZE;

typedef enum
{
    APP_DOWNLOAD_HTTP,
    APP_DOWNLOAD_HTTPS,
} app_download_http_t;
typedef enum
{
    APP_DOWNLOAD_FTP,
    APP_DOWNLOAD_FTPS,
}app_download_ftp_t;

static BOOL sc_parse_server_addr(char *http_url,char *s_addr,short *port,char *path, app_download_http_t *type)
{
    if (type == NULL)
    {
        printf("[%s,%d] param is invalid.", __func__, __LINE__);
        return FALSE;
    }
    isIPV6Addr=FALSE;
    char *uri_ref = NULL;
    const char* tmp = NULL;
    UINT32 path_len = 0;
    UINT32 counter  = 0;
    short server_port = 0;

    int url_len = strlen(http_url);


    if ((http_url == NULL) || (url_len == 0))
    {
        printf(" set url first");
        return FALSE;
    }
    printf("http_url:%s", http_url);

    if (strncmp(http_url, "http://", 7) == 0)
    {
        server_port = 80;
        http_url += 7;
        *type = APP_DOWNLOAD_HTTP;
    }
    else if (strncmp(http_url, "https://", 8) == 0)
    {
        server_port = 443;
        http_url += 8;
        *type = APP_DOWNLOAD_HTTPS;
    }
    else
    {
        printf("ds_http_decode_uri - Invalid protocol type");
        return FALSE;
    }
    uri_ref = http_url;
    printf("%c", *uri_ref);
    if ('[' == *uri_ref)
    {
        do
        {
            if ('\0' == *uri_ref)
            {
                printf("ds_http_decode_uri - IPv6 address end delimiter not found");
                return FALSE;
            }

            uri_ref++;
            counter++;
        } while (']' != *uri_ref);

        uri_ref++;
        counter++;
        isIPV6Addr= TRUE;
    }
    else
    {
        while ('\0' != *uri_ref && SC_URI_PATH_DELIM != *uri_ref && SC_URI_PORT_DELIM != *uri_ref)
        {
            uri_ref++;
            counter++;
        }
    }

    if (0 == counter || SC_FOTA_URL_MAX_SIZE < counter)
    {
        printf("ds_http_decode_uri - Invalid hostname len %d", (int)counter);
        return FALSE;
    }

    memcpy(s_addr,  http_url, counter);

    printf("https server-addr:%s", s_addr);

    if (SC_URI_PORT_DELIM == *uri_ref)
    {
        // get port
        uri_ref++;
        tmp  = uri_ref;
        server_port = 0;

        while ('\0' != *uri_ref)
        {
            if (SC_URI_PATH_DELIM == *uri_ref)
            { break; }

            if (0 == isdigit((UINT8)*uri_ref))
            {
                printf("ds_http_decode_uri - Nondigit port number");

                return FALSE;
            }

            server_port = (server_port * 10) + (*uri_ref - '0');
            uri_ref++;
        }

        if (tmp == uri_ref)
        {
            printf("ds_http_decode_uri - Port delimiter with no port number");

            return FALSE;
        }
    }
    *port = server_port;

    path_len = strlen(uri_ref);
    printf("path_len:%d", (int)path_len);
    if(SC_FOTA_URL_MAX_SIZE < path_len)
    {
        return FALSE;
    }
    else if (0 == path_len)
    {
        path[0] = '/';
    }
    else
    {
        memcpy(path,  uri_ref, path_len);
    }

    printf("https_parse_server_addr out host_p:%s, server_port:%d, type = %d", s_addr, *port, (int)type);
    return TRUE;
}

#ifdef FEATURE_SIMCOM_FTPS
static BOOL sc_parse_ftp_server_addr(char *ftp_url,char *username,char *password,char *s_addr,short *port,char *filename, app_download_ftp_t *type)
{
    if (type == NULL)
    {
        printf("[%s,%d] param is invalid.", __func__, __LINE__);
        return FALSE;
    }
    char *uri_ref = NULL;
    // char tmp_port[10] = {0};
    UINT32 path_len = 0;

    int url_len = strlen(ftp_url);


    if ((ftp_url == NULL) || (url_len == 0))
    {
        printf(" set url first");
        return FALSE;
    }
    printf("ftp_url:%s", ftp_url);

    if (strncmp(ftp_url, "ftp://", strlen("ftp://")) == 0)
    {
        ftp_url += strlen("ftp://");
        *type = APP_DOWNLOAD_FTP;
    }
    else if (strncmp(ftp_url, "ftps://", strlen("ftps://")) == 0)
    {
        ftp_url += strlen("ftps://");
        *type = APP_DOWNLOAD_FTPS;
    }
    else
    {
        printf("ds_ftp_decode_uri - Invalid protocol type");
        return FALSE;
    }
    uri_ref = ftp_url;
    printf("ftp %c", *uri_ref);

    //sscanf(ftp_url, "%[^:]:%[^@]@%[^:]:%[^/]/%s", username, password, s_addr, tmp_port, filename);
    int tmp_port;
    char *start = uri_ref;

    if(strchr(uri_ref,'@'))
    {
        if (sscanf(uri_ref, "%[^:]:%[^@]@", username, password) == 2)
        {
            printf("Username: %s,Password: %s", username,password);
            start = strchr(uri_ref,'@') + 1;
        }
    }

    char *end = strchr(start, '/');
    char *colon = strrchr(start, ':');
    if (colon > end || colon == NULL)
    {
        colon = end;
    }
    strncpy(s_addr, start, colon - start);
    s_addr[colon - start] = '\0';
    tmp_port = atoi(colon + 1);
    //tmp_port = ntohs(tmp_port);
    printf("IP Address: %s,Port: %d", s_addr,tmp_port);

    strcpy(filename, end + 1);
    printf("Filename: %s\n", filename);
    printf("ftp server-addr:%s,%d", s_addr,tmp_port);

    path_len = strlen(filename);
    printf("ftp path_len:%d", (int)path_len);
    if (SC_FOTA_URL_MAX_SIZE < path_len)
    {
        return FALSE;
    }
    //*port = (short)atoi(tmp_port);
    *port = (short)tmp_port;
    printf("ftps_parse_server_addr out username:%s, password:%s, host_p:%s, server_port:%d, type = %d", username, password, s_addr, *port, *type);
    return TRUE;
}
#endif

static BOOL sc_http_get_head(char *buffer_p)
{
    char *findStr_p = NULL;

    findStr_p = strstr((char*)buffer_p, (const char*)"\x0D\x0A\x0D\x0A");

    if (NULL == findStr_p)
    {
        findStr_p = strstr((char*)buffer_p, (const char*)"\x0D\x0D\x0A\x0D\x0D\x0A");
    }

    if (NULL != findStr_p)
        return TRUE;
    else
        return FALSE;
}

static int sc_http_get_content_length(char *buffer_p, int *len_p)
{
    int retValue = -1;
    short i;
    int  crlfLen = 0;
    char  *findStr_p = NULL;
    char *findStatus_p = NULL;
    int http_status = 0;
    findStatus_p = buffer_p;
    *len_p = -1;
    if ((strncmp((char*)findStatus_p, "HTTP/1.1", 8) == 0) || (strncmp((char*)findStatus_p, "HTTP/1.0", 8) == 0))
    {
        findStatus_p += 8;
    }
    else
    {
        goto exit;
    }

    printf("findStatus_p:%x", *findStatus_p);
    while (' ' == *findStatus_p)
    {
        findStatus_p++;
    }
    printf("findStatus_p:%x",  *findStatus_p);
    while (' ' != *findStatus_p)
    {
        printf("*findStatus_p[0x%x]", *findStatus_p);
        if (0 == isdigit((UINT8)*findStatus_p))
        {
            printf("decode http-resp - Nondigit status number");
            break;
        }
        http_status = (http_status * 10) + (*findStatus_p - '0');
        findStatus_p++;
    }

    if(http_status != 200)
    {
        printf("http_status[%d]",http_status);
        goto exit;
    }

    findStr_p = strstr((char*)buffer_p, (const char*)"\x0D\x0A\x0D\x0A");
    crlfLen   = strlen((const char*)"\x0D\x0A\x0D\x0A");

    printf("%s,%s,%d,crlfLen=%d", __FILE__, __FUNCTION__, __LINE__, crlfLen);
    if (NULL == findStr_p)
    {
        findStr_p = strstr((char*)buffer_p, (const char*)"\x0D\x0D\x0A\x0D\x0D\x0A");
        crlfLen   = strlen((const char*)"\x0D\x0D\x0A\x0D\x0D\x0A");
        printf("%s,%s,%d,crlfLen=%d ###", __FILE__, __FUNCTION__, __LINE__, crlfLen);
    }

    if (NULL != findStr_p)
    {
        retValue = (int)((char *)findStr_p - buffer_p + crlfLen);

        printf("%s,%s,%d,retValue=%d ###", __FILE__, __FUNCTION__, __LINE__, retValue);
        if (retValue <= 0)
        {
            retValue = -1;
            goto exit;
        }

        for (i = 0; i < retValue; i++)
        {
            buffer_p[i] = tolower(buffer_p[i]);
        }
        findStr_p = NULL;
        findStr_p = strstr((const char*)buffer_p, (const char*)"content-length:");
        if (NULL != findStr_p)
        {
            findStr_p += strlen((const char*)"content-length:");
            *len_p = atoi(findStr_p);
            printf("The length from http header is %d", *len_p);
            findStr_p = NULL;
        }
        else
        {

            *len_p = -1;
            printf("Failed to get http content-length");
        }

        printf("The length of http header is %d", retValue);
    }

exit:
    return retValue;
}

static inline int app_download_write(int type, int fd, char *buffer, int size,UINT32 client_id)
{
    int ret;
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
    if (type == APP_DOWNLOAD_HTTPS || type == APP_DOWNLOAD_FTPS)
    {
        ret = sAPI_SslSend(client_id, (INT8 *)buffer, size);
    }
    else
#endif
    {
        ret = send(fd, buffer, size, 0);
    }
    return ret;
}

static inline int app_download_read(int type, int fd, char *buffer, int size, UINT32 client_id)
{
    int ret;
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
    if (type == APP_DOWNLOAD_HTTPS || type == APP_DOWNLOAD_FTPS)
    {
        ret = sAPI_SslRead(client_id, (INT8 *)buffer, size);
    }
    else
#endif
    {
        ret = recv(fd, buffer, size, 0);
    }
    return ret;
}

#ifdef FEATURE_SIMCOM_FTPS
static int fota_ftp_tcp_connect(const char *host, unsigned short port,app_download_ftp_t ftp_type,SCAppDownloadPram *pram,int *socket_fd,UINT32 client_id)
{
    int ret = 0;
    int fd = -1;
// int outtime = 0;
    // int set = 0;
    // outtime = 25 * 1000;

    struct addrinfo hints;
    struct addrinfo *addr_list = NULL, *rp = NULL;
    char port_str[6] = {0};

    memset(&hints, 0x0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_protocol = IPPROTO_TCP;

    snprintf(port_str, sizeof(port_str), "%d", port);

    if (getaddrinfo(host, port_str, &hints, &addr_list) != 0)
    {
        printf("getaddrinfo error");
        ret = SC_APP_DOWNLOAD_UNKNOWN_ERR;
        goto exit;
    }

    for (rp = addr_list; rp != NULL; rp = rp->ai_next)
    {
        fd = socket(rp->ai_family, rp->ai_socktype, 0);
        if (fd < 0)
        {
            ret = SC_APP_DOWNLOAD_UNKNOWN_ERR;
            goto exit;
        }
    // set = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &outtime, sizeof(outtime));
    // if (set != 0)
    // {
    //     close(fd);
    //     printf("set socket error!");
    //     return SC_APP_DOWNLOAD_UNKNOWN_ERR;
    // }

        ret = connect(fd, rp->ai_addr, rp->ai_addrlen);

        if (ret == 0) /*Sucess*/
        {
            if (pram->recvtimeout > 0)
            {
                printf("FTP SET SCOKOPT");
                if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &(pram->recvtimeout), sizeof(UINT32)) < 0)
                {
                    printf("set socket opt err");
                    ret = SC_APP_DOWNLOAD_UNKNOWN_ERR;
                    goto exit;
                }
            }

            printf("connect server sucess");
            *socket_fd = fd;
            break;
        }

        close(fd);
    }
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
        if (ftp_type == APP_DOWNLOAD_FTPS)
        {
            SCSslCtx_t ctx;
            memset(&ctx,0,sizeof(ctx));
            ctx.ClientId = client_id;
            ctx.fd = fd;
            ctx.ssl_version = SC_SSL_CFG_VERSION_ALL;
            if (sAPI_SslHandShake(&ctx) != 0)
            {
                printf("[%s,%d] ssl handshake fail.", __func__, __LINE__);
                ret = SC_APP_DOWNLOAD_CONN_REJECT;
                goto exit;
            }
        }
#endif
        return ret;
exit:
    if(fd > 0)
    {
        close(fd);
    }
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
    sAPI_SslClose(1);
#endif
    return ret;
}
static int send_data_command(char* cmd, char* buf, int sock_fd, int type, int client_id)
{

    int ret_value = -1;
    if (sock_fd == -1 || NULL == cmd)
    {
        return (-1);
    }

    printf("ftps: send_command, %s socket, %d ", cmd,sock_fd);
    ret_value=app_download_write(type, sock_fd, cmd, strlen(cmd), client_id);

    return (ret_value);
}
static int recv_command(char* cmd, char* buf, int sock_fd, int type, int client_id)
{
    int read_size = 0;
    int ret_value = -1;
    int need_free_buf = 0;
    if (sock_fd == -1 || NULL == cmd)
    {
        return (-1);
    }
    if (!buf)
    {
        buf = (char*)malloc(sizeof(char) * SC_FOTA_URL_MAX_SIZE);
        need_free_buf = 1;
    }
    read_size = app_download_read(type, sock_fd, buf, sizeof(char) * SC_FOTA_URL_MAX_SIZE, client_id);
    if (read_size > 0)
    {
        printf("ftps: send_command, get reply from socket %d, read size %d", sock_fd, read_size);
        buf[read_size] = '\0';
        ret_value = atol(buf);
        printf("ftps: %s", buf);
    }
    else
    {
        printf("ftps: send_command,get reply from socket %d errror!", sock_fd);
    }
    if (need_free_buf)
    {
        sAPI_Free(buf);
    }
    return (ret_value);
}
static int fota_ftp_get_ftpsreply(int sock_fd,int type,UINT32 client_id)
{
    int reply_code     = 0;
    int count          = 0;
    char rcv_buf[SC_FOTA_URL_MAX_SIZE]  = {0};
    if (-1 == sock_fd)
    {
        printf("Socket [%d] errror!", sock_fd);
        return -1;
    }
    count = app_download_read(type,sock_fd,rcv_buf,SC_FOTA_URL_MAX_SIZE-1,client_id);
    if(count <= 0)
    {
       return -1;
    }
    if (count > 0)
    {
        rcv_buf[count] = '\0';
        reply_code = atol(rcv_buf);
        printf("receive buffer is [%s], reply code = [%d]!", rcv_buf, reply_code);
    }
    else
    {
        printf("Get ftps reply from socket [%d] errror!", sock_fd);
        return -1;
    }

    return reply_code;
}
static int fota_ftps_login(const char *username,const char * password,int sock_fd,int type ,UINT32 client_id)
{
    int reply_code;
    char tmp[SC_FOTA_URL_MAX_SIZE] = {0};
    char user[] = "USER ";
    char pswd[] = "PASS ";
    strncpy(tmp,user,strlen(user));
    strcat(tmp,username);
    strcat(tmp,"\r\n");
    if (app_download_write(type,sock_fd,tmp,strlen(tmp),client_id) < 0)
    {
        printf("Can not send user message!");
        return -1;
    }
    reply_code = fota_ftp_get_ftpsreply(sock_fd,type,client_id);
    if (reply_code == 331)
    {
        memset(tmp,0,SC_FOTA_URL_MAX_SIZE);
        strncpy(tmp,pswd,strlen(pswd));
        strcat(tmp,password);
        strcat(tmp,"\r\n");
        if (app_download_write(type,sock_fd,tmp,strlen(tmp),client_id) < 0)
        {
            printf("Can not send password ");
            return -1;
        }
        reply_code = fota_ftp_get_ftpsreply(sock_fd,type,client_id);
        if (reply_code != 230)
        {
            printf("Password error!");
            return -1;
        }
        return 0;
    }
    else
    {
        if (reply_code == 230)
        {
            printf("Need no pass!");
            return 0;
        }
        printf("User replay error");
        return -1;
    }
}
#endif

#ifdef FEATURE_SIMCOM_FTPS
static int send_command(char *cmd, char *buf, int sock_fd,int type,UINT32 client_id)
{
    int read_size     = 0;
    int need_free_buf = 0;
    int ret_value     = -1;
    if (sock_fd == -1 || NULL == cmd)
    {
        return (-1);
    }
    if (!buf)
    {
        buf = (char *)malloc(sizeof(char) * SC_FOTA_URL_MAX_SIZE);
        need_free_buf = 1;
    }
    printf("ftps: send_command, %s",cmd);
    app_download_write(type,sock_fd,cmd,strlen(cmd),client_id);
    read_size = app_download_read(type,sock_fd,buf,sizeof(char) * SC_FOTA_URL_MAX_SIZE,client_id);
    if (read_size > 0)
    {
        printf("ftps: send_command, get reply from socket %d, read size %d",sock_fd, read_size);
        buf[read_size] = '\0';
        ret_value = atol(buf);
        printf("ftps: %s", buf);
    }
    else
    {
        printf("ftps: send_command,get reply from socket %d errror!", sock_fd);
    }
    if (need_free_buf)
    {
        sAPI_Free(buf);
    }
    return (ret_value);
}
#endif

SCAppDwonLoadReturnCode demo_AppDownload(SCAppDownloadPram *pram)
{
    int ret = 0;
    short port = 0;
    int fd = -1;
    char *header = NULL;
    char *recvbuf = NULL;
    char *buf = NULL;
    BOOL parationisopen = 0;

#ifndef FALCON_1803_PLATFORM
    sAPI_FBF_Disable();//must unlock fbf befor download    1803 does not have this API
#endif
    if(pram == NULL || pram->mod >= SC_APP_DOWNLOAD_MAX || pram->url == NULL)
    {
        ret = SC_APP_DOWNLOAD_PARAM_INVALID;
        goto exit;
    }

    if(sAPI_TcpipPdpActive(1,1) != 0)
    {
        return SC_APP_DOWNLOAD_PDP_ACTIVE_FAIL;
    }

    switch(pram->mod)
    {
    case SC_APP_DOWNLOAD_HTTP_MOD:
    {
        char addr[SC_FOTA_URL_MAX_SIZE],path[SC_FOTA_URL_MAX_SIZE];
        char hostname[SC_FOTA_URL_MAX_SIZE]={0};

        int headMaxlen = SC_APP_DOWNLOAD_BUF_SIZE;
        int size = 0;

        int headlen = 0;
        int contentlen = 0;
        int packeglen = 0;
        app_download_http_t http_type;

        memset(addr,0,sizeof(addr));
        memset(path,0,sizeof(path));

        if(sc_parse_server_addr(pram->url,addr,&port,path, &http_type) == FALSE)
        {
            ret = SC_APP_DOWNLOAD_PARAM_INVALID;
            goto exit;
        }

        struct addrinfo hints;
        struct addrinfo *addr_list = NULL, *rp = NULL;
        memset(&hints, 0x0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_UNSPEC;
        hints.ai_protocol = IPPROTO_TCP;
        char port_str[10] = {0};
        snprintf(port_str, sizeof(port_str), "%d", port);
        if(isIPV6Addr)
        {
            memcpy(hostname,addr+1,strlen(addr)-2);
        }
        else
        {
            memcpy(hostname,addr,strlen(addr));
        }
        if (getaddrinfo(hostname,port_str , &hints, &addr_list) != 0)
        {
            printf("getaddrinfo error");
            ret = SC_APP_DOWNLOAD_UNKNOWN_ERR;
            goto exit;
        }
        for (rp = addr_list; rp != NULL; rp = rp->ai_next)
        {
            fd = socket(rp->ai_family, rp->ai_socktype, 0);
            if (fd < 0)
            {
                printf("creat socket err continue");
                continue;
            }
            printf("fd is %d",fd);

          ret= connect(fd, (struct sockaddr *)rp->ai_addr, sizeof(struct sockaddr));
            if (ret == 0)
            {
                if (pram->recvtimeout > 0)
                {
                    printf("HTTP SET SCOKOPT");
                    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &(pram->recvtimeout), sizeof(UINT32)) < 0)
                    {
                        printf("set socket opt err");
                        ret = SC_APP_DOWNLOAD_UNKNOWN_ERR;
                        goto exit;
                    }
                }

                printf("connect server sucess");
                break;
            }

            close(fd);
        }
         freeaddrinfo(addr_list);

#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
        if (http_type == APP_DOWNLOAD_HTTPS)
        {
                SCSslCtx_t ctx;
                memset(&ctx,0,sizeof(ctx));
                ctx.fd = fd;
                ctx.ssl_version = SC_SSL_CFG_VERSION_ALL;
                if (sAPI_SslHandShake(&ctx) != 0)
                {
                    printf("[%s,%d] ssl handshake fail.", __func__, __LINE__);
                    ret = SC_APP_DOWNLOAD_CONN_REJECT;
                    goto exit;
                }
        }
#endif
        header = sAPI_Malloc(headMaxlen);
        if(header == NULL)
        {
            ret = SC_APP_DOWNLOAD_NO_MEMORY;
            goto exit;
        }

        printf("path[%s]",path);
        printf("addr[%s]",addr);

        size += snprintf(header, headMaxlen - size
            , "GET %s HTTP/1.%d\r\nHost: %s\r\nCache-Control: no-cache\r\nContent-Type: %s\r\nAccept: %s\r\n\r\n\r\n"
            , path, 1, addr, "text/plain", "*/*");

        printf("size[%d] header[%s]",size,header);

        ret = app_download_write(http_type, fd, header, size,0);
        if(ret <= 0)
        {
           ret = SC_APP_DOWNLOAD_SEND_ERR;
           goto exit;
        }

        recvbuf = sAPI_Malloc(SC_APP_DOWNLOAD_BUF_SIZE);
        if(recvbuf == NULL)
        {
            ret = SC_APP_DOWNLOAD_NO_MEMORY;
            goto exit;

        }

        memset(recvbuf,0x0,SC_APP_DOWNLOAD_BUF_SIZE);


        int rebytes = 0;
        int errorno = 0;
        while(1)
        {
            ret = app_download_read(http_type, fd, recvbuf+rebytes, SC_APP_DOWNLOAD_BUF_SIZE-rebytes,0);

            if(ret <= 0)
            {
                errorno = lwip_getsockerrno(fd);
                if((errorno == EAGAIN) || (errorno == EINTR))
                {
                    continue;
                }

                printf("%s,%s,%d,recv err", __FILE__, __FUNCTION__, __LINE__);
                ret = SC_APP_DOWNLOAD_RECV_ERR;
                goto exit;
            }

            rebytes += ret;
            if (sc_http_get_head(recvbuf) == TRUE)
            {
                ret = rebytes;
                printf("%s,%s,%d,get http head!", __FILE__, __FUNCTION__, __LINE__);
                break;
            }

        }

        printf("ret[%d] recvbuf[%s]",ret,recvbuf);
        headlen = sc_http_get_content_length(recvbuf,&contentlen);

        printf("headlen[%d] contentlen[%d]",headlen,contentlen);
        // if(contentlen > RESERVED_PARTITON_SIZE)
        // {
        //     printf("the adiff file is too:%d,RESERVED_PARTITON_SIZE=%d",contentlen,RESERVED_PARTITON_SIZE);
        //     ret = SC_APP_DOWNLOAD_NO_MEMORY;
        //     goto exit;
        // }
        if(headlen < 0 || contentlen <= 0)
        {
            ret = SC_APP_DOWNLOAD_FILE_NOT_EXIST;
            goto exit;
        }

        if(sAPI_AppPackageOpen("w") != SC_SUCCESS)
        {
            ret = SC_APP_DOWNLOAD_OPEN_PARTITION_FAIL;
            goto exit;
        }

        parationisopen = 1;
        if(ret - headlen > 0)
        {
            if(sAPI_AppPackageWrite(recvbuf+headlen,ret - headlen) != SC_SUCCESS)
            {
                ret = SC_APP_DOWNLOAD_WRITE_PARTITION_FAIL;
                goto exit;
            }
            packeglen += ret - headlen;
        }

        while(1)
        {

            printf("packeglen[%d]  contentlen[%d]",packeglen,contentlen);

            if(packeglen == contentlen)
            {
                ret = SC_APP_DOWNLOAD_SUCESSED;
                goto exit;
            }

            memset(recvbuf,0x0,SC_APP_DOWNLOAD_BUF_SIZE);
            if(contentlen - packeglen > SC_APP_DOWNLOAD_BUF_SIZE)
                ret = app_download_read(http_type, fd, recvbuf, SC_APP_DOWNLOAD_BUF_SIZE,0);
            else
                ret = app_download_read(http_type, fd, recvbuf, contentlen - packeglen,0);

            printf("recv ret[%d]\r\n",ret);
            if(ret < 0)
            {
                errorno = lwip_getsockerrno(fd);
                if((errorno == EAGAIN) || (errorno == EINTR))
                {
                    continue;
                }

                ret = SC_APP_DOWNLOAD_RECV_ERR;
                goto exit;
            }

            if(sAPI_AppPackageWrite(recvbuf,ret) != SC_SUCCESS)
            {
                ret = SC_APP_DOWNLOAD_WRITE_PARTITION_FAIL;
                goto exit;
            }

            packeglen+=ret;

        }
            break;
    }
#ifdef FEATURE_SIMCOM_FTPS
    case SC_APP_DOWNLOAD_FTP_MOD:
    {
        char addr[SC_FOTA_URL_MAX_SIZE] = {0};
        char filepath[SC_FOTA_URL_MAX_SIZE] = {0};
        char username[SC_FOTA_URL_MAX_SIZE] = {0};
        char password[SC_FOTA_URL_MAX_SIZE] = {0};
        char data_addr[SC_FOTA_URL_MAX_SIZE] = {0};
        char cmd[SC_FOTA_URL_MAX_SIZE+30] = {0};
        char *pos = NULL;
        int ret_value = 0;
        int file_size = 0;
        unsigned int data_port =0;
        int data_fd = -1;
        int err = 0;
        int total_size = 0;
        app_download_ftp_t type;
        printf("FTP START FTP DOWNLOAD");
        if(sc_parse_ftp_server_addr(pram->url,username,password,addr,&port,filepath,&type) == FALSE)
        {
            printf("FTP PARSE FAIL");
            ret = SC_APP_DOWNLOAD_PARAM_INVALID;
            goto exit;
        }
        /*login the ftp server*/
        printf("FTP START FTP CONNECT");
        if(fota_ftp_tcp_connect(addr,port,type,pram,&fd,0) != SC_APP_DOWNLOAD_SUCESSED)
        {
            printf("FTP CONNECT FAIL");
            ret = SC_APP_DOWNLOAD_CONN_REJECT;
            goto exit;
        }
        if(fd != -1)
        {
            printf("FTP GET REPLY");
            if(fota_ftp_get_ftpsreply(fd,type,0) != 220)
            {

                ret = SC_APP_DOWNLOAD_CONN_REJECT;
                goto exit;
            }
            printf("FTP LOGIN");
            if(fota_ftps_login(username,password,fd,type,0)== 0)
            {
                if(type == APP_DOWNLOAD_FTPS)
                {
                    err = send_command("PBSZ 0\r\n", NULL,fd,type,0);
                    if((err != 200) && (err != 220))
                    {
                        ret = SC_APP_DOWNLOAD_CONN_REJECT;
                        goto exit;
                    }
                    if((err = send_command("PROT P\r\n", NULL, fd,type,0)) != 200)
                    {
                        ret = SC_APP_DOWNLOAD_CONN_REJECT;
                        goto exit;
                    }
                }
            }
            else
                {
                    printf("Login error");
                    ret = SC_APP_DOWNLOAD_CONN_REJECT;
                    goto exit;
                }
        }else
        {
            printf("Login error");
            ret = SC_APP_DOWNLOAD_CONN_REJECT;
            goto exit;
        }
        /*get the file*/
        recvbuf = sAPI_Malloc(SC_APP_DOWNLOAD_BUF_SIZE);
        if(recvbuf == NULL)
        {
            ret = SC_APP_DOWNLOAD_NO_MEMORY;
            goto exit;

        }
        memset(recvbuf,0x0,SC_APP_DOWNLOAD_BUF_SIZE);
        buf = sAPI_Malloc(SC_APP_DOWNLOAD_BUF_SIZE);
        if (NULL == buf)
        {
            printf("error,[%s], malloc failed", __FUNCTION__);
            ret = SC_APP_DOWNLOAD_NO_MEMORY;
            goto exit;
        }
        memset(buf,0,SC_APP_DOWNLOAD_BUF_SIZE);
        snprintf(cmd, SC_FOTA_URL_MAX_SIZE+30, "SIZE %s\r\n", filepath);
        ret_value = send_command(cmd, buf, fd,type,0);
        printf("ret_value = [%d]", ret_value);
        if (ret_value != 213)
        {
            return SC_APP_DOWNLOAD_UNKNOWN_ERR;
            goto exit;
        }
        pos = buf + 4;
        file_size = atol(pos);
        // if(file_size > RESERVED_PARTITON_SIZE)
        // {
        //     printf("the adiff file is too:%d,RESERVED_PARTITON_SIZE=%d",file_size,RESERVED_PARTITON_SIZE);
        //     ret = SC_APP_DOWNLOAD_NO_MEMORY;
        //     goto exit;
        // }
        memset(buf,0,SC_APP_DOWNLOAD_BUF_SIZE);
        //sAPI_Free(buf);
        struct sockaddr addrf = {0};
        socklen_t addr_len = sizeof(struct sockaddr);

        if (getsockname(fd, &addrf, &addr_len) != 0)
        {
            return SC_APP_DOWNLOAD_UNKNOWN_ERR;
            goto exit;
        }
        if(AF_INET == addrf.sa_family)
        {
            if (send_command("PASV\r\n", buf, fd, type, 0) != 227)
            {
                printf("error, pasv mode is not support by remote server.");
                return SC_APP_DOWNLOAD_UNKNOWN_ERR;
                goto exit;
            }
            printf("recv PASV rsp is [%s]", buf);
            int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, port1 = 0, port2 = 0;
            sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2);
            sprintf(data_addr, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
            data_port = port1 * 256 + port2;
        }
        else
        {
            char formatter[30] = {0};
            char splitchr = '|';
            char port_str[10] = {0};
            if (send_command("EPSV\r\n", buf, fd, type, 0) != 229)
            {
                printf("error, pasv mode is not support by remote server.");
                return SC_APP_DOWNLOAD_UNKNOWN_ERR;
                goto exit;
            }
            sscanf(buf,"%*[^(]%*[(]%c",&splitchr);
            printf("server split char: [%c],socket %d", splitchr,fd);
            snprintf(formatter, sizeof(formatter), "%%*[^(]%%*[(]%c%c%c%%u%c)", splitchr, splitchr, splitchr, splitchr);
            printf("server formmter: [%s]socket %d", formatter,fd);
            sscanf(buf, formatter, &data_port);
            // sscanf(buf, "229 Entering Extended Passive Mode (|||%u|)", &data_port);
            snprintf(port_str, sizeof(port_str), "%u", data_port);
            printf("hostname:%s, PASV PORT: %u,socket %d", addr,data_port,fd);
        }

        if (send_command("TYPE I\r\n", NULL, fd, type, 0) != 200)
        {
            ret = SC_APP_DOWNLOAD_CONN_REJECT;
            goto exit;
        }
        sprintf(cmd, "RETR %s\r\n", filepath);
        int res = send_data_command(cmd, NULL, fd, type, 0);
        if (fota_ftp_tcp_connect(addr, data_port, type, pram, &data_fd, 1) != SC_APP_DOWNLOAD_SUCESSED)
        {
            ret = SC_APP_DOWNLOAD_CONN_REJECT;
            goto exit;
        }
        res=recv_command(cmd, NULL, fd, type,0);
        if (res != 150 && res != 125)
        {
            ret = SC_APP_DOWNLOAD_CONN_REJECT;
            goto exit;
        }
        if(sAPI_AppPackageOpen("w") != SC_SUCCESS)
        {
            ret = SC_APP_DOWNLOAD_OPEN_PARTITION_FAIL;
            goto exit;
        }

        parationisopen = 1;
        do
        {
            ret = 0;
            if(total_size == file_size)
            {
                ret = SC_APP_DOWNLOAD_SUCESSED;
                goto exit;
            }
            ret = app_download_read(type,data_fd,recvbuf,SC_APP_DOWNLOAD_BUF_SIZE,1);
            printf("recv ret[%d]\r\n",ret);
            if(ret < 0)
            {
                if(total_size < file_size)
                {
                    ret = SC_APP_DOWNLOAD_RECV_ERR;
                    goto exit;
                }
            }
            if(sAPI_AppPackageWrite(recvbuf,ret) != SC_SUCCESS)
            {
                ret = SC_APP_DOWNLOAD_WRITE_PARTITION_FAIL;
                goto exit;
            }
            total_size += ret;
        } while (total_size <= file_size);
    break;
    }
#endif
    default:
        printf("ds_decode_uri - notsupprt protocol type");
    break;
    }
exit:
    if(fd > 0)
    {
        close(fd);
    }
#if defined(FEATURE_SIMCOM_MSSL) || defined(SIMCOM_OPENSDK_SSL)
    sAPI_SslClose(0);
    sAPI_SslClose(1);
#endif
    if(header != NULL)
    {
        sAPI_Free(header);
        header = NULL;
    }
    if(recvbuf != NULL)
    {
        sAPI_Free(recvbuf);
        recvbuf = NULL;
    }
    if (NULL != buf)
    {
        sAPI_Free(buf);
        buf = NULL;
    }
    if(parationisopen == 1)
        sAPI_AppPackageClose();

    sAPI_TcpipPdpDeactive(1,1);
    return ret;
}


/**
  * @brief  Demo for app download with HTTP\FTP, need to upload new customer_app.bin file to server in advance.
  * @param  void
  * @note   Need to select FTP or HTTP by inputing in serial terminal.
  * @retval void
  */
void AppDownloadDemo(void)
{
    UINT32 opt = 0;
    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
        "1. HTTP APP DOWNLOAD",
        "2. HTTPS APP DOWNLOAD",
        "3. FTP APP DOWNLOAD",
        "4. FTPS APP DOWNLOAD",
        "5. APP DOWNLOAD TEST",
        "99. back",
    };


    while (1) {
        PrintfResp(note);
        PrintfOptionMenu(options_list, sizeof(options_list) / sizeof(options_list[0]));

        opt = UartReadValue();

        switch (opt) {
        case 1:
        case 2:
        case 3:
        case 4: {
            SCAppDownloadPram pram;
            SCAppDwonLoadReturnCode ret = SC_APP_DOWNLOAD_SUCESSED;
            SCAppPackageInfo gAppUpdateInfo = {0};

            switch (opt)
            {
            case 1:
                pram.url = "http://47.108.134.22/zyk_fota/customer_app.bin";
                break;
            case 2:
                pram.url = "https://183.230.174.137:6066/http_share/customer_app.bin";
                break;
            case 3:
                pram.url = "ftp://yangkui.zhang:simcom@183.230.174.137:6121/zyk_fota/customer_app.bin";
                break;
            case 4:
                pram.url = "ftps://mouwy:123@183.230.174.137:6134/customer_app.bin";
                break;
            default:
                // pram.url = APP_OTA_URL;
                break;
            }
            if (strncmp(pram.url, "https://", strlen("https://")) == 0 || strncmp(pram.url, "http://", strlen("http://")) == 0) {
                pram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
            }
            if (strncmp(pram.url, "ftp://", strlen("ftp://")) == 0 || strncmp(pram.url, "ftps://", strlen("ftps://")) == 0) {
                pram.mod = SC_APP_DOWNLOAD_FTP_MOD;
            }
            pram.recvtimeout = 20000;

            ret = demo_AppDownload(&pram);
            if ( ret == SC_APP_DOWNLOAD_SUCESSED )
                PrintfResp("app download success.\r\n");
            else
                PrintfResp("app download fail.\r\n");
            printf("demo_AppDownload download customer_app.bin ret[%d] ... ", ret);

            ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
            if (SC_APP_DOWNLOAD_SUCESSED == ret) {
                printf("app crc success --> reset!");
                PrintfResp("app crc success.\r\n");
                sAPI_TaskSleep(20);
                sAPI_SysReset();
            } else {
                printf("app package crc fail, errcode = [%d]", ret);
                PrintfResp("app crc fail.\r\n");

            }
            break;
        }

        case 88:
            PrintfResp("app download test!\r\n");
            break;

        case 99:

            return;

        }

    }
}

void sAPI_app_download_test(void)
{
	SCAppDownloadPram cpram;
	SCAppDwonLoadReturnCode cret = SC_APP_DOWNLOAD_SUCESSED;
	SCAppPackageInfo cgAppUpdateInfo = {0};

    sAPI_TaskSleep(200);

    //cpram.url = "http://47.108.134.22/zyk_fota/customer_app.bin";
    cpram.url = "ftp://yangkui.zhang:simcom@183.230.174.137:6121/zyk_fota/customer_app.bin";

    if (strncmp(cpram.url, "https://", strlen("https://")) == 0 || strncmp(cpram.url, "http://", strlen("http://")) == 0)
    {
        cpram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
    }
    if (strncmp(cpram.url, "ftp://", strlen("ftp://")) == 0 || strncmp(cpram.url, "ftps://", strlen("ftps://")) == 0)
    {
        cpram.mod = SC_APP_DOWNLOAD_FTP_MOD;
    }
    cpram.recvtimeout = 20000;

    cret = demo_AppDownload(&cpram);
    if ( cret == SC_APP_DOWNLOAD_SUCESSED )
        PrintfResp("app download success.\r\n");
    else
        PrintfResp("app download fail.\r\n");
    printf("demo_AppDownload download customer_app.bin ret[%d] ... ", cret);

    cret = sAPI_AppPackageCrc(&cgAppUpdateInfo);
    if (SC_APP_DOWNLOAD_SUCESSED == cret)
	{
        PrintfResp("app crc success --> reset!");
        sAPI_TaskSleep(20);
        sAPI_SysReset();
    }
    else
    {
        printf("app package crc fail, errcode = [%d]", cret);
        PrintfResp("app crc fail.\r\n");
    }
}


#if 0
#include "simcom_api.h"

#define APP_OTA_TASK_SIZE (1024 * 10)
#define APP_OTA_URL "https://test.com/customer_app.bin" //support http/https, the url is filled in by the customer

static sTaskRef app_download_task_ref;

static void sAPP_AppDownloadProcesser(void *arg)
{
    SCAppDownloadPram pram;
    SCAppDwonLoadReturnCode ret;
    SCAppPackageInfo gAppUpdateInfo = {0};
    printf("sAPP_FotaProcesser start");
    int pGreg = 0;

    while (1) {
        sAPI_NetworkGetCgreg(&pGreg);
        if (1 != pGreg) {
            printf("fota NETWORK STATUS IS [%d]", pGreg);
            sAPI_TaskSleep(5 * 200);
        } else {
            printf("fota NETWORK STATUS IS NORMAL");
            break;
        }
    }
    pram.url = APP_OTA_URL;
    if (strncmp(pram.url, "https://", strlen("https://")) == 0 || strncmp(pram.url, "http://", strlen("http://")) == 0) {
        pram.mod = SC_APP_DOWNLOAD_HTTP_MOD;
    }
    if (strncmp(pram.url, "ftp://", strlen("ftp://")) == 0 || strncmp(pram.url, "ftps://", strlen("ftps://")) == 0) {
        pram.mod = SC_APP_DOWNLOAD_FTP_MOD;
    }
    pram.recvtimeout = 5000;

    ret = sAPI_AppDownload(&pram);

    printf("sc_app_download_test download 2.bin ret[%d] ... ", ret);


    ret = sAPI_AppPackageCrc(&gAppUpdateInfo);
    if (SC_APP_DOWNLOAD_SUCESSED == ret) {
        printf("the next is reset");
        sAPI_SysReset();
    } else
        printf("app package crc fail, errcode = [%d]", ret);

}

void sAPP_AppDownloadTaskDemo(void)
{

    SC_STATUS status;

    status = sAPI_TaskCreate(&app_download_task_ref, NULL, APP_OTA_TASK_SIZE, 70, (char *)"fota_demo_thread", sAPP_AppDownloadProcesser, (void *)0);
    if (status != SC_SUCCESS)
        printf("create app_download task fail status[%d]", status);
}
#endif
