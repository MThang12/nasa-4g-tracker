#ifndef SMUX_H
#define SMUX_H

#include "contiki.h"
#include <stdint.h>
#include <stddef.h>

#ifndef SMUX_MEMB_SIZE
#define SMUX_MEMB_SIZE (64 * 1024);
#endif /* SMUX_MEMB_SIZE */

#ifndef SMUX_FRAME_MTU
#define SMUX_FRAME_MTU 1024
#endif /* SMUX_FRAME_MTU */

/**
 * SMUX preamble type definition
 */
#define SMUX_PREAMBLE_SIZE  4       /* preamble char numbers */
#define SMUX_PREAMBLE_UABT  "UABT"  /* aboot tiny */

typedef void (*smux_rx_cb_t)(const uint8_t *data, size_t len);

void smux_register_cmd_callback(smux_rx_cb_t cb);
void smux_register_data_callback(smux_rx_cb_t cb);
void smux_write_aboot_cmd(const uint8_t *data, size_t len);
void smux_set_aboot_data_size(size_t size);
void smux_write_aboot_data(const uint8_t *data, size_t len);
void smux_init(void);
void smux_exit(void);

#endif /* SMUX_H */
