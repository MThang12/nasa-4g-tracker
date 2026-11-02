#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "contiki.h"
#include "serialport.h"
#include "hotplug.h"
#include "at-fallback.h"

/*---------------------------------------------------------------------------*/
const char *at_cmds[] = {
  "AT+BOOTLDR=1\r\n",
  "AT$MYDOWNLOAD=1\r\n",
  "AT+QDOWNLOAD=1\r\n",
};
/*---------------------------------------------------------------------------*/
PROCESS(at_fallback_process, "at fallback process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(at_fallback_process, ev, data)
{
  static struct etimer timer;
  static int at_index;

  PROCESS_BEGIN();

  at_index = 0;

  /* Setup a periodic timer that expires after 100 milli-seconds. */
  etimer_set(&timer, CLOCK_SECOND / 10);

  while(1) {
    PROCESS_YIELD();

    if(ev == PROCESS_EVENT_TIMER) {
      const char *p = at_cmds[at_index];
      at_index = (at_index + 1) % (sizeof(at_cmds) / sizeof(at_cmds[0]));
      char *cmd = (char *)malloc(strlen(p) + 1);
      strcpy(cmd, p);
      serial_port_write((uint8_t *)cmd, strlen(cmd));
      etimer_reset(&timer);
    } else if(ev == PROCESS_EVENT_EXIT) {
      etimer_stop(&timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
atcmd_device_online(tty_device_info_t *device)
{
  serial_port_connect_opt_t connect_op;
  connect_op.baud = 115200;
  connect_op.data_bits = 8;
  connect_op.rtscts = false;
  connect_op.xon = false;
  connect_op.xoff = false;
  connect_op.xany = false;
  connect_op.dsrdtr = false;
  connect_op.hupcl = false;
  connect_op.parity = SERIALPORT_PARITY_NONE;
  connect_op.stop_bits = SERIALPORT_STOPBITS_ONE;

  if(!serial_port_open(device->path, &connect_op)) {
    serial_port_register_rx_cb(NULL);
    process_start(&at_fallback_process, NULL);
  }
}
/*---------------------------------------------------------------------------*/
void
atcmd_device_offline(tty_device_info_t *device)
{
  (void)device;
  process_exit(&at_fallback_process);
  serial_port_close();
}
/*---------------------------------------------------------------------------*/
