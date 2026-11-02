#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "contiki.h"
#include "os/dev/serialport.h"
#include "aboot-transport.h"
#include "aboot-callback.h"
#include "aboot-download.h"
#include "aboot-hotplug.h"
#include "aboot-device.h"
#include "preamble.h"

#include "sys/log.h"
#define LOG_MODULE "Preamble"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
typedef enum {
  PREAMBLE_STATE_INIT,
  PREAMBLE_STATE_WAIT_FOR_DEVICE,
  PREAMBLE_STATE_CHECK_DEVICE_TYPE,
  PREAMBLE_STATE_FINAL,
  PREAMBLE_STATE_EXIT,
} preamble_state_t;
/*---------------------------------------------------------------------------*/
static char *aboot_device_path;
static int aboot_baud_rate;
static preamble_state_t preamble_state;
static int preamble_index;
static char preamble_buffer[ABOOT_PREAMBLE_SIZE + 1];
static bool aboot_hotplug_enabled;
/*---------------------------------------------------------------------------*/
PROCESS(preamble_process, "preamble process");
AUTOSTART_PROCESSES(&preamble_process);
/*---------------------------------------------------------------------------*/
static void
preamble_callback(uint8_t *data, size_t size)
{
  if(preamble_state != PREAMBLE_STATE_CHECK_DEVICE_TYPE) {
    return;
  }

  while(size--) {
    if(preamble_index < 0) {
      if(*data == ABOOT_FRAME_DELIMITER) {
        preamble_index = 0;
      }
    } else if(preamble_index < ABOOT_PREAMBLE_SIZE) {
      if(*data == ABOOT_FRAME_DELIMITER) {
        preamble_index = 0;
      } else {
        preamble_buffer[preamble_index++] = *data;
      }
    } else if(preamble_index == ABOOT_PREAMBLE_SIZE) {
      if(*data == ABOOT_FRAME_DELIMITER) {
        if(!strncmp(preamble_buffer, ABOOT_PREAMBLE_UUUU, ABOOT_PREAMBLE_SIZE)) {
          aboot_transport_init(ABOOT_DEVICE_TYPE_ETHOS);
          preamble_state = PREAMBLE_STATE_FINAL;
          process_poll(&preamble_process);
          break;
        } else if(!strncmp(preamble_buffer, ABOOT_PREAMBLE_UABT, ABOOT_PREAMBLE_SIZE)) {
          aboot_transport_init(ABOOT_DEVICE_TYPE_SMUX);
          preamble_state = PREAMBLE_STATE_FINAL;
          process_poll(&preamble_process);
          break;
        } else {
          preamble_index = 0;
        }
      } else {
        preamble_index = -1;
      }
    }
    data++;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(preamble_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  preamble_state = PREAMBLE_STATE_INIT;

  while(1) {
    PROCESS_YIELD();

    if(preamble_state == PREAMBLE_STATE_INIT) {
      /* Setup a periodic timer that expires after 100 milli-seconds. */
      etimer_set(&timer, CLOCK_SECOND / 10);
      preamble_state = PREAMBLE_STATE_WAIT_FOR_DEVICE;
      if(aboot_hotplug_enabled) {
        aboot_hotplug_init();
      }
      aboot_download_init();
    } else if(preamble_state == PREAMBLE_STATE_WAIT_FOR_DEVICE) {
      /* Wait for the periodic timer to expire and then restart the timer. */
      PROCESS_WAIT_UNTIL(etimer_expired(&timer));

      if(aboot_device_path && !access(aboot_device_path, F_OK)) {
        serial_port_connect_opt_t connect_op;
        connect_op.baud = aboot_baud_rate;
        connect_op.data_bits = 8;
        connect_op.rtscts = false;
        connect_op.xon = false;
        connect_op.xoff = false;
        connect_op.xany = false;
        connect_op.dsrdtr = false;
        connect_op.hupcl = false;
        connect_op.parity = SERIALPORT_PARITY_NONE;
        connect_op.stop_bits = SERIALPORT_STOPBITS_ONE;

        if(!serial_port_open(aboot_device_path, &connect_op)) {
          serial_port_register_rx_cb(preamble_callback);
          etimer_reset(&timer);
          preamble_state = PREAMBLE_STATE_CHECK_DEVICE_TYPE;
        } else {
          LOG_ERR("open device \"%s\" failed\n", aboot_device_path);
          aboot_callback_start(-1, NULL);
          etimer_stop(&timer);
          preamble_state = PREAMBLE_STATE_INIT;
        }
      } else {
        etimer_reset(&timer);
      }
    } else if(preamble_state == PREAMBLE_STATE_CHECK_DEVICE_TYPE) {
      /* Wait for the periodic timer to expire and then restart the timer. */
      PROCESS_WAIT_UNTIL(etimer_expired(&timer));

      uint8_t *buffer = (uint8_t *)malloc(ABOOT_PREAMBLE_SIZE + 1);
      if(!buffer) {
        LOG_WARN("out of memory\n");
        etimer_reset(&timer);
        continue;
      }
      memcpy(buffer, ABOOT_PREAMBLE_UUUU, ABOOT_PREAMBLE_SIZE);
      buffer[ABOOT_PREAMBLE_SIZE] = ABOOT_FRAME_DELIMITER;
      if(serial_port_write(buffer, ABOOT_PREAMBLE_SIZE + 1) < 0) {
        aboot_callback_start(-1, NULL);
        etimer_stop(&timer);
        preamble_state = PREAMBLE_STATE_INIT;
        free(buffer);
      } else {
        etimer_reset(&timer);
      }
    } else if(preamble_state == PREAMBLE_STATE_FINAL) {
      etimer_stop(&timer);
    } else if(preamble_state == PREAMBLE_STATE_EXIT) {
      if(aboot_hotplug_enabled) {
        aboot_hotplug_exit();
      }
      aboot_download_exit();
      /* timer maybe alreay stopped, call again has no problem */
      etimer_stop(&timer);
      aboot_transport_exit();
      serial_port_close();
      aboot_callback_stop(0, NULL);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
preamble_start(const char *dev_path, int baud_rate)
{
  if(aboot_device_path) {
    free(aboot_device_path);
    aboot_device_path = NULL;
  }

  if(dev_path) {
    int len = strlen(dev_path);
    aboot_device_path = (char *)malloc(len + 1);
    if(!aboot_device_path) {
      LOG_ERR("out of memory\n");
      return -1;
    }
    strcpy(aboot_device_path, dev_path);
    aboot_hotplug_enabled = false;
  } else {
    aboot_hotplug_enabled = true;
  }

  aboot_baud_rate = baud_rate;
  preamble_state = PREAMBLE_STATE_INIT;
  preamble_index = -1;

  process_poll(&preamble_process);

  return 0;
}
/*---------------------------------------------------------------------------*/
void
preamble_stop(void)
{
  preamble_state = PREAMBLE_STATE_EXIT;
  process_poll(&preamble_process);
}
/*---------------------------------------------------------------------------*/
void
aboot_device_online(tty_device_info_t *device)
{
  if(aboot_device_path) {
    free(aboot_device_path);
    aboot_device_path = NULL;
  }
  int len = strlen(device->path);
  aboot_device_path = (char *)malloc(len + 1);
  if(!aboot_device_path) {
    LOG_ERR("out of memory\n");
    return;
  }
  strcpy(aboot_device_path, device->path);
}
/*---------------------------------------------------------------------------*/
void
aboot_device_offline(tty_device_info_t *device)
{
  (void)device;
}
/*---------------------------------------------------------------------------*/
