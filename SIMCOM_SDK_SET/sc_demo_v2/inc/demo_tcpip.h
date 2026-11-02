#ifndef __DEMO_TCPIP_H__
#define __DEMO_TCPIP_H__


#define ABGTCP_TCP_PDPTYPE "tcp_pdptype"
#define ARGTCP_SERVERIP "serverip"
#define ARGTCP_TCP_PORT "tcp_port"
#define ARGTCP_TCP_SENDSIZE "tcp_sendsize"
#define TCPSEND_DATA_CACHE_MAX 1400


#define ARGTCP_REMOTEIP "remoteip"
#define ARGTCP_UDP_PORT "udp_port"
#define ARGTCP_UDP_SENDSIZE "udp_sendsize"

#define ARGTCP_SERVER_LOCALPORT "tcpserver_localport"

#define ARGTCP_CONNECT_ID "connect_id"

typedef enum{
    NET_TYPE_TCPCLI,
    NET_TYPE_UDPCLI,
}TcpipNettype_E;

#ifdef HAS_DEMO_SSL
struct demo_ssl_cfg
{
    bool enable_sni;
    bool ignorelocaltime;
    unsigned char ssl_version;
    unsigned char auth_mode;
    unsigned short negotiate_time_s; // seconde

    char *root_cert_data;
    char *client_cert_data;
    char *client_key_data;

    char *root_cert_path;
    char *client_cert_path;
    char *client_key_path;

    unsigned char client_id;
    bool client_is_on_use;
};
enum demo_ssl_ret
{
    SSL_ERROR = -1,         // something is wrong
    SSL_OK,                 // everything is OK
    SSL_CFG_ID_ERROR,       // cfg_id is invalid
    SSL_IS_NOT_INIT,        // SSL is not initialized
    SSL_CLIENT_IS_ON_USE,   // SSL current client is on use
    SSL_HANDSHAKE_FAILED,   // SSL handshake failed
};
ret_msg_t demo_ssl_handshake_cfg(struct demo_ssl_cfg *ssl_cfg);
#endif /*_HAS_DEMO_SSL_*/
#endif /* __DEMO_TCPIP_H__ */
