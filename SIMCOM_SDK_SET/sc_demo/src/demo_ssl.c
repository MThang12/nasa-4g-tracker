/**
  ******************************************************************************
  * @file    demo_ssl.c
  * @author  SIMCom OpenSDK Team
  * @brief   Source file of ssl feature.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 SIMCom Wireless.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "simcom_ssl.h"
#include "simcom_tcpip.h"
#include "simcom_tcpip_old.h"
#include "simcom_debug.h"
#include "simcom_os.h"
#include "simcom_common.h"
#include "uart_api.h"


typedef enum{
    SC_SSL_COMMUNICATE          = 1,
    SC_SSL_TEST                 = 2,
    SC_SSL_TWO_WAY_AUTH         = 3,
    SC_SSL_DEMO_MAX             = 99
}SC_SSL_DEMO_TYPE;

extern void PrintfOptionMenu(char* options_list[], int array_size);
extern void PrintfResp(char* format);
extern void SslTestDemoInit(int sslTestTime);

static void ssl_verify_demo(void);

/**
  * @brief  SSL Demo.
  * @param  void
  * @note
  * @retval void
  */
void SslDemo(void)
{
    INT32 ret = 0;
    SCSslCtx_t ctx;
    INT32 sockfd = -1;
    SChostent *host_entry = NULL;
    SCsockAddrIn server;
    UINT32 opt = 0;
    INT8 recvbuf[1024];
    INT8 sendbuf[] = { /* Packet 8 */
    0x47, 0x45, 0x54, 0x20, 0x2f, 0x20, 0x48, 0x54,
    0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a,
    0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x77, 0x77,
    0x77, 0x2e, 0x62, 0x61, 0x69, 0x64, 0x75, 0x2e,
    0x63, 0x6f, 0x6d, 0x0d, 0x0a, 0x43, 0x61, 0x63,
    0x68, 0x65, 0x2d, 0x43, 0x6f, 0x6e, 0x74, 0x72,
    0x6f, 0x6c, 0x3a, 0x20, 0x6e, 0x6f, 0x2d, 0x63,
    0x61, 0x63, 0x68, 0x65, 0x0d, 0x0a, 0x43, 0x6f,
    0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79,
    0x70, 0x65, 0x3a, 0x20, 0x74, 0x65, 0x78, 0x74,
    0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x0d, 0x0a,
    0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20,
    0x2a, 0x2f, 0x2a, 0x0d, 0x0a, 0x0d, 0x0a, 0x0d,
    0x0a };

    char *note = "\r\nPlease select an option to test from the items listed below.\r\n";
    char *options_list[] = {
    "1. SSL CONNECT DEMO",
    "2. TEST FOR SSL",
    "3. TWO WAY AUTH",
    "99. back",
     };
     /*{
    "1. test for ssl",
    "99. back",
     }; */

    while(1)
    {
        PrintfResp(note);
        PrintfOptionMenu(options_list,sizeof(options_list)/sizeof(options_list[0]));

        opt = UartReadValue();

        switch(opt)
        {
            case SC_SSL_COMMUNICATE:
                {
                    char path[1024 + 256] = {0};
                    if (-1 == sAPI_TcpipPdpActive(1,1))
                    {
                        sAPI_Debug("PDP active err");
                        snprintf(path,sizeof(path),"\r\nPDP active err\r\n");
                        PrintfResp(path);
                        break;
                    }


                    sockfd = sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0);

                    sAPI_Debug("demo_ssl_test sockfd[%d]",sockfd);
                    if(sockfd < 0)
                    {
                        sAPI_Debug("create socket err");
                        snprintf(path,sizeof(path),"\r\ncreate socket err\r\n");
                        PrintfResp(path);
                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;
                    }

                    host_entry = sAPI_TcpipGethostbyname((INT8 *)"www.baidu.com");
                    if (host_entry == NULL)
                    {
                        sAPI_SslClose(0);
                        sAPI_TcpipClose(sockfd);
                        sAPI_Debug("DNS gethostbyname fail");
                        snprintf(path,sizeof(path),"\r\nDNS gethostbyname fail\r\n");
                        PrintfResp(path);
                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;

                    }

                    server.sin_family = SC_AF_INET;
                    server.sin_port = sAPI_TcpipHtons(443);
                    server.sin_addr.s_addr= *(UINT32 *)host_entry->h_addr_list[0];

                    sAPI_Debug("start connect!!!");
                    ret = sAPI_TcpipConnect(sockfd,(SCsockAddr *)&server,sizeof(SCsockAddr));
                    if(ret != 0)
                    {
                        sAPI_SslClose(0);
                        sAPI_TcpipClose(sockfd);
                        sAPI_Debug("connect server fail");
                        snprintf(path,sizeof(path),"\r\nconnect server fail\r\n");
                        PrintfResp(path);
                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;
                    }
                    memset(&ctx,0,sizeof(ctx));
                    ctx.fd = sockfd;
                    ctx.ssl_version = SC_SSL_CFG_VERSION_ALL;


                    ret = sAPI_SslHandShake(&ctx);
                    sAPI_Debug("sAPI_SslHandShake ret[%d]",ret);

                    if(ret == 0)
                    {
                        ret = sAPI_SslSend(0,sendbuf,sizeof(sendbuf));
                        sAPI_Debug("ret [%d] sendbuf[%s]",ret,sendbuf);
                        memset(recvbuf,0x0,1024);
                        ret = sAPI_SslRead(0,recvbuf,1024);
                        sAPI_Debug("ret [%d] recvbuf[%s]",ret,recvbuf);
                        sAPI_Debug("sApi_SslConnect [%d]",ret);
                        sAPI_SslClose(0);
                        sAPI_TcpipClose(sockfd);
                        snprintf(path,sizeof(path),"\r\nret [%d] recvbuf[%s]\r\n",(int)ret,(char *)recvbuf);
                        PrintfResp(path);

                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;

                    }
                    else
                    {
                        sAPI_SslClose(0);
                        sAPI_TcpipClose(sockfd);
                        snprintf(path,sizeof(path),"\r\nhandshake fail,ret:%d\r\n",(int)ret);
                        PrintfResp(path);
                        if (-1 == sAPI_TcpipPdpDeactive(1,1))
                        {
                            sAPI_Debug("PDP deactive err");
                            snprintf(path,sizeof(path),"\r\nPDP deactive err\r\n");
                            PrintfResp(path);
                        }
                        break;
                    }
                }

            case SC_SSL_TEST:
                {
                    sAPI_Debug("Start SSL demo test thread!");
                    int sslTestTime = 0;
                    PrintfResp("\r\nPlease input test time.\r\n");
                    sslTestTime = UartReadValue();
                    SslTestDemoInit(sslTestTime);
                    break;
                }

            case SC_SSL_TWO_WAY_AUTH:
                {
                    sAPI_Debug("Start SSL demo two-way auth thread!");
                    ssl_verify_demo();
                    break;
                }
            case SC_SSL_DEMO_MAX:
                {
                    return;
                }

            default :
                 break;
        }

    }
}

