#include "contiki.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include "lib/queue.h"
#include "lib/assert.h"
#include "serialport.h"

#include "sys/log.h"
#define LOG_MODULE "SerialPort"
#define LOG_LEVEL LOG_LEVEL_INFO

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

/*---------------------------------------------------------------------------*/
typedef struct serial_tx_memb {
  struct serial_tx_memb *next;
  uint8_t *data;
  int len;
  int remain;
} serial_tx_memb_t;
/*---------------------------------------------------------------------------*/
static uint8_t rx_buf[SERIAL_RX_BUF_SIZE];
static serial_port_rx_cb_t rx_cb;
static int serial_fd = -1;
static int fd_error;
QUEUE(tx_queue);
/*---------------------------------------------------------------------------*/
static int
set_fd(fd_set *rset, fd_set *wset)
{
  if(serial_fd > 0 && !fd_error) {
    FD_SET(serial_fd, rset);
    if(!queue_is_empty(tx_queue)) {
      FD_SET(serial_fd, wset);
    }
    return 1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
static void
serial_do_tx(void)
{
  serial_tx_memb_t *p;
  int cnt, offset;

  p = queue_peek(tx_queue);
  if(p) {
    offset = p->len - p->remain;
    cnt = write(serial_fd, p->data + offset, p->remain);
    if(cnt > 0) {
      p->remain -= cnt;
      if(p->remain == 0) {
        queue_dequeue(tx_queue);
        free(p->data);
        free(p);
      }
    } else if(cnt < 0) {
      LOG_ERR("write error: error code %d\n", errno);
      fd_error = errno;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
handle_fd(fd_set *rset, fd_set *wset)
{
  int cnt;
  if(serial_fd >= 0 && !fd_error) {
    if(FD_ISSET(serial_fd, wset)) {
      serial_do_tx();
      if(queue_is_empty(tx_queue)) {
        FD_CLR(serial_fd, wset);
      }
    }

    if(FD_ISSET(serial_fd, rset)) {
      cnt = read(serial_fd, rx_buf, sizeof(rx_buf));
      if((cnt > 0) && rx_cb) {
        rx_cb(rx_buf, cnt);
      } else if(cnt < 0) {
        LOG_ERR("read error: error code %d\n", errno);
        fd_error = errno;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static const struct select_callback serial_select_callback = {
  set_fd,
  handle_fd,
};
/*---------------------------------------------------------------------------*/
static int
to_baud_const(int baud)
{
  switch(baud) {
  case 0: return B0;
  case 50: return B50;
  case 75: return B75;
  case 110: return B110;
  case 134: return B134;
  case 150: return B150;
  case 200: return B200;
  case 300: return B300;
  case 600: return B600;
  case 1200: return B1200;
  case 1800: return B1800;
  case 2400: return B2400;
  case 4800: return B4800;
  case 9600: return B9600;
  case 19200: return B19200;
  case 38400: return B38400;
  case 57600: return B57600;
  case 115200: return B115200;
  case 230400: return B230400;
#if defined(__linux__)
  case 460800: return B460800;
  case 500000: return B500000;
  case 576000: return B576000;
  case 921600: return B921600;
  case 1000000: return B1000000;
  case 1152000: return B1152000;
  case 1500000: return B1500000;
  case 2000000: return B2000000;
  case 2500000: return B2500000;
  case 3000000: return B3000000;
  case 3500000: return B3500000;
  case 4000000: return B4000000;
#endif
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
to_data_bits_const(int data_bits)
{
  switch(data_bits) {
  case 8: default: return CS8;
  case 7: return CS7;
  case 6: return CS6;
  case 5: return CS5;
  }
  return -1;
}
/*---------------------------------------------------------------------------*/
static int
set_baudrate(int fd, serial_port_connect_opt_t *opt)
{
  /* lookup the standard baudrates from the table */
  int baud_rate = to_baud_const(opt->baud);

  /* get port options */
  struct termios options;

  if(-1 == tcgetattr(fd, &options)) {
    LOG_ERR("Error: %s setting custom baud rate of %d", strerror(errno), opt->baud);
    return -1;
  }

  if(-1 == baud_rate) {
    LOG_ERR("Error baud rate of %d is not supported on your platform", opt->baud);
    return -1;
  }

  /* If we have a good baud rate set it and lets go */
  cfsetospeed(&options, baud_rate);
  cfsetispeed(&options, baud_rate);
  /* throw away all the buffered data */
  tcflush(fd, TCIOFLUSH);
  /* make the changes now */
  tcsetattr(fd, TCSANOW, &options);

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
setup(int fd, serial_port_connect_opt_t *opt)
{
  int data_bits = to_data_bits_const(opt->data_bits);
  if(data_bits == -1) {
    LOG_ERR("Invalid data bits %d\n", opt->data_bits);
    return -1;
  }
  if(fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
    LOG_ERR("Set FD_CLOEXEC failed\n");
    return -1;
  }

  /* Get port configuration for modification */
  struct termios options;
  tcgetattr(fd, &options);

  /* IGNPAR: ignore bytes with parity errors */
  options.c_iflag = IGNPAR;
  /* ICRNL: map CR to NL (otherwise a CR input on the other computer will not terminate input) */
  /* Future potential option */
  /* options.c_iflag = ICRNL; */
  /* otherwise make device raw (no other input processing) */

  /* Specify data bits */
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= data_bits;

  options.c_cflag &= ~(CRTSCTS);

  if(opt->rtscts) {
    options.c_cflag |= CRTSCTS;
  }

  options.c_iflag &= ~(IXON | IXOFF | IXANY);

  if(opt->xon) {
    options.c_iflag |= IXON;
  }

  if(opt->xoff) {
    options.c_iflag |= IXOFF;
  }

  if(opt->xany) {
    options.c_iflag |= IXANY;
  }

  switch(opt->parity) {
  case SERIALPORT_PARITY_NONE:
    options.c_cflag &= ~PARENB;
    break;
  case SERIALPORT_PARITY_ODD:
    options.c_cflag |= PARENB;
    options.c_cflag |= PARODD;
    break;
  case SERIALPORT_PARITY_EVEN:
    options.c_cflag |= PARENB;
    options.c_cflag &= ~PARODD;
    break;
  default:
    LOG_ERR("Invalid parity setting %d", opt->parity);
    return -1;
  }

  switch(opt->stop_bits) {
  case SERIALPORT_STOPBITS_ONE:
    options.c_cflag &= ~CSTOPB;
    break;
  case SERIALPORT_STOPBITS_TWO:
    options.c_cflag |= CSTOPB;
    break;
  default:
    LOG_ERR("Invalid stop bits setting %d", opt->stop_bits);
    return -1;
  }

  options.c_cflag |= CLOCAL;      /* ignore status lines */
  options.c_cflag |= CREAD;       /* enable receiver */
  options.c_oflag = 0;
  /* ICANON makes partial lines not readable. It should be optional. */
  /* It works with ICRNL. */
  options.c_lflag = 0; /* ICANON; */
  options.c_cc[VMIN] = 0;
  options.c_cc[VTIME] = 0;

  tcsetattr(fd, TCSANOW, &options);

  if(set_baudrate(fd, opt) == -1) {
    return -1;
  }

  return 1;
}
/*---------------------------------------------------------------------------*/
int
serial_port_open(char *dev, serial_port_connect_opt_t *opt)
{
  assert(serial_fd == -1);   /* previously port has not been closed */
  serial_fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC | O_SYNC);
  if(serial_fd < 0) {
    return -1;
  }
  if(setup(serial_fd, opt) < 0) {
    return -1;
  }
  rx_cb = NULL;
  fd_error = 0;
  select_set_callback(serial_fd, &serial_select_callback);
  queue_init(tx_queue);

  return 0;
}
void
serial_port_close(void)
{
  serial_tx_memb_t *p;

  if(serial_fd >= 0) {
    select_set_callback(serial_fd, NULL);
    rx_cb = NULL;
    close(serial_fd);
    serial_fd = -1;
    while(1) {
      p = queue_dequeue(tx_queue);
      if(!p) {
        break;
      }
      free(p->data);
      free(p);
    }
  }
}
/*---------------------------------------------------------------------------*/
int
serial_port_write(uint8_t *buf, size_t len)
{
  serial_tx_memb_t *p;

  if(serial_fd < 0 || fd_error) {
    free(buf);
    return len;
  }

  p = malloc(sizeof(serial_tx_memb_t));
  if(!p) {
    LOG_ERR("out of memory\n");
    return -1;
  }

  p->data = buf;
  p->len = p->remain = len;
  queue_enqueue(tx_queue, p);

  return len;
}
/*---------------------------------------------------------------------------*/
void
serial_port_register_rx_cb(serial_port_rx_cb_t cb)
{
  rx_cb = cb;
}
/*---------------------------------------------------------------------------*/
