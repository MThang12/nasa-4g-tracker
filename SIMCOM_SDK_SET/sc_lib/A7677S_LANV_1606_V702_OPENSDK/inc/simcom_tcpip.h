#ifndef _SIMCOM_TCPIP_H_
#define _SIMCOM_TCPIP_H_
#include "simcom_os.h"

#define SC_TCPIP_SUCCESS 0
#define SC_TCPIP_FAIL -1

enum SCnetType
{
	TCPIP_PDP_IPV4V6,
	TCPIP_PDP_IPV4,
	TCPIP_PDP_IPV6,
	TCPIP_PDP_INVALID
};

struct SCipInfo
{
	enum SCnetType type;
	UINT32 ip4;
	UINT32 ip6[4];
};
enum socket_state
{
    SOCKET_NONE,
    SOCKET_WRITE,
    SOCKET_LISTEN,
    SOCKET_CONNECT,
    SOCKET_CLOSE
};

INT32 sAPI_TcpipPdpActive(int cid, int channel);
INT32 sAPI_TcpipPdpDeactive(int cid, int channel);
INT32 sAPI_TcpipPdpDeactiveNotify(sMsgQRef msgQ);
INT32 sAPI_TcpipGetSocketPdpAddr(int cid, int channel, struct SCipInfo *info);
enum socket_state sAPI_GetSocketState(int fd);
#endif