static int ssl_pdp_active(void)
{
    const int counter = 10;

    for (int i = 0; i < counter; i++)
    {
        if (sAPI_TcpipPdpActive(1, 1) != 0)
        {
            sAPI_Debug("pdp active fail. waiting 3s.\r\n");
            sAPI_TaskSleep(3);
            continue;
        }

        sAPI_Debug("pdp active success.\r\n");
        return 0;
    }

    return -1;
}

static int ssl_connect_to_server(const char *domain, const char *port)
{
    if (domain == NULL)
    {
        sAPI_Debug("The paramter is invalid. Please check input parameter.\r\n");
        return -1;
    }

    int fd = -1;
    struct addrinfo hints, *addr_list;

    memset( &hints, 0, sizeof( hints ) );
    hints.ai_family = SC_AF_INET;
    hints.ai_socktype = SC_SOCK_STREAM;
    hints.ai_protocol = SC_IPPROTO_TCP;

    if (getaddrinfo(domain, port, &hints, &addr_list ) != 0 )
    {
        sAPI_Debug("Dns parse domain fail. Please check domain:%s.\r\n", domain);
        return -1;
    }

    int flag = 0;
    for (struct addrinfo *cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            sAPI_Debug("create socket fail.\r\n");
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            flag = 1;
            sAPI_Debug("connect to server success.\r\n");
            break;
        }

        close(fd);
    }

    freeaddrinfo(addr_list);

    return flag ? fd : -1;
}

