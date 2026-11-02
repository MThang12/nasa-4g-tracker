#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <stdint.h>

#ifndef SERIAL_RX_BUF_SIZE
#define SERIAL_RX_BUF_SIZE (1024)
#endif

typedef enum {
  SERIALPORT_PARITY_NONE  = 1,
  SERIALPORT_PARITY_MARK  = 2,
  SERIALPORT_PARITY_EVEN  = 3,
  SERIALPORT_PARITY_ODD   = 4,
  SERIALPORT_PARITY_SPACE = 5
} serialport_parity_t;

typedef enum {
  SERIALPORT_STOPBITS_ONE         = 1,
  SERIALPORT_STOPBITS_ONE_FIVE    = 2,
  SERIALPORT_STOPBITS_TWO         = 3
} serialport_stop_bits_t;

typedef void (*serial_port_rx_cb_t)(uint8_t *data, size_t len);

typedef struct {
  int baud;
  int data_bits;
  bool rtscts;
  bool xon;
  bool xoff;
  bool xany;
  bool dsrdtr;
  bool hupcl;
  serialport_parity_t parity;
  serialport_stop_bits_t stop_bits;
} serial_port_connect_opt_t;

int serial_port_open(char *dev, serial_port_connect_opt_t *opt);
void serial_port_close(void);
int serial_port_write(uint8_t *buf, size_t len);
void serial_port_register_rx_cb(serial_port_rx_cb_t cb);

#endif /* SERIALPORT_H */
