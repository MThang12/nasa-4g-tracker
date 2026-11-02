#include "contiki.h"
#include <stdlib.h>
#include <string.h>
#include "lib/assert.h"
#include "smux.h"
#include "serialport.h"
#include "aboot-callback.h"

#include "sys/log.h"
#define LOG_MODULE "Smux"
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
  SMUX_FRAME_TYPE_STDIO       = (0x00),
  SMUX_FRAME_TYPE_HELLO       = (0x01),
  SMUX_FRAME_TYPE_HELLO_REPLY = (0x02),
  SMUX_FRAME_TYPE_ABOOT_CMD   = (0x03),
  SMUX_FRAME_TYPE_ABOOT_DATA  = (0x04),
  SMUX_FRAME_TYPE_HEART_BEAT  = (0x05),
  SMUX_ESC_CHAR               = (0x7D),
  SMUX_FRAME_DELIMITER        = (0x7E)
} smux_frame_type_t;

typedef struct {
  running_stage_t stage;
  uint8_t rxbuf[SMUX_FRAME_MTU];  /**< rxbuffer for incoming data */
  smux_rx_cb_t cmd_cb;
  smux_rx_cb_t data_cb;
  line_state_t state;             /**< Line status variable */
  size_t framesize;               /**< size of currently incoming frame */
  smux_frame_type_t frametype;    /**< type of currently incoming frame */
  uint16_t mtu;                   /**< SMUX maximum transmit unit */
  uint8_t *txbuf;
  size_t txbuf_size;
  size_t txbuf_used;
  size_t total_tx_size;
  int timeout_count;              /**< count for timeout */
} smux_t;
/*---------------------------------------------------------------------------*/
static smux_t *smux_dev;
PROCESS(smux_connect_process, "smux connect process");
/*---------------------------------------------------------------------------*/
static void
smux_set_tx_size(size_t size)
{
  smux_t *smux = smux_dev;

  assert(smux);
  smux->total_tx_size = size;
  smux->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
void
smux_set_aboot_data_size(size_t size)
{
  smux_set_tx_size(size);
}
/*---------------------------------------------------------------------------*/
static void
send_txbuf(void)
{
  smux_t *smux = smux_dev;

  serial_port_write(smux->txbuf, smux->txbuf_used);
  smux->txbuf = NULL;
  smux->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
static void
alloc_txbuf(void)
{
  smux_t *smux = smux_dev;

  smux->txbuf = (uint8_t *)malloc(smux->txbuf_size);
  assert(smux->txbuf);
  smux->txbuf_used = 0;
}
/*---------------------------------------------------------------------------*/
static void
smux_send_frame(const uint8_t *data, size_t size, smux_frame_type_t frame_type)
{
  smux_t *smux = smux_dev;

  smux->total_tx_size -= size;
  size_t used = smux->txbuf_used;

  while(size > 0) {
    if(!smux->txbuf) {
      alloc_txbuf();
      used = 0;
    }
    if(used + 4 > smux->txbuf_size) {
      smux->txbuf_used = used;
      send_txbuf();
      used = 0;
      continue;
    }
    uint8_t *p = smux->txbuf + used;
    /* send frame delimiter */
    *p++ = SMUX_FRAME_DELIMITER;
    used++;
    /* set frame type */
    if(frame_type) {
      *p++ = SMUX_ESC_CHAR;
      *p++ = frame_type ^ 0x20;
      used += 2;
    }
    uint16_t mtu = smux->mtu;
    /* send frame content */
    while(size && used + 3 <= smux->txbuf_size && mtu--) {
      uint8_t c = *data++;
      switch(c) {
      case SMUX_FRAME_DELIMITER:
        *p++ = SMUX_ESC_CHAR;
        *p++ = SMUX_FRAME_DELIMITER ^ 0x20;
        used += 2;
        break;
      case SMUX_ESC_CHAR:
        *p++ = SMUX_ESC_CHAR;
        *p++ = SMUX_ESC_CHAR ^ 0x20;
        used += 2;
        break;
      default:
        *p++ = c;
        used++;
      }
      size--;
    }
    /* send frame delimiter */
    *p++ = SMUX_FRAME_DELIMITER;
    used++;
    if(!size) {
      if(smux->total_tx_size == 0) {
        smux->txbuf_used = used;
        send_txbuf();
        used = 0;
      }
    }
  }
  smux->txbuf_used = used;
}
/*---------------------------------------------------------------------------*/
void
smux_write_aboot_data(const uint8_t *data, size_t len)
{
  smux_send_frame(data, len, SMUX_FRAME_TYPE_ABOOT_DATA);
}
/*---------------------------------------------------------------------------*/
void
smux_write_aboot_cmd(const uint8_t *data, size_t len)
{
  smux_set_tx_size(len);
  smux_send_frame(data, len, SMUX_FRAME_TYPE_ABOOT_CMD);
}
/*---------------------------------------------------------------------------*/
static void
_reset_state(smux_t *smux)
{
  smux->state = WAIT_FRAMESTART;
  smux->frametype = 0;
  smux->framesize = 0;
}
/*---------------------------------------------------------------------------*/
static void
_handle_char(smux_t *smux, char c)
{
  switch(smux->frametype) {
  case SMUX_FRAME_TYPE_STDIO:
  case SMUX_FRAME_TYPE_HELLO:
  case SMUX_FRAME_TYPE_HELLO_REPLY:
  case SMUX_FRAME_TYPE_ABOOT_CMD:
  case SMUX_FRAME_TYPE_ABOOT_DATA:
  case SMUX_FRAME_TYPE_HEART_BEAT:
    if(smux->framesize < smux->mtu) {
      smux->rxbuf[smux->framesize++] = c;
    } else {
      LOG_ERR("lost frame\n");
      _reset_state(smux);
    }
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
_end_of_frame(smux_t *smux)
{
  switch(smux->frametype) {
  case SMUX_FRAME_TYPE_STDIO:
    if(smux->stage == STAGE_INIT) {
      if(smux->framesize == SMUX_PREAMBLE_SIZE &&
         !strncmp((const char *)smux->rxbuf, SMUX_PREAMBLE_UABT, SMUX_PREAMBLE_SIZE)) {
        smux->stage = STAGE_HELLO;
        break;
      }
    }
    smux->rxbuf[smux->framesize] = '\0';
    alog("%s", smux->rxbuf);
    break;
  case SMUX_FRAME_TYPE_HELLO_REPLY:
    if(smux->stage == STAGE_HELLO) {
      if(smux->framesize == sizeof(smux->mtu) && smux->stage == STAGE_HELLO) {
        smux->mtu = (uint8_t)smux->rxbuf[0] << 8 | (uint8_t)smux->rxbuf[1];
        aboot_callback_start(0, NULL);
        smux->stage = STAGE_RUNNING;
        aboot_callback_status(0, ABOOT_TINY_STATUS_RUNNING);
      }
    } else {
      LOG_ERR("Unexpected HELLO REPLY frame in stage %d\n", smux->stage);
      aboot_callback_start(-1, NULL);
    }
    break;
  case SMUX_FRAME_TYPE_ABOOT_CMD:
    if(smux->cmd_cb) {
      smux->cmd_cb(smux->rxbuf, smux->framesize);
    }
    break;
  case SMUX_FRAME_TYPE_HEART_BEAT:
    break;
  case SMUX_FRAME_TYPE_ABOOT_DATA:
    if(smux->data_cb) {
      smux->data_cb(smux->rxbuf, smux->framesize);
    }
    break;
  default:
    break;
  }

  smux->timeout_count = 0;
  _reset_state(smux);
}
/*---------------------------------------------------------------------------*/
static void
smux_process_incoming_data(const uint8_t *data, size_t len)
{
  smux_t *smux = smux_dev;

  for(size_t i = 0; i < len; i++) {
    uint8_t c = data[i];

    switch(smux->state) {
    case WAIT_FRAMESTART:
      if(c == SMUX_FRAME_DELIMITER) {
        _reset_state(smux);
        smux->state = IN_FRAME;
      }
      break;
    case IN_FRAME:
      if(c == SMUX_ESC_CHAR) {
        smux->state = IN_ESCAPE;
      } else if(c == SMUX_FRAME_DELIMITER) {
        if(smux->framesize) {
          _end_of_frame(smux);
        }
      } else {
        _handle_char(smux, c);
      }
      break;
    case IN_ESCAPE:
      switch(c) {
      case (SMUX_FRAME_DELIMITER ^ 0x20):
        _handle_char(smux, SMUX_FRAME_DELIMITER);
        break;
      case (SMUX_ESC_CHAR ^ 0x20):
        _handle_char(smux, SMUX_ESC_CHAR);
        break;
      case (SMUX_FRAME_TYPE_STDIO ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_STDIO;
        break;
      case (SMUX_FRAME_TYPE_HELLO ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_HELLO;
        break;
      case (SMUX_FRAME_TYPE_HELLO_REPLY ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_HELLO_REPLY;
        break;
      case (SMUX_FRAME_TYPE_ABOOT_CMD ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_ABOOT_CMD;
        break;
      case (SMUX_FRAME_TYPE_ABOOT_DATA ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_ABOOT_DATA;
        break;
      case (SMUX_FRAME_TYPE_HEART_BEAT ^ 0x20):
        smux->frametype = SMUX_FRAME_TYPE_HEART_BEAT;
        break;
      }
      smux->state = IN_FRAME;
      break;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
smux_rx_handle(uint8_t *data, size_t len)
{
  smux_process_incoming_data(data, len);
}
/*---------------------------------------------------------------------------*/
void
smux_register_cmd_callback(smux_rx_cb_t cb)
{
  smux_t *smux = smux_dev;
  assert(smux);
  smux->cmd_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
smux_register_data_callback(smux_rx_cb_t cb)
{
  smux_t *smux = smux_dev;
  assert(smux);
  smux->data_cb = cb;
}
/*---------------------------------------------------------------------------*/
void
smux_init(void)
{
  smux_t *smux;

  smux = (smux_t *)malloc(sizeof(smux_t));
  assert(smux);
  memset(smux, 0x0, sizeof(smux_t));
  smux->stage = STAGE_INIT;
  smux->state = WAIT_FRAMESTART;
  smux->txbuf_size = SMUX_MEMB_SIZE;
  smux->frametype = SMUX_FRAME_TYPE_ABOOT_CMD;
  smux->mtu = SMUX_FRAME_MTU;
  smux->timeout_count = 0;
  smux_dev = smux;
  serial_port_register_rx_cb(smux_rx_handle);
  process_start(&smux_connect_process, NULL);
}
/*---------------------------------------------------------------------------*/
void
smux_exit(void)
{
  smux_t *smux = smux_dev;

  serial_port_register_rx_cb(NULL);
  process_exit(&smux_connect_process);

  if(smux) {
    smux->cmd_cb = NULL;
    smux->data_cb = NULL;
    if(smux->txbuf) {
      free(smux->txbuf);
      smux->txbuf = NULL;
    }
    free(smux);
    smux_dev = NULL;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(smux_connect_process, ev, data)
{
  static struct etimer timer;
  static smux_t *smux;

  PROCESS_BEGIN();

  etimer_set(&timer, TIMER_PERIOD_100MS);
  smux = smux_dev;
  assert(smux);

  aboot_callback_status(0, ABOOT_TINY_STATUS_CONNECTING);

  while(1) {
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&timer)) {
      if(smux->stage == STAGE_INIT) {
        smux_set_tx_size(SMUX_PREAMBLE_SIZE);
        smux_send_frame((uint8_t *)SMUX_PREAMBLE_UABT, SMUX_PREAMBLE_SIZE, SMUX_FRAME_TYPE_STDIO);
      } else if(smux->stage == STAGE_HELLO) {
        uint16_t mtu = (uint16_t)((uint16_t)smux->mtu << 8 | (uint16_t)smux->mtu >> 8);
        smux_set_tx_size(2);
        smux_send_frame((uint8_t *)&mtu, 2, SMUX_FRAME_TYPE_HELLO);
      } else {
        /* check link status */
        if(++smux->timeout_count > LINK_CHECK_COUNT) {
          smux->stage = STAGE_FAILED;
          LOG_ERR("smux link lost!\n");
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