static void ssl_verify(void *ptr)
{
    const char *domain = "mouwy.top";
    const char *port = "6066";
    const char *request = "GET /http_share/4k.txt HTTP/1.1\r\nHost: mouwy.top:6066\r\nAccept: */*\r\nCache-Control: no-cache\r\n\r\n";
    const char *ca_cert =
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIEqjCCA5KgAwIBAgIQAnmsRYvBskWr+YBTzSybsTANBgkqhkiG9w0BAQsFADBh\r\n"
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
        "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
        "QTAeFw0xNzExMjcxMjQ2MTBaFw0yNzExMjcxMjQ2MTBaMG4xCzAJBgNVBAYTAlVT\r\n"
        "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
        "b20xLTArBgNVBAMTJEVuY3J5cHRpb24gRXZlcnl3aGVyZSBEViBUTFMgQ0EgLSBH\r\n"
        "MTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALPeP6wkab41dyQh6mKc\r\n"
        "oHqt3jRIxW5MDvf9QyiOR7VfFwK656es0UFiIb74N9pRntzF1UgYzDGu3ppZVMdo\r\n"
        "lbxhm6dWS9OK/lFehKNT0OYI9aqk6F+U7cA6jxSC+iDBPXwdF4rs3KRyp3aQn6pj\r\n"
        "pp1yr7IB6Y4zv72Ee/PlZ/6rK6InC6WpK0nPVOYR7n9iDuPe1E4IxUMBH/T33+3h\r\n"
        "yuH3dvfgiWUOUkjdpMbyxX+XNle5uEIiyBsi4IvbcTCh8ruifCIi5mDXkZrnMT8n\r\n"
        "wfYCV6v6kDdXkbgGRLKsR4pucbJtbKqIkUGxuZI2t7pfewKRc5nWecvDBZf3+p1M\r\n"
        "pA8CAwEAAaOCAU8wggFLMB0GA1UdDgQWBBRVdE+yck/1YLpQ0dfmUVyaAYca1zAf\r\n"
        "BgNVHSMEGDAWgBQD3lA1VtFMu2bwo+IbG8OXsj3RVTAOBgNVHQ8BAf8EBAMCAYYw\r\n"
        "HQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8C\r\n"
        "AQAwNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdp\r\n"
        "Y2VydC5jb20wQgYDVR0fBDswOTA3oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQu\r\n"
        "Y29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNybDBMBgNVHSAERTBDMDcGCWCGSAGG\r\n"
        "/WwBAjAqMCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BT\r\n"
        "MAgGBmeBDAECATANBgkqhkiG9w0BAQsFAAOCAQEAK3Gp6/aGq7aBZsxf/oQ+TD/B\r\n"
        "SwW3AU4ETK+GQf2kFzYZkby5SFrHdPomunx2HBzViUchGoofGgg7gHW0W3MlQAXW\r\n"
        "M0r5LUvStcr82QDWYNPaUy4taCQmyaJ+VB+6wxHstSigOlSNF2a6vg4rgexixeiV\r\n"
        "4YSB03Yqp2t3TeZHM9ESfkus74nQyW7pRGezj+TC44xCagCQQOzzNmzEAP2SnCrJ\r\n"
        "sNE2DpRVMnL8J6xBRdjmOsC3N6cQuKuRXbzByVBjCqAA8t1L0I+9wXJerLPyErjy\r\n"
        "rMKWaBFLmfK/AHNF4ZihwPGOc7w6UHczBZXH5RFzJNnww+WnKuTPI0HfnVH8lg==\r\n"
        "-----END CERTIFICATE-----\r\n";

    const char *client_cert =
        "-----BEGIN CERTIFICATE-----\r\n"
        "MIIGdTCCBV2gAwIBAgIQD0q30+38OXvqGKvJmWHirTANBgkqhkiG9w0BAQsFADBu\r\n"
        "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
        "d3cuZGlnaWNlcnQuY29tMS0wKwYDVQQDEyRFbmNyeXB0aW9uIEV2ZXJ5d2hlcmUg\r\n"
        "RFYgVExTIENBIC0gRzEwHhcNMjMwMTMwMDAwMDAwWhcNMjQwMTMwMjM1OTU5WjAU\r\n"
        "MRIwEAYDVQQDEwltb3V3eS50b3AwggGiMA0GCSqGSIb3DQEBAQUAA4IBjwAwggGK\r\n"
        "AoIBgQCW11JgavWVD9sCdGFR9ZnvIHXqnbqlChea7rCLgY7Ra5GCIh5Jm4CeXz1C\r\n"
        "sHcmgqMMKuIjaghejohD3owOL8Snxu0qXGRgSrQIOUTlVr2FNUKQabzbdiY2+ww7\r\n"
        "ZIhCz7s4IsShUKde8Kt43OnSbDlzzLT4RSTLy080ybMhlD0cLEl5NJ0y7Ir3K84/\r\n"
        "+beJzSiFFneAXjqtipbQcetRyTZRyaB3FzGdJM7rJSt1wL/OoE4BUDOb9e6BIvh7\r\n"
        "PW6BK9reB2v5MKrWGgKaWLGd952QvCFtR2XRt+nJZe35+WcFbLki/tL44a0a9okk\r\n"
        "0J6YOueKuhNLmrP1rZAJa00HvyWYBPZ3D0IJTz09FkNeZ8qO3BMkpfgXTQY4bqS5\r\n"
        "dtrxeqVQxBJCGr0/Oz/vlEZOTIjV6GYuf5Svz2hyFnvBcLzYJ8Ua2mvnCDWL2+pB\r\n"
        "AwCQKYkRUtBhZUbbGrqSMuCdb7yDft3xm/Awt+LyH/mbyIVoIMBtEC63jd5NuCje\r\n"
        "NEQf/S0CAwEAAaOCAucwggLjMB8GA1UdIwQYMBaAFFV0T7JyT/VgulDR1+ZRXJoB\r\n"
        "hxrXMB0GA1UdDgQWBBT7BcWiti5Ij45oBMKY/1mwdu9dKzAjBgNVHREEHDAagglt\r\n"
        "b3V3eS50b3CCDXd3dy5tb3V3eS50b3AwDgYDVR0PAQH/BAQDAgWgMB0GA1UdJQQW\r\n"
        "MBQGCCsGAQUFBwMBBggrBgEFBQcDAjA+BgNVHSAENzA1MDMGBmeBDAECATApMCcG\r\n"
        "CCsGAQUFBwIBFhtodHRwOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwgYAGCCsGAQUF\r\n"
        "BwEBBHQwcjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQuY29tMEoG\r\n"
        "CCsGAQUFBzAChj5odHRwOi8vY2FjZXJ0cy5kaWdpY2VydC5jb20vRW5jcnlwdGlv\r\n"
        "bkV2ZXJ5d2hlcmVEVlRMU0NBLUcxLmNydDAJBgNVHRMEAjAAMIIBfQYKKwYBBAHW\r\n"
        "eQIEAgSCAW0EggFpAWcAdgDuzdBk1dsazsVct520zROiModGfLzs3sNRSFlGcR+1\r\n"
        "mwAAAYYAwsonAAAEAwBHMEUCIDfJJMuss6w9MI0Xvu/6pSSQSYErwO68gL48tZ6a\r\n"
        "5e6GAiEAkXgnLqbQgNZyC663cxbwvBuBXfNB/ZkA6dAWp1NmDlcAdQBz2Z6JG0yW\r\n"
        "eKAgfUed5rLGHNBRXnEZKoxrgBB6wXdytQAAAYYAwsp7AAAEAwBGMEQCIEfkiRT2\r\n"
        "FBSNBbisSz/S7T48W5taafOOPIi9WxCPopbkAiAYHlWrZFq64ses0YZQEUYLZZO7\r\n"
        "bNFACgTuTu6JN8WwGAB2AEiw42vapkc0D+VqAvqdMOscUgHLVt0sgdm7v6s52IRz\r\n"
        "AAABhgDCyk0AAAQDAEcwRQIgae9nPeTexyLABuDaPva1zYrLae1MK3pcXaCMkiV4\r\n"
        "sdECIQDVd8bxzIyUr6Lcrg1EzQwiCEMXdO+Gm9f8QqQlQerRjDANBgkqhkiG9w0B\r\n"
        "AQsFAAOCAQEAAJ4gaUbeFGbQt80w/qAnSB7o50b+CsRolm1Xo7yf3Li7pmZfe6T3\r\n"
        "v0kaKewwXRNRc5inWY+qYwksv5QyFKpWlbubRNlGj5BFGMZn2W3Vb4PPT/IAIrcd\r\n"
        "4hNz/gMloCNB2R6wv31BuecUyEdR3aU5DWFyHiN22IMTlTzJEVbrRMRK9tT850lm\r\n"
        "oy8OYLd0982RJr0cRTTxs9FCD7A/eoGhZm48qSF/NCmJowKVDR80D8TXQyjzZ8Fn\r\n"
        "Mal93gsLRSqkAHnzPb93Qbq5oPy1KV8Uh+6ShLJ8h+Q39qJ/g/poqKuKRi4xhCEW\r\n"
        "33PJ3goxuhHb4noG5mmTcyrICd3StY/L7Q==\r\n"
        "-----END CERTIFICATE-----\r\n";

    const char *client_key =
        "-----BEGIN RSA PRIVATE KEY-----\r\n"
        "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\r\n"
        "-----END RSA PRIVATE KEY-----\r\n";

    sAPI_Debug("Start ssl connect.\r\n");

    if (ssl_pdp_active() != 0)
    {
        sAPI_Debug("The network may be unavailable. Please check related equipment.\r\n");
        return;
    }

    int fd = ssl_connect_to_server(domain, port);
    if (fd < 0)
    {
        // sAPI_Debug("connect to server:%s fail. error:%d.\r\n", domain, sAPI_TcpipGetErrno());
        return;
    }

    SCSslCtx_t sslCtx;

    memset(&sslCtx, 0, sizeof(SCSslCtx_t));
    sslCtx.ClientId = 0;
    sslCtx.fd = fd;
    sslCtx.ciphersuitesetflg = 0;
    sslCtx.ssl_version = SC_SSL_CFG_VERSION_TLS12;
    // sslCtx.enable_SNI = 0;
    sslCtx.enable_SNI = 1;
    sslCtx.ipstr = (INT8 *)"mouwy.top";
    sslCtx.root_ca = (INT8 *)ca_cert;
    sslCtx.root_ca_len = strlen(ca_cert) + 1;
    sslCtx.ignore_local_time = 1;
    // sslCtx.auth_mode = SC_SSL_CFG_VERIFY_MODE_REQUIRED;
    // sslCtx.auth_mode = SC_SSL_CFG_VERIFY_MODE_CLIENT;
    sslCtx.auth_mode = SC_SSL_CFG_VERIFY_MODE_SERVER_CLIENT;
    sslCtx.client_cert = (INT8 *)client_cert;
    sslCtx.client_cert_len = strlen(client_cert) + 1;
    sslCtx.client_key = (INT8 *)client_key;
    sslCtx.client_key_len = strlen(client_key) + 1;

    if (sAPI_SslHandShake(&sslCtx) != 0)
    {
        sAPI_Debug("ssl handshake fail.\r\n");
        return;
    }

    sAPI_Debug("Ssl Handshake success.\r\n");

    int status = 0;
    int send_len = 0;
    int total_len = strlen(request);

    while (send_len < total_len)
    {
        int bytes = sAPI_SslSend(0, (INT8 *)request + send_len, total_len - send_len);
        if (bytes < 0)
        {
            status = 1;
            sAPI_Debug("send ssl data fail.\r\n");
            break;
        }
        else if (bytes == 0)
        {
            status = 2;
            sAPI_Debug("the remote server is closed.\r\n");
            break;
        }

        send_len += bytes;
    }

    if (status != 0)
    {
        goto close_ssl;
    }

    while (1)
    {
        char buffer[512];
        int bytes;

        bytes = sAPI_SslRead(0, (INT8 *)buffer, sizeof(buffer) - 1);
        if (bytes < 0)
        {
            sAPI_Debug("read ssl data fail.\r\n");
            break;
        }
        else if (bytes == 0)
        {
            sAPI_Debug("the remote server is closed.\r\n");
            break;
        }

        buffer[bytes] = 0;

        sAPI_Debug("read bytes:%d, read data:%s.\r\n", bytes, buffer);
    }

close_ssl:
    sAPI_SslClose(0);
    close(fd);
    sAPI_TcpipPdpDeactive(1, 1);

    sAPI_Debug("End ssl connect.\r\n");
}

