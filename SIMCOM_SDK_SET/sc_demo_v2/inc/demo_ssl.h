#ifndef __DEMO_SSL_H__
#define __DEMO_SSL_H__

#include <stdbool.h>

#define ARGSSL_CLIENT_ID       "client_id"
#define ARGSSL_SNI             "enable_SNI"
#define ARGSSL_IGNORELOCALTIME "ignorelocaltime"
#define ARGSSL_SSL_VERSION     "ssl_version"
#define ARGSSL_AUTH_MODE       "auth_mode"
#define ARGSSL_NEGOTIATE_TIME  "negotiatetime_s"
#define ARGSSL_ROOT_CERT       "root_cert file path"
#define ARGSSL_CLIENT_CERT     "client_cert file path"
#define ARGSSL_CLIENT_KEY      "client_key file path"

#define SSL_CHANNEL_MAX 2   // only support 2 links to execute the ssl handshake directly, for TCP only.
#define SSL_CONFIG_MAX 10   // we use ssl_client_id to config SSL to kernel, it is used by other services such as HTTPS.

/**
 * @brief About enter the certificate file name.
 * First, you need to use the file system to upload the certificate file to the root directory of drive C.
 * 
 * TCP_link are handshake by tcp using the sAPI_SslHandShake interface.
 * In this case, we need to use a certificate file, and the demo uses the method of reading integers from the file system.
 * The certificate file must be stored in the file system.
 * According to authmode, you only need to configure the required certificates. A certificate that is not required is ignored
 * e.g.: C:/baidu.pem
 * 
 * SSL Link 0 to link 9 Use the sAPI_SslSetContextIdMsg interface to configure ssl to the kernel.
 * In this case, We need to pass in the certificate file name and ensure that the certificate already exists in the root directory of drive C.
 * All Settings are configured to the kernel, and if some certificates are not configured, set to "NULL"(Case insensitive). Otherwise, it is regarded as the certificate file name
 * e.g.: baidu.pem
 */

enum
{
    SSL_3_0,
    TLS_1_0,
    TLS_1_1,
    TLS_1_2,
    SSL_ALL  // default
};

enum
{
    SSL_AUTH_NONE,
    SSL_AUTH_SERVER,
    SSL_AUTH_SERVER_CLIENT,
    SSL_AUTH_CLIENT
};

enum demo_ssl_mode
{
    demo_ssl_default = 0,       // default mode
    demo_ssl_handshake_mode,    //When TCP uses SSL encryption, we complete the handshake process.
    demo_ssl_cfg_mode,          //When other data services use SSL encryption, we configure parameters to the kernel.
};

#endif /* __DEMO_SSL_H__ */
