#ifndef UDP_TRANSPORT_H
#define UDP_TRANSPORT_H

#include "contiki.h"
#include <stdint.h>
#include <stddef.h>

typedef void (*udp_transport_rx_cb_t)(const uint8_t *data, size_t len);

void udp_transport_register_cmd_callback(udp_transport_rx_cb_t cb);
void udp_transport_register_data_callback(udp_transport_rx_cb_t cb);
void udp_transport_write_aboot_cmd(const uint8_t *data, size_t len);
void udp_transport_set_aboot_data_size(size_t size);
void udp_transport_write_aboot_data(const uint8_t *data, size_t len);
void udp_transport_init(void);
void udp_transport_exit(void);

#endif /* UDP_TRANSPORT_H */
