#ifndef __SAL_NETIF_H__
#define __SAL_NETIF_H__


void lwip_set_tcp_syn_max_rtx(int max_rtx);
void lwip_set_tcp_sync_backoff(unsigned char * backoff_cfg, int size_n);
void lwip_set_tcp_max_rtx(int max_rtx);
void lwip_set_tcp_backoff(unsigned char * backoff_cfg, int size_n);
void lwip_set_netif_pdp_mtu(unsigned short mtu, char cid);
int lwip_set_tcp_rcv_win(unsigned int value);
void lwip_set_dns_max_retries(unsigned int value);
void lwip_set_dns_wait_tmr(unsigned char * wait_cfg, int size_n);
int lwip_get_tcp_syn_max_rtx(void);
void lwip_get_tcp_sync_backoff(unsigned char * backoff_read, int size_n);
int lwip_get_tcp_max_rtx(void);
void lwip_get_tcp_backoff(unsigned char * backoff_read, int size_n);
unsigned short lwip_get_netif_pdp_mtu(void);
unsigned int lwip_get_tcp_rcv_win(void);
unsigned int lwip_get_dns_max_wait_time(void);
unsigned int lwip_get_dns_max_retries(void);
void lwip_get_dns_wait_tmr(unsigned char * wait_read, int size_n);



#endif /* __SAL_NETIF_H__ */
