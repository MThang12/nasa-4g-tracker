#include "contiki.h"
#include <stdlib.h>
#include <string.h>
#include "lib/assert.h"
#include "ethos.h"
#include "serialport.h"
#include "aboot-callback.h"

#include "sys/log.h"
#define LOG_MODULE "Ethos"
#define LOG_LEVEL LOG_LEVEL_INFO

#define TIMER_PERIOD_100MS  (CLOCK_SECOND / 10)
#define LINK_CHECK_COUNT    (3 * 10)

/*---------------------------------------------------------------------------*/
typedef enum {
  WAIT_FRAMESTART,
  IN_FRAME,
  IN_ESCAPE
} line_state_t;

typedef enum {
  STAGE_INIT,
  STAGE_HELLO,
  STAGE_RUNNING,
  STAGE_FAILED,
  STAGE_SUCCEEDED
} running_stage_t;

typedef enum {
  ETHOS_FRAME_TYPE_DATA        = (0x00),
  ETHOS_FRAME_TYPE_TEXT        = (0x01),
  ETHOS_FRAME_TYPE_HELLO       = (0x02),
  ETHOS_FRAME_TYPE_HELLO_REPLY = (0x03),
  ETHOS_FRAME_TYPE_HEART_BEAT  = (0x04),
  ETHOS_FRAME_TYPE_UPLOAD_PROG = (0x05),
  ETHOS_ESC_CHAR               = (0x7D),
  ETHOS_FRAME_DELIMITER        = (0x7E)
} ethos_frame_type_t;