static void ssl_verify_demo(void)
{
    sTaskRef sslTask;

    if (0 != sAPI_TaskCreate(&sslTask, NULL, 8192, 120, (char *)"ssl verify task", ssl_verify, NULL))
    {
        sAPI_Debug("ssl verify task create fail.\r\n");
        return;
    }

    sAPI_Debug("ssl verify task create success.\r\n");
}





















#if 0

#include "simcom_api.h"


#define FALSE 0
#define TRUE 1
sTaskRef sslProcesser;
static UINT32 sslProcesserStack[3072] = {0xA5A5A5A5};

void sTask_SslProcesser(void * arg)
{
#if 0
    sAPI_TaskSleep(200*50);

    int result = 0;
    SCsslContent content = {0};
    result = sAPI_SslSetContextIdMsg("sslversion",0,"1");
    sAPI_Debug("result = [%d]",result);

    result = sAPI_SslSetContextIdMsg("cacert",0,"test.pem");
    sAPI_Debug("result = [%d]",result);

    content = sAPI_SslGetContextIdMsg(0);
    sAPI_Debug("version = [%d]",content.ssl_version);
#else
        sAPI_TaskSleep(200*6);//20s

        sAPI_Debug("ssl Test START");
        demo_ssl_test();
        sAPI_Debug("ssl Test END");

#endif
}



