#ifndef ETHOS_H
#define ETHOS_H

#include <stdint.h>
#include <stddef.h>

#include "contiki.h"
#include "net/ipv6/uip.h"

#ifndef ETHOS_MEMB_SIZE
#define ETHOS_MEMB_SIZE (64 * 1024);
#endif /* ETHOS_MEMB_SIZE */

#ifndef ETHOS_FRAME_MTU
#define ETHOS_FRAME_MTU 2048
#endif /* ETHOS_FRAME_MTU */

/**
 * ETHOS preamble type definition
 */
#define ETHOS_PREAMBLE_SIZE  4       /* preamble char numbers */
#define ETHOS_PREAMBLE_UUUU  "UUUU"  /* aboot standard */

#define ETHOS_MAC_ADDR_LEN   6

typedef void (*ethos_rx_cb_t)(const uint8_t *data, size_t len);

int ethos_get_mac(uip_eth_addr *lladdr);
void ethos_register_cmd_callback(ethos_rx_cb_t cb);
void ethos_register_data_callback(ethos_rx_cb_t cb);
void ethos_write_aboot_cmd(const uint8_t *data, size_t len);
void ethos_set_aboot_data_size(size_t size);
void ethos_write_aboot_data(const uint8_t *data, size_t len);
void ethos_init(void);
void ethos_exit(void);

#endif /* ETHOS_H */