typedef struct {
  running_stage_t stage;
  uint8_t rxbuf[ETHOS_FRAME_MTU]; /**< rxbuffer for incoming data */
  ethos_rx_cb_t cmd_cb;
  ethos_rx_cb_t data_cb;
  line_state_t state;             /**< Line status variable */
  size_t framesize;               /**< size of currently incoming frame */
  ethos_frame_type_t frametype;   /**< type of currently incoming frame */
  uint8_t mac_addr[ETHOS_MAC_ADDR_LEN];            /**< this device's MAC address */
  uint8_t remote_mac_addr[ETHOS_MAC_ADDR_LEN];     /**< this device's MAC address */
  uint8_t header[4];              /**< tranport header */
  uint8_t *txbuf;
  size_t txbuf_size;
  size_t txbuf_used;
  size_t total_tx_size;
  int timeout_count;                /**< count for timeout */
} ethos_t;
/*---------------------------------------------------------------------------*/
static ethos_t *ethos_dev;
static void ethos_send_frame(const uint8_t *data, size_t len, ethos_frame_type_t frame_type);
PROCESS(ethos_connect_process, "ethos connect process");
/*---------------------------------------------------------------------------*/
static void
ethos_set_tx_size(size_t size)
{
  ethos_t *ethos = ethos_dev;

  assert(ethos);
  ethos->total_tx_size = size;
  ethos->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
static void
ethos_send_data_request(size_t size)
{
  ethos_t *ethos = ethos_dev;

  uint8_t *buffer = (uint8_t *)malloc(32);
  if(!buffer) {
    LOG_ERR("out of memory\n");
    aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
    return;
  }
  memcpy(buffer, ethos->header, 4);
  sprintf((char *)buffer + 4, "data-request:%08x", (uint32_t)size);
  ethos_send_frame(buffer, strlen((const char *)buffer + 4) + 4, ETHOS_FRAME_TYPE_TEXT);
}
/*---------------------------------------------------------------------------*/
void
ethos_set_aboot_data_size(size_t size)
{
  ethos_set_tx_size(size);
  ethos_send_data_request(size);
}
/*---------------------------------------------------------------------------*/
static void
send_txbuf(void)
{
  ethos_t *ethos = ethos_dev;

  if(serial_port_write(ethos->txbuf, ethos->txbuf_used) < 0) {
    free(ethos->txbuf);
  }
  ethos->txbuf = NULL;
  ethos->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
static void
alloc_txbuf(void)
{
  ethos_t *ethos = ethos_dev;

  ethos->txbuf = (uint8_t *)malloc(ethos->txbuf_size);
  assert(ethos->txbuf);
  ethos->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
static void
ethos_send_raw_data(const uint8_t *data, size_t size)
{
  ethos_t *ethos = ethos_dev;

  ethos->total_tx_size -= size;
  size_t used = ethos->txbuf_used;

  while(size > 0) {
    if(!ethos->txbuf) {
      alloc_txbuf();
      used = 0;
    }

    size_t len = ethos->txbuf_size - used;
    if(size <= len) {
      len = size;
    }
    memcpy(ethos->txbuf + used, data, len);
    used += len;
    size -= len;

    if(used == ethos->txbuf_size) {
      ethos->txbuf_used = used;
      send_txbuf();
      used = 0;
    }
    if(!size) {
      if(ethos->total_tx_size == 0 && used) {
        ethos->txbuf_used = used;
        send_txbuf();
        used = 0;
        ethos_send_data_request(0);
      }
    }
  }
  ethos->txbuf_used = used;
}
/*---------------------------------------------------------------------------*/
static uint8_t *
_write_escaped(uint8_t *p, const uint8_t *buf, size_t n)
{
  while(n--) {
    uint8_t c = *buf++;
    switch(c) {
    case ETHOS_FRAME_DELIMITER:
      *p++ = ETHOS_ESC_CHAR;
      *p++ = (ETHOS_FRAME_DELIMITER ^ 0x20);
      break;
    case ETHOS_ESC_CHAR:
      *p++ = ETHOS_ESC_CHAR;
      *p++ = (ETHOS_ESC_CHAR ^ 0x20);
      break;
    default:
      *p++ = c;
      break;
    }
  }
  return p;
}
/*---------------------------------------------------------------------------*/
static void
ethos_send_frame(const uint8_t *data, size_t len, ethos_frame_type_t frame_type)
{
  uint8_t *frame = (uint8_t *)malloc(ETHOS_FRAME_MTU);
  if(!frame) {
    LOG_ERR("out of memory\n");
    return;
  }

  uint8_t *p = frame;

  /* send frame delimiter */
  *p++ = ETHOS_FRAME_DELIMITER;

  /* set frame type */
  if(frame_type != ETHOS_FRAME_TYPE_DATA) {
    *p++ = ETHOS_ESC_CHAR;
    *p++ = (frame_type ^ 0x20);
  }

  /* send frame content */
  p = _write_escaped(p, data, len);

  /* end of frame */
  *p++ = ETHOS_FRAME_DELIMITER;

  len = p - frame;
  if(serial_port_write(frame, len) < 0) {
    free(frame);
  }
}
/*---------------------------------------------------------------------------*/
int
ethos_get_mac(uip_eth_addr *lladdr)
{
  ethos_t *ethos = ethos_dev;

  if(ethos && ethos->stage == STAGE_RUNNING) {
    memcpy(lladdr->addr, ethos->remote_mac_addr, ETHOS_MAC_ADDR_LEN);
    return 1;
  } else {
    return 0;
  }
}
/*---------------------------------------------------------------------------*/
void
ethos_write_aboot_cmd(const uint8_t *data, size_t len)
{
  ethos_send_frame(data, len, ETHOS_FRAME_TYPE_DATA);
}
/*---------------------------------------------------------------------------*/
void
ethos_write_aboot_data(const uint8_t *data, size_t len)
{
  ethos_t *ethos = ethos_dev;

  assert(len > 4);

  memcpy(ethos->header, data, 4);
  ethos_send_raw_data(data + 4, len - 4);
}
/*---------------------------------------------------------------------------*/
static void
_reset_state(ethos_t *ethos)
{
  ethos->state = WAIT_FRAMESTART;
  ethos->frametype = 0;
  ethos->framesize = 0;
}
/*---------------------------------------------------------------------------*/
static void
_handle_char(ethos_t *ethos, char c)
{
  switch(ethos->frametype) {
  case ETHOS_FRAME_TYPE_DATA:
  case ETHOS_FRAME_TYPE_TEXT:
  case ETHOS_FRAME_TYPE_HELLO:
  case ETHOS_FRAME_TYPE_HELLO_REPLY:
  case ETHOS_FRAME_TYPE_HEART_BEAT:
  case ETHOS_FRAME_TYPE_UPLOAD_PROG:
    if(ethos->framesize < ETHOS_FRAME_MTU) {
      ethos->rxbuf[ethos->framesize++] = c;
    } else {
      LOG_ERR("lost frame\n");
      _reset_state(ethos);
    }
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
_end_of_frame(ethos_t *ethos)
{
  switch(ethos->frametype) {
  case ETHOS_FRAME_TYPE_DATA:
    if(ethos->stage == STAGE_INIT) {
      if(ethos->framesize == ETHOS_PREAMBLE_SIZE &&
         !strncmp((const char *)ethos->rxbuf, ETHOS_PREAMBLE_UUUU, ETHOS_PREAMBLE_SIZE)) {
        ethos->stage = STAGE_HELLO;
        break;
      }
    }
    if(ethos->cmd_cb) {
      ethos->cmd_cb(ethos->rxbuf, ethos->framesize);
    }
    break;
  case ETHOS_FRAME_TYPE_TEXT:
    ethos->rxbuf[ethos->framesize] = '\0';
    alog("%s", ethos->rxbuf);
    break;
  case ETHOS_FRAME_TYPE_HELLO:
    if(ethos->framesize == ETHOS_MAC_ADDR_LEN) {
      memcpy(ethos->remote_mac_addr, ethos->rxbuf, ethos->framesize);
      ethos_send_frame(ethos->mac_addr, ETHOS_MAC_ADDR_LEN, ETHOS_FRAME_TYPE_HELLO_REPLY);
    }
    break;
  case ETHOS_FRAME_TYPE_HELLO_REPLY:
    if(ethos->stage == STAGE_HELLO) {
      if(ethos->framesize == ETHOS_MAC_ADDR_LEN) {
        memcpy(ethos->remote_mac_addr, ethos->rxbuf, ethos->framesize);
        ethos->stage = STAGE_RUNNING;
        aboot_callback_status(0, ABOOT_TINY_STATUS_RUNNING);
      }
    } else {
      LOG_ERR("Unexpected HELLO REPLY frame in stage %d\n", ethos->stage);
      aboot_callback_start(-1, NULL);
    }
    break;
  case ETHOS_FRAME_TYPE_UPLOAD_PROG:
    break;
  case ETHOS_FRAME_TYPE_HEART_BEAT:
    break;
  default:
    break;
  }

  ethos->timeout_count = 0;
  _reset_state(ethos);
}
/*---------------------------------------------------------------------------*/
static void
ethos_process_incoming_data(const uint8_t *data, size_t len)
{
  ethos_t *ethos = ethos_dev;

  for(size_t i = 0; i < len; i++) {
    uint8_t c = data[i];

    switch(ethos->state) {
    case WAIT_FRAMESTART:
      if(c == ETHOS_FRAME_DELIMITER) {
        _reset_state(ethos);
        ethos->state = IN_FRAME;
      }
      break;
    case IN_FRAME:
      if(c == ETHOS_ESC_CHAR) {
        ethos->state = IN_ESCAPE;
      } else if(c == ETHOS_FRAME_DELIMITER) {
        if(ethos->framesize) {
          _end_of_frame(ethos);
        }
      } else {
        _handle_char(ethos, c);
      }
      break;
    case IN_ESCAPE:
      switch(c) {
      case (ETHOS_FRAME_DELIMITER ^ 0x20):
        _handle_char(ethos, ETHOS_FRAME_DELIMITER);
        break;
      case (ETHOS_ESC_CHAR ^ 0x20):
        _handle_char(ethos, ETHOS_ESC_CHAR);
        break;
      case (ETHOS_FRAME_TYPE_TEXT ^ 0x20):
        ethos->frametype = ETHOS_FRAME_TYPE_TEXT;
        break;
      case (ETHOS_FRAME_TYPE_HELLO ^ 0x20):
        ethos->frametype = ETHOS_FRAME_TYPE_HELLO;
        break;
      case (ETHOS_FRAME_TYPE_HELLO_REPLY ^ 0x20):
        ethos->frametype = ETHOS_FRAME_TYPE_HELLO_REPLY;
        break;
      case (ETHOS_FRAME_TYPE_HEART_BEAT ^ 0x20):
        ethos->frametype = ETHOS_FRAME_TYPE_HEART_BEAT;
        break;
      case (ETHOS_FRAME_TYPE_UPLOAD_PROG ^ 0x20):
        ethos->frametype = ETHOS_FRAME_TYPE_UPLOAD_PROG;
        break;
      }
      ethos->state = IN_FRAME;
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
ethos_rx_handle(uint8_t *data, size_t len)
{
  ethos_process_incoming_data(data, len);
}
/*---------------------------------------------------------------------------*/
void
ethos_register_cmd_callback(ethos_rx_cb_t cb)
{
  ethos_t *ethos = ethos_dev;
  assert(ethos);
  ethos->cmd_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
ethos_register_data_callback(ethos_rx_cb_t cb)
{
  ethos_t *ethos = ethos_dev;
  assert(ethos);
  ethos->data_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
ethos_init(void)
{
  ethos_t *ethos;

  ethos = (ethos_t *)malloc(sizeof(ethos_t));
  assert(ethos);
  memset(ethos, 0x0, sizeof(ethos_t));
  ethos->stage = STAGE_INIT;
  ethos->state = WAIT_FRAMESTART;
  ethos->txbuf_size = ETHOS_MEMB_SIZE;
  ethos->frametype = ETHOS_FRAME_TYPE_DATA;
  ethos->timeout_count = 0;
  ethos->header[0] = 3; /* set kIdFastboot */
  memcpy(ethos->mac_addr, uip_lladdr.addr, ETHOS_MAC_ADDR_LEN);
  ethos_dev = ethos;
  serial_port_register_rx_cb(ethos_rx_handle);
  process_start(&ethos_connect_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
ethos_exit(void)
{
  ethos_t *ethos = ethos_dev;

  serial_port_register_rx_cb(NULL);
  process_exit(&ethos_connect_process);

  if(ethos) {
    ethos->cmd_cb = NULL;
    ethos->data_cb = NULL;
    if(ethos->txbuf) {
      free(ethos->txbuf);
      ethos->txbuf = NULL;
    }
    free(ethos);
    ethos_dev = NULL;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ethos_connect_process, ev, data)
{
  static struct etimer timer;
  static ethos_t *ethos;

  PROCESS_BEGIN();

  etimer_set(&timer, TIMER_PERIOD_100MS);
  ethos = ethos_dev;
  assert(ethos);

  aboot_callback_status(0, ABOOT_TINY_STATUS_CONNECTING);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&timer)) {
      if(ethos->stage == STAGE_INIT) {
        ethos_send_frame((uint8_t *)ETHOS_PREAMBLE_UUUU, ETHOS_PREAMBLE_SIZE, ETHOS_FRAME_TYPE_DATA);
      } else if(ethos->stage == STAGE_HELLO) {
        ethos_send_frame(ethos->mac_addr, ETHOS_MAC_ADDR_LEN, ETHOS_FRAME_TYPE_HELLO);
      } else {
        /* check link status */
        if(++ethos->timeout_count > LINK_CHECK_COUNT) {
          ethos->stage = STAGE_FAILED;
          LOG_ERR("ethos link lost!\n");
          aboot_callback_status(-1, ABOOT_TINY_STATUS_FAILED);
        }
      }
      etimer_reset(&timer);
    }
    if(ev == PROCESS_EVENT_EXIT) {
      etimer_stop(&timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