void sAPP_SslTaskDemo()
{

    BOOL is_inited = FALSE;
    SC_STATUS status;
    sAPI_Debug("is_inited = [%d]",is_inited);

    if(TRUE == is_inited)
    return;

    status = sAPI_TaskCreate(&sslProcesser,sslProcesserStack,8000,152,"scssl",sTask_SslProcesser,(void *)0);
    if(SC_SUCCESS != status)
    {
        is_inited = FALSE;
        sAPI_Debug("task create fail");
    }

    if(SC_SUCCESS == status)
    is_inited = TRUE;


}


void demo_ssl_test(void)
{
    INT32 ret = 0;
    SCSslCtx_t ctx;
    INT32 sockfd = -1;
    SChostent *host_entry = NULL;
    SCsockAddrIn sa;
    SCsockAddrIn server;
    INT8 recvbuf[1024];
    INT8 sendbuf[] = { /* Packet 8 */
    0x47, 0x45, 0x54, 0x20, 0x2f, 0x20, 0x48, 0x54,
    0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a,
    0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, 0x77, 0x77,
    0x77, 0x2e, 0x62, 0x61, 0x69, 0x64, 0x75, 0x2e,
    0x63, 0x6f, 0x6d, 0x0d, 0x0a, 0x43, 0x61, 0x63,
    0x68, 0x65, 0x2d, 0x43, 0x6f, 0x6e, 0x74, 0x72,
    0x6f, 0x6c, 0x3a, 0x20, 0x6e, 0x6f, 0x2d, 0x63,
    0x61, 0x63, 0x68, 0x65, 0x0d, 0x0a, 0x43, 0x6f,
    0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79,
    0x70, 0x65, 0x3a, 0x20, 0x74, 0x65, 0x78, 0x74,
    0x2f, 0x70, 0x6c, 0x61, 0x69, 0x6e, 0x0d, 0x0a,
    0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20,
    0x2a, 0x2f, 0x2a, 0x0d, 0x0a, 0x0d, 0x0a, 0x0d,
    0x0a };

    UINT8 pGreg = 0;
    while(1)
    {
        sAPI_NetworkGetCgreg(&pGreg);
        if(1 != pGreg)
        {
            sAPI_Debug("SSL NETWORK STATUS IS [%d]",pGreg);
            sAPI_TaskSleep(10*300);
        }
        else
        {
            sAPI_Debug("SSL NETWORK STATUS IS NORMAL");
            break;
        }
    }

     if (-1 == sAPI_TcpipPdpActive(1,1))
    {
      sAPI_Debug("PDP active err");
      return;
    }


    sockfd = sAPI_TcpipSocket(SC_AF_INET, SC_SOCK_STREAM, 0);

    sAPI_Debug("demo_ssl_test sockfd[%d]",sockfd);
    if(sockfd < 0)
    {
        sAPI_Debug("create socket err");
        return;
    }

    host_entry = sAPI_TcpipGethostbyname("www.baidu.com");
    if (host_entry == NULL)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("DNS gethostbyname fail");
        return;

    }

    server.sin_family = SC_AF_INET;
    server.sin_port = sAPI_TcpipHtons(443);
    server.sin_addr.s_addr= *(UINT32 *)host_entry->h_addr_list[0];

    sAPI_Debug("start connect!!!");
    ret = sAPI_TcpipConnect(sockfd,&server,sizeof(SCsockAddr));
    if(ret != 0)
    {
        sAPI_SslClose(0);
        sAPI_TcpipClose(sockfd);
        sAPI_Debug("connect server fail errno");
        return;
    }
    memset(&ctx,0,sizeof(ctx));
    ctx.fd = sockfd;
    ctx.ssl_version = SC_SSL_CFG_VERSION_ALL;


    ret = sAPI_SslHandShake(&ctx);
    sAPI_Debug("sAPI_SslHandShake ret[%d]",ret);

    if(ret == 0)
    {
        ret = sAPI_SslSend(0,sendbuf,sizeof(sendbuf));
        sAPI_Debug("ret [%d] sendbuf[%s]",ret,sendbuf);
        memset(recvbuf,0x0,1024);
        ret = sAPI_SslRead(0,recvbuf,1024);
        sAPI_Debug("ret [%d] recvbuf[%s]",ret,recvbuf);

    }
    sAPI_Debug("sApi_SslConnect [%d]",ret);

    sAPI_SslClose(0);
    sAPI_TcpipClose(sockfd);

    if (-1 == sAPI_TcpipPdpDeactive(1,1))
    {
         sAPI_Debug("PDP deactive err");
         return;
    }

}


#endif

